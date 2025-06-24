#include "EdgeDetection.h"
#include <algorithm>
#include <QColor>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void EdgeDetection::laplacianFilter(std::unique_ptr<Image>& image, int kernelSize) {
    if (!image || kernelSize <= 0) {
        return;
    }
    
    // Upewnienie się, że rozmiar jądra jest nieparzysty
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }
    
    // Minimum 3x3
    if (kernelSize < 3) {
        kernelSize = 3;
    }
    
    // Generowanie jądra Laplaciana
    auto kernel = generateLaplacianKernel(kernelSize);
    
    // Aplikowanie konwolucji
    applyConvolution(image, kernel);
}

void EdgeDetection::laplacianOfGaussian(std::unique_ptr<Image>& image, double sigma, int windowSize, double threshold) {
    if (!image || sigma <= 0 || windowSize < 3) {
        return;
    }
    
    // Upewnienie się, że rozmiar okna jest nieparzysty
    if (windowSize % 2 == 0) {
        windowSize++;
    }
    
    // Aplikowanie algorytmu LoG z progowaniem zgodnie z instrukcją
    applyLoGWithThresholding(image, sigma, windowSize, threshold);
}

void EdgeDetection::laplacianOfGaussianSimple(std::unique_ptr<Image>& image, double sigma) {
    if (!image || sigma <= 0) {
        return;
    }
    
    // Oblicz rozmiar jądra LoG na podstawie sigma
    int kernelSize = std::max(3, static_cast<int>(6 * sigma + 1));
    if (kernelSize % 2 == 0) kernelSize++; // Upewnij się, że jest nieparzysty
    
    // Generowanie jądra Laplacian of Gaussian
    auto logKernel = generateLoGKernel(sigma, kernelSize);
    
    // Aplikowanie prostej konwolucji z normalizacją
    int width = image->width();
    int height = image->height();
    int kernelRadius = kernelSize / 2;
    
    // Tworzymy kopię oryginalnego obrazu
    std::vector<std::vector<QColor>> originalPixels(width, std::vector<QColor>(height));
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            originalPixels[x][y] = image->pixelAt(x, y);
        }
    }
    
    // Obliczanie odpowiedzi LoG dla każdego piksela
    std::vector<std::vector<double>> logResponse(width, std::vector<double>(height));
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double sum = 0.0;
            
            for (int kx = 0; kx < kernelSize; kx++) {
                for (int ky = 0; ky < kernelSize; ky++) {
                    int pixelX = x + kx - kernelRadius;
                    int pixelY = y + ky - kernelRadius;
                    
                    // Obsługa granic obrazu (odbicie)
                    pixelX = std::max(0, std::min(width - 1, pixelX));
                    pixelY = std::max(0, std::min(height - 1, pixelY));
                    
                    QColor pixel = originalPixels[pixelX][pixelY];
                    // Konwersja na luminancję
                    double luminance = 0.299 * pixel.red() + 0.587 * pixel.green() + 0.114 * pixel.blue();
                    
                    sum += luminance * logKernel[kx][ky];
                }
            }
            
            logResponse[x][y] = sum;
        }
    }
    
    // Znajdź zakres wartości dla normalizacji
    double minResponse = logResponse[0][0];
    double maxResponse = logResponse[0][0];
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            minResponse = std::min(minResponse, logResponse[x][y]);
            maxResponse = std::max(maxResponse, logResponse[x][y]);
        }
    }
    
    // Normalizuj do zakresu 0-255
    double range = maxResponse - minResponse;
    if (range > 0) {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                double normalizedValue = (logResponse[x][y] - minResponse) / range * 255.0;
                int outputValue = clamp(static_cast<int>(normalizedValue));
                
                // Ustaw ten sam poziom dla wszystkich kanałów (obraz w skali szarości)
                image->setPixel(x, y, outputValue, outputValue, outputValue);
            }
        }
    }
}

