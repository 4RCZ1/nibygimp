#include "Binarization.h"
#include <algorithm>
#include <vector>
#include <cmath>

void Binarization::thresholdBinarization(std::unique_ptr<Image>& image, int threshold) {
    if (!image) return;
    
    // Upewnij się, że próg jest w zakresie 0-255
    threshold = std::max(0, std::min(255, threshold));
    
    int width = image->width();
    int height = image->height();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = image->getPixelR(x, y);
            int g = image->getPixelG(x, y);
            int b = image->getPixelB(x, y);
            
            int binaryValue = (rgbToGray(r, g, b) > threshold) ? 255 : 0;
            image->setPixel(x, y, binaryValue, binaryValue, binaryValue);
        }
    }
}

int Binarization::rgbToGray(int r, int g, int b) {
    // z Greyscale.cpp
    return static_cast<int>(0.3 * r + 0.6 * g + 0.1 * b);
}

void Binarization::otsuBinarization(std::unique_ptr<Image>& image) {
    if (!image) return;
    
    // Oblicz histogram obrazu
    std::vector<int> histogram = calculateHistogram(image);
    
    // Znajdź optymalny próg metodą Otsu
    int threshold = findOtsuThreshold(histogram);
    
    // Zastosuj binaryzację z znalezionym progiem
    thresholdBinarization(image, threshold);
}

std::vector<int> Binarization::calculateHistogram(std::unique_ptr<Image>& image) {
    std::vector<int> histogram(256, 0);
    
    int width = image->width();
    int height = image->height();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = image->getPixelR(x, y);
            int g = image->getPixelG(x, y);
            int b = image->getPixelB(x, y);
            
            int gray = rgbToGray(r, g, b);
            histogram[gray]++;
        }
    }
    
    return histogram;
}

int Binarization::findOtsuThreshold(const std::vector<int>& histogram) {
    int total = 0;
    
    // Oblicz całkowitą liczbę pikseli
    for (int i = 0; i < 256; ++i) {
        total += histogram[i];
    }
    
    if (total == 0) return 128; // Fallback
    
    double bestVariance = 0.0;
    int bestThreshold = 0;
    
    // Dla każdego możliwego progu t
    for (int t = 0; t < 255; ++t) {
        // Oblicz P0 - prawdopodobieństwo wystąpienia klasy 0 przy progu t
        int sum0 = 0;
        for (int i = 0; i <= t; ++i) {
            sum0 += histogram[i];
        }
        double P0 = static_cast<double>(sum0) / total;
        
        // Oblicz P1 - prawdopodobieństwo wystąpienia klasy 1 przy progu t
        double P1 = 1.0 - P0;
        
        if (P0 == 0.0 || P1 == 0.0) continue;
        
        // Oblicz μ0 - średnią jasność klasy 0 przy progu t
        double weightedSum0 = 0.0;
        for (int i = 0; i <= t; ++i) {
            weightedSum0 += i * histogram[i];
        }
        double mu0 = (sum0 > 0) ? weightedSum0 / sum0 : 0.0;
        
        // Oblicz μ1 - średnią jasność klasy 1 przy progu t
        double weightedSum1 = 0.0;
        int sum1 = 0;
        for (int i = t + 1; i < 256; ++i) {
            weightedSum1 += i * histogram[i];
            sum1 += histogram[i];
        }
        double mu1 = (sum1 > 0) ? weightedSum1 / sum1 : 0.0;
        
        // Oblicz wariancję międzyklasową: η(t) = P0 · P1 · (μ0 - μ1)²
        double variance = P0 * P1 * (mu0 - mu1) * (mu0 - mu1);
        
        // Znajdź t o największej wartości i przyjmij go za próg T
        if (variance > bestVariance) {
            bestVariance = variance;
            bestThreshold = t;
        }
    }
    
    return bestThreshold;
}
