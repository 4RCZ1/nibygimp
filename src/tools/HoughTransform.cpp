#include "HoughTransform.h"
#include "Greyscale.h"
#include "EdgeDetection.h"
#include "../image/PPM.h"
#include <algorithm>
#include <QColor>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::unique_ptr<Image> HoughTransform::houghLineDetection(const std::unique_ptr<Image>& image, int thetaDensity, bool skipEdgeDetection) {
    if (!image) {
        return nullptr;
    }
    
    // 1. Przekonwertuj obraz wejściowy na skalę odcieni szarości
    int width = image->width();
    int height = image->height();
    
    // Create a working copy and convert to grayscale
    auto workingImage = std::make_unique<PPM>(width, height);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            QColor pixel = image->pixelAt(x, y);
            workingImage->setPixel(x, y, pixel.red(), pixel.green(), pixel.blue());
        }
    }
    
    // Convert to grayscale using existing function
    Greyscale::convertToGreyscale(reinterpret_cast<std::unique_ptr<Image>&>(workingImage));
    
    // Extract grayscale values into array for processing
    std::vector<std::vector<int>> processedPixels(width, std::vector<int>(height));
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            int gray = workingImage->getPixelR(x, y); // All channels are the same after grayscale conversion
            processedPixels[x][y] = gray;
        }
    }
    
    // 2. Jeżeli należy przeprowadzić wykrywanie krawędzi (getParameter("skip_edge_detection").toBool() == false), 
    // to wykryj krawędzie na obrazie (np. metodą Laplasjanu)
    if (!skipEdgeDetection) {
        applyLaplacianOnArray(processedPixels, width, height);
    }
    
    // 3. ρmax := długość przekątnej obrazu wejściowego
    double rhoMax = std::sqrt(width * width + height * height);
    
    // 4. θsize := 180 · θdensity
    int thetaSize = 180 * thetaDensity;
    
    // 5. Utwórz obraz o rozmiarach jak skali odcieni szarości, o wymiarach θsize na ρmax · 2 + 1
    std::vector<std::vector<int>> hough(thetaSize, std::vector<int>(static_cast<int>(2 * rhoMax + 1), 0));
    
    // 7. Dla każdego piksela (i, j) na obrazie wejściowym (z wykrytymi krawędziami):
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (processedPixels[i][j] > 0) {
                for (int k = 0; k < thetaSize; ++k) {
                    double theta = k * M_PI / (thetaDensity * 180.0);
                    
                    double rho = i * std::cos(theta) + j * std::sin(theta);
                    
                    int rhoIndex = static_cast<int>(std::round(rho + rhoMax));
                    if (rhoIndex >= 0 && rhoIndex < static_cast<int>(2 * rhoMax + 1)) {
                        hough[k][rhoIndex]++;
                    }
                }
            }
        }
    }
    
    // 8. Przenieś znormalizowane do przedziału [0; 255] wartości z macierzy hough do obrazu wyjściowego
    int maxHough = 0;
    for (int k = 0; k < thetaSize; ++k) {
        for (int r = 0; r < static_cast<int>(2 * rhoMax + 1); ++r) {
            if (hough[k][r] > maxHough) {
                maxHough = hough[k][r];
            }
        }
    }
    
    auto outputImage = std::make_unique<PPM>(thetaSize, static_cast<int>(2 * rhoMax + 1));
    
    for (int k = 0; k < thetaSize; ++k) {
        for (int r = 0; r < static_cast<int>(2 * rhoMax + 1); ++r) {
            int intensity = maxHough > 0 ? static_cast<int>(255.0 * hough[k][r] / maxHough) : 0;
            outputImage->setPixel(k, r, intensity, intensity, intensity);
        }
    }
    
    return outputImage;
}