void EdgeDetection::laplacianFilterGrayscale(std::unique_ptr<Image>& image, int kernelSize) {
    if (!image || kernelSize <= 0) {
        return;
    }
    
    // Upewnienie się, że rozmiar jądra jest nieparzysty
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }
    
    // Minimum 3x3
    if (kernelSize < 3) {
        kernelSize = 3;
    }
    
    // Generowanie jądra Laplaciana
    auto kernel = generateLaplacianKernel(kernelSize);
    
    // Aplikowanie konwolucji z konwersją na skalę szarości
    applyLaplacianGrayscaleConvolution(image, kernel);
}

void EdgeDetection::laplacianFilterNegative(std::unique_ptr<Image>& image, int kernelSize) {
    if (!image || kernelSize <= 0) {
        return;
    }
    
    // Upewnienie się, że rozmiar jądra jest nieparzysty
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }
    
    // Minimum 3x3
    if (kernelSize < 3) {
        kernelSize = 3;
    }
    
    // Generowanie negatywnego jądra Laplaciana
    auto kernel = generateLaplacianNegativeKernel(kernelSize);
    
    // Aplikowanie konwolucji
    applyConvolution(image, kernel);
}

void EdgeDetection::robertsFilter(std::unique_ptr<Image>& image) {
    if (!image) {
        return;
    }
    
    // Generowanie jąder Robertsa
    auto robertsKernels = generateRobertsKernels();
    
    // Aplikowanie filtrów gradientowych
    applyGradientConvolution(image, robertsKernels.first, robertsKernels.second);
}

void EdgeDetection::prewittFilter(std::unique_ptr<Image>& image) {
    if (!image) {
        return;
    }
    
    // Generowanie jąder Prewitta
    auto prewittKernels = generatePrewittKernels();
    
    // Aplikowanie filtrów gradientowych
    applyGradientConvolution(image, prewittKernels.first, prewittKernels.second);
}

void EdgeDetection::sobelFilter(std::unique_ptr<Image>& image) {
    if (!image) {
        return;
    }
    
    // Generowanie jąder Sobela
    auto sobelKernels = generateSobelKernels();
    
    // Aplikowanie filtrów gradientowych
    applyGradientConvolution(image, sobelKernels.first, sobelKernels.second);
}

void EdgeDetection::customEdgeFilter(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& matrix) {
    if (!image || matrix.empty() || matrix[0].empty()) {
        return;
    }
    
    // Sprawdzenie czy macierz jest kwadratowa
    size_t size = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != size) {
            return; // Macierz nie jest kwadratowa
        }
    }
    
    // Sprawdzenie czy rozmiar jest nieparzysty
    if (size % 2 == 0) {
        return; // Rozmiar musi być nieparzysty
    }
    
    // Aplikowanie konwolucji z niestandardową macierzą
    applyConvolution(image, matrix);
}

std::vector<std::vector<double>> EdgeDetection::generateLaplacianKernel(int size) {
    std::vector<std::vector<double>> kernel(size, std::vector<double>(size, 0.0));
    
    if (size == 3) {
        // Standard 3x3 Laplacian kernel (4-connected)
        kernel = {
            { 0, -1,  0},
            {-1,  4, -1},
            { 0, -1,  0}
        };
    } else if (size == 5) {
        // 5x5 Laplacian kernel
        kernel = {
            { 0,  0, -1,  0,  0},
            { 0, -1, -2, -1,  0},
            {-1, -2, 16, -2, -1},
            { 0, -1, -2, -1,  0},
            { 0,  0, -1,  0,  0}
        };    } else {
        // For other sizes, create basic Laplacian kernel
        int center = size / 2;
        
        // Set center value to positive (number of neighbors)
        kernel[center][center] = 4.0;
        
        // Set cross pattern around center to negative
        if (center > 0) {
            kernel[center-1][center] = -1.0; // top
            kernel[center+1][center] = -1.0; // bottom
            kernel[center][center-1] = -1.0; // left
            kernel[center][center+1] = -1.0; // right
        }
    }
    
    return kernel;
}

