#include "Canny.h"
#include "Blur.h"
#include "Greyscale.h"
#include <cmath>
#include <algorithm>
#include <stack>

void Canny::applyCanny(std::unique_ptr<Image>& image, double upperThresh, double lowerThresh) {
    if (!image) return;
    
    try {
        step1_convertToGrayscale(image);
        
        step2_applyGaussianBlur(image);
        
        std::vector<std::vector<double>> gradientX, gradientY;
        step3_computeSobelGradients(image, gradientX, gradientY);
        
        std::vector<std::vector<double>> magnitude, direction;
        step4_computeMagnitudeAndDirection(gradientX, gradientY, magnitude, direction);
        
        upperThresh = std::max(1.0, upperThresh);
        lowerThresh = std::max(1.0, std::min(lowerThresh, upperThresh * 0.8));
        std::vector<std::vector<bool>> strongEdges;
        step5_nonMaximumSuppression(magnitude, direction, strongEdges, upperThresh);
        
        std::vector<std::vector<bool>> finalEdges;
        step6_hysteresisThresholding(magnitude, direction, strongEdges, finalEdges, lowerThresh);

        // Zastąpienie obrazu wynikiem
        int width = image->width();
        int height = image->height();
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                if (finalEdges[x][y]) {
                    image->setPixel(x, y, 255, 255, 255); // biała krawędź
                } else {
                    image->setPixel(x, y, 0, 0, 0); // czarne tło
                }
            }
        }
        
    } catch (const std::exception& e) {
        // nic nie zmieniaj przy błędzie
        return;
    }
}

void Canny::step1_convertToGrayscale(std::unique_ptr<Image>& image) {
    Greyscale::convertToGreyscale(image);
}

void Canny::step2_applyGaussianBlur(std::unique_ptr<Image>& image) {
    Blur::gaussianBlur(image, 1.6, 3);
}

void Canny::step3_computeSobelGradients(std::unique_ptr<Image>& image,
                                       std::vector<std::vector<double>>& gradientX,
                                       std::vector<std::vector<double>>& gradientY) {
    int width = image->width();
    int height = image->height();
    
    // Inicjalizacja tablic gradientów
    gradientX.assign(width, std::vector<double>(height, 0.0));
    gradientY.assign(width, std::vector<double>(height, 0.0));
    
    // Generowanie Kerneli Sobela
    auto sobelKernels = generateSobelKernels();
    auto sobelX = sobelKernels.first;  // rawHorizontalDetection
    auto sobelY = sobelKernels.second; // rawVerticalDetection
    
    std::vector<std::vector<int>> originalPixels(width, std::vector<int>(height));
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // Ponieważ obraz jest już w skali szarości, używamy tylko jednego kanału
            originalPixels[x][y] = image->getPixelR(x, y);
        }
    }
    
    // Obliczenie gradientów Gx i Gy
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double gx = 0.0, gy = 0.0;
            
            // Aplikowanie operatora Sobela
            for (int kx = 0; kx < 3; kx++) {
                for (int ky = 0; ky < 3; ky++) {
                    int pixelX = std::max(0, std::min(width - 1, x + kx - 1));
                    int pixelY = std::max(0, std::min(height - 1, y + ky - 1));
                    
                    int intensity = originalPixels[pixelX][pixelY];
                    
                    gx += intensity * sobelX[kx][ky];
                    gy += intensity * sobelY[kx][ky];
                }
            }
            
            gradientX[x][y] = gx;
            gradientY[x][y] = gy;
        }
    }
}

void Canny::step4_computeMagnitudeAndDirection(const std::vector<std::vector<double>>& gradientX,
                                              const std::vector<std::vector<double>>& gradientY,
                                              std::vector<std::vector<double>>& magnitude,
                                              std::vector<std::vector<double>>& direction) {
    int width = gradientX.size();
    int height = gradientX[0].size();
    
    // Inicjalizacja tablic
    magnitude.assign(width, std::vector<double>(height, 0.0));
    direction.assign(width, std::vector<double>(height, 0.0));
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double gx = gradientX[x][y];
            double gy = gradientY[x][y];
            
            // Obliczenie magnitude: mij = √(Gx² + Gy²)
            magnitude[x][y] = std::sqrt(gx * gx + gy * gy);
            
            // Obliczenie kierunku: θij = arctan(Gy/Gx)
            direction[x][y] = std::atan2(gy, gx) * 180.0 / M_PI; // w stopniach
        }
    }
}

void Canny::step5_nonMaximumSuppression(const std::vector<std::vector<double>>& magnitude,
                                       const std::vector<std::vector<double>>& direction,
                                       std::vector<std::vector<bool>>& strongEdges,
                                       double upperThresh) {
    int width = magnitude.size();
    int height = magnitude[0].size();
    
    strongEdges.assign(width, std::vector<bool>(height, false));
    
    for (int x = 1; x < width - 1; x++) {
        for (int y = 1; y < height - 1; y++) {
            double currentMagnitude = magnitude[x][y];
            
            // 1. sprawdź, w którym z 4 możliwych kierunków jest gradient piksela (i,j)
            int sector = getDirectionSector(direction[x][y]);
            
            // 2. wybierz sąsiadów piksela (i,j) będących na linii prostopadłej do kierunku gradientu
            auto neighbors = getNeighborsForDirection(sector);
            int x1 = x + neighbors.first.first;
            int y1 = y + neighbors.first.second;
            int x2 = x + neighbors.second.first;
            int y2 = y + neighbors.second.second;
            
            // Sprawdzenie granic
            if (x1 < 0 || x1 >= width || y1 < 0 || y1 >= height ||
                x2 < 0 || x2 >= width || y2 < 0 || y2 >= height) {
                continue;
            }
            
            // 3. jeżeli moc gradientu piksela (i,j) jest większa od mocy gradientów 
            //    odpowiadających mu sąsiadów oraz większa od upper-thresh, 
            //    to dodaj piksel (i,j) do początkowego zbioru krawędzi
            if (currentMagnitude > magnitude[x1][y1] && 
                currentMagnitude > magnitude[x2][y2] && 
                currentMagnitude > upperThresh) {
                strongEdges[x][y] = true;
            }
        }
    }
}