void HoughTransform::drawDetectedLines(std::unique_ptr<Image>& image, int thetaDensity, bool skipEdgeDetection, int threshold) {
    if (!image) {
        return;
    }
    
    // 1. Przekonwertuj obraz wejściowy na skalę odcieni szarości
    int width = image->width();
    int height = image->height();
    
    // Create a working copy and convert to grayscale
    auto workingImage = std::make_unique<PPM>(width, height);
    for (int i = 0; i < width; ++i) { // i to współrzędna x
        for (int j = 0; j < height; ++j) { // j to współrzędna y
            QColor pixel = image->pixelAt(i, j);
            workingImage->setPixel(i, j, pixel.red(), pixel.green(), pixel.blue());
        }
    }
    
    Greyscale::convertToGreyscale(reinterpret_cast<std::unique_ptr<Image>&>(workingImage));
    
    std::vector<std::vector<int>> processedPixels(width, std::vector<int>(height));
    for (int i = 0; i < width; ++i) { // i to współrzędna x
        for (int j = 0; j < height; ++j) { // j to współrzędna y
            int gray = workingImage->getPixelR(i, j); // All channels are the same after grayscale conversion
            processedPixels[i][j] = gray;
        }
    }
    
    if (!skipEdgeDetection) {
        applyLaplacianOnArray(processedPixels, width, height);
    }
    
    // 3. ρmax := długość przekątnej obrazu wejściowego
    double rhoMax = std::sqrt(width * width + height * height);
    
    // 4. θsize := 180 · θdensity
    int thetaSize = 180 * thetaDensity;
    
    // 6. hough := macierz zerowa o wymiarach jak obraz wyjściowy
    std::vector<std::vector<int>> hough(thetaSize, std::vector<int>(static_cast<int>(2 * rhoMax + 1), 0));
    
    // 7. Dla każdego piksela (i, j) na obrazie wejściowym (z wykrytymi krawędziami):
    for (int j = 0; j < height; ++j) { // j to współrzędna y
        for (int i = 0; i < width; ++i) { // i to współrzędna x
            // Jeżeli I(i, j) > 0 (należy do jakiejś krawędzi), to dla każdego k w przedziale [0; θsize):
            if (processedPixels[i][j] > 0) { // Zmieniono próg na 0 zgodnie ze schematem
                for (int k = 0; k < thetaSize; ++k) {
                    // θ = k · π / (θdensity · 180)
                    double theta = k * M_PI / (thetaDensity * 180.0);
                    
                    // ρ = i · cos(θ) + j · sin(θ)
                    double rho = i * std::cos(theta) + j * std::sin(theta);
                    
                    // Inkrementuj hough(k, ρ + ρmax)
                    int rhoIndex = static_cast<int>(std::round(rho + rhoMax));
                    if (rhoIndex >= 0 && rhoIndex < static_cast<int>(2 * rhoMax + 1)) {
                        hough[k][rhoIndex]++;
                    }
                }
            }
        }
    }
    
    // Get peak lines and draw them on the original image
    auto lines = getPeakLines(hough, thetaSize, static_cast<int>(rhoMax), threshold, thetaDensity);
    
    for (const auto& line : lines) {
        double theta = line.first;
        double rho = line.second;
        
        // Draw line using parametric equation: i*cos(theta) + j*sin(theta) = rho
        double cosTheta = std::cos(theta);
        double sinTheta = std::sin(theta);
        
        if (std::abs(sinTheta) > 0.001) { // Not vertical line
            for (int i = 0; i < width; ++i) {
                int j = static_cast<int>((rho - i * cosTheta) / sinTheta);
                if (j >= 0 && j < height) {
                    image->setPixel(i, j, 255, 0, 0); // Red line
                }
            }
        } else { // Vertical line
            int i = static_cast<int>(rho / cosTheta);
            if (i >= 0 && i < width) {
                for (int j = 0; j < height; ++j) {
                    image->setPixel(i, j, 255, 0, 0); // Red line
                }
            }
        }
    }
}

std::vector<std::pair<double, double>> HoughTransform::getPeakLines(const std::vector<std::vector<int>>& houghSpace, 
                                                                     int thetaSize, int rhoMax, int threshold, int thetaDensity) {
    std::vector<std::pair<double, double>> lines;
    
    for (int k = 0; k < thetaSize; ++k) {
        for (int r = 0; r < static_cast<int>(houghSpace[k].size()); ++r) {
            if (houghSpace[k][r] > threshold) {
                // θ = k · π / (θdensity · 180) - zgodnie ze schematem
                double theta = k * M_PI / (thetaDensity * 180.0);
                double rho = r - rhoMax;
                lines.emplace_back(theta, rho);
            }
        }
    }
    
    return lines;
}

void HoughTransform::applyEdgeDetection(std::unique_ptr<Image>& image) {
    EdgeDetection::laplacianFilterGrayscale(image, 3);
}

void HoughTransform::applyLaplacianOnArray(std::vector<std::vector<int>>& arr, int width, int height) {
    // Generate Laplacian kernel
    std::vector<std::vector<double>> kernel = EdgeDetection::generateLaplacianKernel(3);
    int kernelSize = 3;
    int kernelRadius = kernelSize / 2;
    
    // Create a copy of the original array for convolution
    std::vector<std::vector<int>> original = arr;
    
    // Apply Laplacian convolution
    for (int j = kernelRadius; j < height - kernelRadius; j++) {
        for (int i = kernelRadius; i < width - kernelRadius; i++) {
            double sum = 0;
            
            // Apply convolution
            for (int ky = 0; ky < kernelSize; ky++) {
                for (int kx = 0; kx < kernelSize; kx++) {
                    int pixelY = j - kernelRadius + ky;
                    int pixelX = i - kernelRadius + kx;
                    sum += original[pixelY][pixelX] * kernel[ky][kx];
                }
            }
            
            // Clamp the result to valid range
            arr[j][i] = std::max(0, std::min(255, static_cast<int>(sum)));
        }
    }
}