std::vector<std::vector<double>> EdgeDetection::generateLaplacianNegativeKernel(int size) {
    std::vector<std::vector<double>> kernel(size, std::vector<double>(size, 0.0));
    
    if (size == 3) {
        // Negative 3x3 Laplacian kernel (detects dark lines on bright background)
        kernel = {
            { 0,  1,  0},
            { 1, -4,  1},
            { 0,  1,  0}
        };
    } else if (size == 5) {
        // 5x5 Negative Laplacian kernel
        kernel = {
            { 0,  0,  1,  0,  0},
            { 0,  1,  2,  1,  0},
            { 1,  2,-16,  2,  1},
            { 0,  1,  2,  1,  0},
            { 0,  0,  1,  0,  0}
        };
    } else {
        // For other sizes, create negative Laplacian kernel
        int center = size / 2;
        
        // Set center value to negative
        kernel[center][center] = -4.0;
        
        // Set cross pattern around center to positive
        if (center > 0) {
            kernel[center-1][center] = 1.0; // top
            kernel[center+1][center] = 1.0; // bottom
            kernel[center][center-1] = 1.0; // left
            kernel[center][center+1] = 1.0; // right
        }
    }
    
    return kernel;
}

std::vector<std::vector<double>> EdgeDetection::generateLoGKernel(double sigma, int size) {
    std::vector<std::vector<double>> kernel(size, std::vector<double>(size));
    
    int center = size / 2;
    double sigma2 = sigma * sigma;
    double sigma4 = sigma2 * sigma2;
    
    // Wzór Laplacian of Gaussian: LoG(x,y) = -1/(π*σ⁴) * [1 - (x²+y²)/(2*σ²)] * e^(-(x²+y²)/(2*σ²))
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            int dx = x - center;
            int dy = y - center;
            double r2 = dx * dx + dy * dy; // x² + y²
            
            double gaussianPart = std::exp(-r2 / (2.0 * sigma2));
            double logPart = -1.0 / (M_PI * sigma4) * (1.0 - r2 / (2.0 * sigma2));
            
            kernel[x][y] = logPart * gaussianPart;
        }
    }
    
    return kernel;
}

