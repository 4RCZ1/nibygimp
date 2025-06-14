#include "Blur.h"
#include <algorithm>
#include <cmath>

void Blur::gaussianBlur(std::unique_ptr<Image>& image, double sigma, int kernelSize) {
    if (!image || sigma <= 0) {
        return;
    }
    
    // Obliczanie rozmiaru jądra jeśli nie został podany
    if (kernelSize <= 0) {
        kernelSize = calculateKernelSize(sigma);
    }
    
    // Upewnienie się, że rozmiar jądra jest nieparzysty
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }
    
    // Generowanie jądra Gaussa
    auto kernel = generateGaussianKernel(sigma, kernelSize);
      // Aplikowanie konwolucji
    applyConvolution(image, kernel);
}

void Blur::uniformBlur(std::unique_ptr<Image>& image, int kernelSize) {
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
    
    // Generowanie jądra równomiernego
    auto kernel = generateUniformKernel(kernelSize);
    
    // Aplikowanie konwolucji
    applyConvolution(image, kernel);
}

std::vector<std::vector<double>> Blur::generateGaussianKernel(double sigma, int size) {
    std::vector<std::vector<double>> kernel(size, std::vector<double>(size));
    double sum = 0.0;
    int center = size / 2;
    
    // Obliczanie wartości jądra według wzoru Gaussa
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            double value = gaussianFunction(x - center, y - center, sigma);
            kernel[x][y] = value;
            sum += value;
        }
    }
    
    // Normalizacja jądra (suma wszystkich elementów = 1)
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            kernel[x][y] /= sum;
        }
    }
      return kernel;
}

std::vector<std::vector<double>> Blur::generateUniformKernel(int size) {
    std::vector<std::vector<double>> kernel(size, std::vector<double>(size));
    
    // Wszystkie elementy jądra mają taką samą wartość
    double value = 1.0 / (size * size);
    
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            kernel[x][y] = value;
        }
    }
    
    return kernel;
}

int Blur::calculateKernelSize(double sigma) {
    // Rozmiar jądra = 6 * sigma + 1 (zaokrąglone do najbliższej nieparzystej liczby)
    int size = static_cast<int>(6 * sigma + 1);
    if (size % 2 == 0) {
        size++;
    }
    // Minimum 3x3
    return std::max(3, size);
}

double Blur::gaussianFunction(int x, int y, double sigma) {
    // Wzór funkcji Gaussa: (1/(2*π*σ²)) * e^(-(x²+y²)/(2*σ²))
    double exponent = -(x * x + y * y) / (2.0 * sigma * sigma);
    return (1.0 / (2.0 * M_PI * sigma * sigma)) * std::exp(exponent);
}

void Blur::applyConvolution(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& kernel) {
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
            
            // Ograniczenie wartości do zakresu 0-255 i ustawienie nowego piksela
            image->setPixel(x, y, clamp(static_cast<int>(newR)),
                                 clamp(static_cast<int>(newG)),
                                 clamp(static_cast<int>(newB)));
        }
    }
}

int Blur::clamp(int value, int min, int max) {
    return std::max(min, std::min(max, value));
}