void Canny::step6_hysteresisThresholding(const std::vector<std::vector<double>>& magnitude,
                                        const std::vector<std::vector<double>>& direction,
                                        const std::vector<std::vector<bool>>& strongEdges,
                                        std::vector<std::vector<bool>>& finalEdges,
                                        double lowerThresh) {
    int width = magnitude.size();
    int height = magnitude[0].size();
    
    finalEdges.assign(width, std::vector<bool>(height, false));
    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
    
    // Dla każdego piksela z początkowego zbioru krawędzi
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (strongEdges[x][y] && !visited[x][y]) {
                // Użyj stosu do iteracyjnego śledzenia
                std::stack<std::pair<int, int>> stack;
                stack.push({x, y});
                
                while (!stack.empty()) {
                    auto current = stack.top();
                    stack.pop();
                    
                    int cx = current.first;
                    int cy = current.second;
                    
                    if (cx < 0 || cx >= width || cy < 0 || cy >= height || visited[cx][cy]) {
                        continue;
                    }
                    
                    visited[cx][cy] = true;
                    finalEdges[cx][cy] = true;
                    
                    // Sprawdź osobno jego dwóch sąsiadów wyznaczonych przez kierunek gradientu
                    int sector = getDirectionSector(direction[cx][cy]);
                    auto neighbors = getNeighborsForDirection(sector);
                    
                    std::vector<std::pair<int, int>> candidates = {
                        {cx + neighbors.first.first, cy + neighbors.first.second},
                        {cx + neighbors.second.first, cy + neighbors.second.second}
                    };
                    
                    for (auto& candidate : candidates) {
                        int nx = candidate.first;
                        int ny = candidate.second;
                        
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height && !visited[nx][ny]) {

                            // 1. moc gradientu większą od lower-thresh
                            if (magnitude[nx][ny] >= lowerThresh) {
                                
                                // 2. gradient skierowany w tę samą stronę co piksel, z którego przyszedł
                                double dirDiff = std::abs(direction[nx][ny] - direction[cx][cy]);
                                if (dirDiff > 180) dirDiff = 360 - dirDiff; // normalizacja
                                
                                if (dirDiff <= 22.5) { // bardziej restrykcyjna tolerancja kierunku
                                    
                                    // 3. moc gradientu większa od jego sąsiadów (lokalny maksimum)
                                    bool isLocalMaximum = true;
                                    
                                    // Sprawdź czy kandydat jest lokalnym maksimum w kierunku prostopadłym do gradientu
                                    int candidateSector = getDirectionSector(direction[nx][ny]);
                                    auto candidateNeighbors = getNeighborsForDirection(candidateSector);
                                    
                                    // Sprawdź dwóch sąsiadów prostopadłych do kierunku gradientu
                                    int n1x = nx + candidateNeighbors.first.first;
                                    int n1y = ny + candidateNeighbors.first.second;
                                    int n2x = nx + candidateNeighbors.second.first;
                                    int n2y = ny + candidateNeighbors.second.second;
                                    
                                    // Sprawdź granice i porównaj magnitude
                                    if (n1x >= 0 && n1x < width && n1y >= 0 && n1y < height) {
                                        if (magnitude[nx][ny] <= magnitude[n1x][n1y]) {
                                            isLocalMaximum = false;
                                        }
                                    }
                                    if (n2x >= 0 && n2x < width && n2y >= 0 && n2y < height) {
                                        if (magnitude[nx][ny] <= magnitude[n2x][n2y]) {
                                            isLocalMaximum = false;
                                        }
                                    }
                                    
                                    // Dodaj kandydata tylko jeśli spełnia wszystkie warunki
                                    if (isLocalMaximum) {
                                        stack.push({nx, ny});
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Funkcje pomocnicze
std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> Canny::generateSobelKernels() {
    // Operator Sobela - kierunek X (rawHorizontalDetection)
    std::vector<std::vector<double>> sobelX = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    
    // Operator Sobela - kierunek Y (rawVerticalDetection)
    std::vector<std::vector<double>> sobelY = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };
    
    return {sobelX, sobelY};
}

int Canny::getDirectionSector(double angle) {
    // Normalizacja kąta do zakresu [0, 180)
    while (angle < 0) angle += 180;
    while (angle >= 180) angle -= 180;
    
    // Podział na 4 sektory (każdy po 45 stopni)
    if (angle < 22.5 || angle >= 157.5) {
        return 0; // kierunek poziomy (0°, 180°)
    } else if (angle >= 22.5 && angle < 67.5) {
        return 1; // kierunek 45°
    } else if (angle >= 67.5 && angle < 112.5) {
        return 2; // kierunek pionowy (90°)
    } else {
        return 3; // kierunek 135°
    }
}

std::pair<std::pair<int, int>, std::pair<int, int>> Canny::getNeighborsForDirection(int sector) {
    switch (sector) {
        case 0: // kierunek poziomy
            return {{-1, 0}, {1, 0}};
        case 1: // kierunek 45°
            return {{-1, -1}, {1, 1}};
        case 2: // kierunek pionowy
            return {{0, -1}, {0, 1}};
        case 3: // kierunek 135°
            return {{-1, 1}, {1, -1}};
        default:
            return {{0, 0}, {0, 0}};
    }
}