void EdgeDetection::applyLoGWithThresholding(std::unique_ptr<Image>& image, double sigma, int windowSize, double threshold) {
    int width = image->width();
    int height = image->height();
    
    // Krok 1: Oblicz rozmiar jądra LoG na podstawie sigma
    int kernelSize = std::max(3, static_cast<int>(6 * sigma + 1));
    if (kernelSize % 2 == 0) kernelSize++; // Upewnij się, że jest nieparzysty
    
    // Generowanie jądra Laplacian of Gaussian
    auto logKernel = generateLoGKernel(sigma, kernelSize);
    
    // Aplikowanie konwolucji LoG
    int kernelRadius = kernelSize / 2;
    std::vector<std::vector<double>> logResponse(width, std::vector<double>(height));
    
    // Tworzymy kopię oryginalnego obrazu
    std::vector<std::vector<QColor>> originalPixels(width, std::vector<QColor>(height));
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            originalPixels[x][y] = image->pixelAt(x, y);
        }
    }
    
    // Obliczanie odpowiedzi LoG dla każdego piksela
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double sum = 0.0;
            
            for (int kx = 0; kx < kernelSize; kx++) {
                for (int ky = 0; ky < kernelSize; ky++) {
                    int pixelX = x + kx - kernelRadius;
                    int pixelY = y + ky - kernelRadius;
                    
                    // Obsługa granic obrazu (odbicie)
                    pixelX = std::max(0, std::min(width - 1, pixelX));
                    pixelY = std::max(0, std::min(height - 1, pixelY));
                    
                    QColor pixel = originalPixels[pixelX][pixelY];
                    // Konwersja na luminancję
                    double luminance = 0.299 * pixel.red() + 0.587 * pixel.green() + 0.114 * pixel.blue();
                    
                    sum += luminance * logKernel[kx][ky];
                }
            }
            
            logResponse[x][y] = sum;
        }
    }
      // Krok 2: Znajdź zakres wartości LoG dla adaptacyjnego progowania
    double minResponse = logResponse[0][0];
    double maxResponse = logResponse[0][0];
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            minResponse = std::min(minResponse, logResponse[x][y]);
            maxResponse = std::max(maxResponse, logResponse[x][y]);
        }
    }
      // Oblicz adaptacyjny próg na podstawie zakresu wartości i parametru threshold
    double range = maxResponse - minResponse;
    double adaptiveThreshold = range * threshold; // threshold jako procent zakresu
    
    int windowRadius = windowSize / 2;
    
    // Krok 3: Dla każdego piksela (i,j) - progowanie zgodnie z algorytmem
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double currentValue = logResponse[x][y];
            
            // Pobierz okno z laplasjanu gaussowskiego
            double minVal = currentValue;
            double maxVal = currentValue;
            
            for (int wx = -windowRadius; wx <= windowRadius; wx++) {
                for (int wy = -windowRadius; wy <= windowRadius; wy++) {
                    int nx = std::max(0, std::min(width - 1, x + wx));
                    int ny = std::max(0, std::min(height - 1, y + wy));
                    
                    double val = logResponse[nx][ny];
                    minVal = std::min(minVal, val);
                    maxVal = std::max(maxVal, val);
                }
            }
            
            // Modyfikowany algorytm progowania:
            // Szukamy przejść przez zero (zero-crossings) które są charakterystyczne dla LoG
            int outputValue = 0;
              // Sprawdź czy w oknie występuje przejście przez zero z wystarczającą różnicą
            if ((maxVal - minVal) > adaptiveThreshold) {
                // Jest wystarczająca zmiana w oknie - to może być krawędź
                // Normalizuj wartość LoG do zakresu 0-255
                double normalizedValue = std::abs(currentValue - minResponse) / range * 255.0;
                outputValue = clamp(static_cast<int>(normalizedValue));
            }
            
            // Ustaw ten sam poziom dla wszystkich kanałów (obraz w skali szarości)
            image->setPixel(x, y, outputValue, outputValue, outputValue);
        }
    }
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> EdgeDetection::generateRobertsKernels() {
    // Operator Robertsa X (ukośne krawędzie)
    std::vector<std::vector<double>> robertsX = {
        {1, 0},
        {0, -1}
    };
    
    // Operator Robertsa Y (ukośne krawędzie)
    std::vector<std::vector<double>> robertsY = {
        {0, 1},
        {-1, 0}
    };
    
    return std::make_pair(robertsX, robertsY);
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> EdgeDetection::generatePrewittKernels() {
    // Operator Prewitta X (pionowe krawędzie)
    std::vector<std::vector<double>> prewittX = {
        {-1, 0, 1},
        {-1, 0, 1},
        {-1, 0, 1}
    };
    
    // Operator Prewitta Y (poziome krawędzie)
    std::vector<std::vector<double>> prewittY = {
        {-1, -1, -1},
        { 0,  0,  0},
        { 1,  1,  1}
    };
    
    return std::make_pair(prewittX, prewittY);
}

std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> EdgeDetection::generateSobelKernels() {
    // Operator Sobela X (pionowe krawędzie)
    std::vector<std::vector<double>> sobelX = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    
    // Operator Sobela Y (poziome krawędzie)
    std::vector<std::vector<double>> sobelY = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };
    
    return std::make_pair(sobelX, sobelY);
}

