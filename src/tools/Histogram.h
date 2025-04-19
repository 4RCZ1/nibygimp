#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <memory>
#include <array>
#include <vector>
#include "../image/Image.h"

class Histogram {
public:
    // Typy histogramów
    enum class Channel {
        RED,
        GREEN,
        BLUE,
        LUMINANCE
    };

    static std::array<int, 256> calculateHistogram(const std::unique_ptr<Image>& image, Channel channel);
    
    // Normalizacja histogramu (do wyświetlania)
    static std::vector<double> normalizeHistogram(const std::array<int, 256>& histogram, int height);
    
    static void stretchHistogram(std::unique_ptr<Image>& image);
    
    static void equalizeHistogram(std::unique_ptr<Image>& image);

private:
    static int calculateLuminance(int r, int g, int b);
    
    static std::array<int, 256> calculateCumulativeHistogram(const std::array<int, 256>& histogram);
    
    static int findMinNonZero(const std::array<int, 256>& histogram);
    
    static void applyLUT(std::unique_ptr<Image>& image, const std::array<int, 256>& lut);

    static std::array<int, 256> createEqualizationLUT(const std::unique_ptr<Image>& image, Channel channel, int totalPixels);
};

#endif // HISTOGRAM_H