void EdgeDetection::applyConvolution(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& kernel) {
    int width = image->width();
    int height = image->height();
    int kernelSize = kernel.size();
    int kernelRadius = kernelSize / 2;
    
    // Tworzymy kopię oryginalnego obrazu do odczytu wartości
    std::vector<std::vector<QColor>> originalPixels(width, std::vector<QColor>(height));
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            originalPixels[x][y] = image->pixelAt(x, y);
        }
    }
    
    // Aplikowanie konwolucji
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double newR = 0.0, newG = 0.0, newB = 0.0;
            
            // Iteracja przez jądro
            for (int kx = 0; kx < kernelSize; kx++) {
                for (int ky = 0; ky < kernelSize; ky++) {
                    int pixelX = x + kx - kernelRadius;
                    int pixelY = y + ky - kernelRadius;
                    
                    // Obsługa pikseli poza granicami obrazu (odbicie)
                    pixelX = std::max(0, std::min(width - 1, pixelX));
                    pixelY = std::max(0, std::min(height - 1, pixelY));
                    
                    QColor pixel = originalPixels[pixelX][pixelY];
                    double kernelValue = kernel[kx][ky];
                    
                    newR += pixel.red() * kernelValue;
                    newG += pixel.green() * kernelValue;
                    newB += pixel.blue() * kernelValue;
                }
            }
              // Dla operatora Laplace'a bierzemy wartość absolutną dla lepszej wizualizacji
            newR = std::abs(newR);
            newG = std::abs(newG);
            newB = std::abs(newB);
            
            // Opcjonalnie: przekształcenie na skalę szarości dla lepszej wizualizacji krawędzi
            // double magnitude = 0.299 * newR + 0.587 * newG + 0.114 * newB;
            // image->setPixel(x, y, clamp(static_cast<int>(magnitude)),
            //                      clamp(static_cast<int>(magnitude)),
            //                      clamp(static_cast<int>(magnitude)));
            
            // Ograniczenie wartości do zakresu 0-255 i ustawienie nowego piksela
            image->setPixel(x, y, clamp(static_cast<int>(newR)),
                                 clamp(static_cast<int>(newG)),
                                 clamp(static_cast<int>(newB)));
        }
    }
}

void EdgeDetection::applyLaplacianGrayscaleConvolution(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& kernel) {
    int width = image->width();
    int height = image->height();
    int kernelSize = kernel.size();
    int kernelRadius = kernelSize / 2;
    
    // Tworzymy kopię oryginalnego obrazu do odczytu wartości
    std::vector<std::vector<QColor>> originalPixels(width, std::vector<QColor>(height));
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            originalPixels[x][y] = image->pixelAt(x, y);
        }
    }
    
    // Aplikowanie konwolucji
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double newR = 0.0, newG = 0.0, newB = 0.0;
            
            // Iteracja przez jądro
            for (int kx = 0; kx < kernelSize; kx++) {
                for (int ky = 0; ky < kernelSize; ky++) {
                    int pixelX = x + kx - kernelRadius;
                    int pixelY = y + ky - kernelRadius;
                    
                    // Obsługa pikseli poza granicami obrazu (odbicie)
                    pixelX = std::max(0, std::min(width - 1, pixelX));
                    pixelY = std::max(0, std::min(height - 1, pixelY));
                    
                    QColor pixel = originalPixels[pixelX][pixelY];
                    double kernelValue = kernel[kx][ky];
                    
                    newR += pixel.red() * kernelValue;
                    newG += pixel.green() * kernelValue;
                    newB += pixel.blue() * kernelValue;
                }
            }
            
            // Obliczenie magnitude dla każdego kanału i konwersja na skalę szarości
            double magnitudeR = std::abs(newR);
            double magnitudeG = std::abs(newG);
            double magnitudeB = std::abs(newB);
            
            // Konwersja na skalę szarości używając luminancji
            double magnitude = 0.299 * magnitudeR + 0.587 * magnitudeG + 0.114 * magnitudeB;
            int grayValue = clamp(static_cast<int>(magnitude));
            
            // Ustawienie tego samego poziomu szarości dla wszystkich kanałów
            image->setPixel(x, y, grayValue, grayValue, grayValue);
        }
    }
}

void EdgeDetection::applyGradientConvolution(std::unique_ptr<Image>& image, 
                                            const std::vector<std::vector<double>>& kernelX,
                                            const std::vector<std::vector<double>>& kernelY) {
    int width = image->width();
    int height = image->height();
    int kernelSizeX = kernelX.size();
    int kernelSizeY = kernelY.size();
    int kernelRadiusX = kernelSizeX / 2;
    int kernelRadiusY = kernelSizeY / 2;
    
    // Tworzymy kopię oryginalnego obrazu do odczytu wartości
    std::vector<std::vector<QColor>> originalPixels(width, std::vector<QColor>(height));
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            originalPixels[x][y] = image->pixelAt(x, y);
        }
    }
    
    // Aplikowanie konwolucji gradientowej zgodnie z dokumentacją
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // Image_x = horizontalDetection() - gradient po x
            double gxR = 0.0, gxG = 0.0, gxB = 0.0;
            // Image_y = verticalDetection() - gradient po y  
            double gyR = 0.0, gyG = 0.0, gyB = 0.0;
            
            // Iteracja przez jądro X
            for (int kx = 0; kx < kernelSizeX; kx++) {
                for (int ky = 0; ky < kernelSizeX; ky++) {
                    int pixelX = x + kx - kernelRadiusX;
                    int pixelY = y + ky - kernelRadiusX;
                    
                    // Obsługa pikseli poza granicami obrazu (odbicie)
                    pixelX = std::max(0, std::min(width - 1, pixelX));
                    pixelY = std::max(0, std::min(height - 1, pixelY));
                    
                    QColor pixel = originalPixels[pixelX][pixelY];
                    double kernelXValue = kernelX[kx][ky];
                    double kernelYValue = kernelY[kx][ky];

                    gxR += pixel.red() * kernelXValue;
                    gxG += pixel.green() * kernelXValue;
                    gxB += pixel.blue() * kernelXValue;
                }
            }
            
            // Iteracja przez jądro Y
            for (int kx = 0; kx < kernelSizeY; kx++) {
                for (int ky = 0; ky < kernelSizeY; ky++) {
                    int pixelX = x + kx - kernelRadiusY;
                    int pixelY = y + ky - kernelRadiusY;
                    
                    // Obsługa pikseli poza granicami obrazu (odbicie)
                    pixelX = std::max(0, std::min(width - 1, pixelX));
                    pixelY = std::max(0, std::min(height - 1, pixelY));
                    
                    QColor pixel = originalPixels[pixelX][pixelY];
                    double kernelYValue = kernelY[kx][ky];
                    
                    gyR += pixel.red() * kernelYValue;
                    gyG += pixel.green() * kernelYValue;
                    gyB += pixel.blue() * kernelYValue;
                }
            }
            
            // Obliczenie magnitude gradientu: I_new(i,j) := √(Image_x(i,j)² + Image_y(i,j)²)
            double magnitudeR = std::sqrt(gxR * gxR + gyR * gyR);
            double magnitudeG = std::sqrt(gxG * gxG + gyG * gyG);
            double magnitudeB = std::sqrt(gxB * gxB + gyB * gyB);
            
            // Ograniczenie wartości do zakresu 0-255 i ustawienie nowego piksela
            image->setPixel(x, y, clamp(static_cast<int>(magnitudeR)),
                                 clamp(static_cast<int>(magnitudeG)),
                                 clamp(static_cast<int>(magnitudeB)));
        }
    }
}

int EdgeDetection::clamp(int value, int min, int max) {
    return std::max(min, std::min(max, value));
}
