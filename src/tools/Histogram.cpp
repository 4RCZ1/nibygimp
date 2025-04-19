#include "Histogram.h"
#include <algorithm>
#include <cmath>

std::array<int, 256> Histogram::calculateHistogram(const std::unique_ptr<Image>& image, Channel channel) {
    std::array<int, 256> histogram{};
    histogram.fill(0);
    
    int width = image->width();
    int height = image->height();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int value = 0;
            
            switch (channel) {
                case Channel::RED:
                    value = image->getPixelR(x, y);
                    break;
                case Channel::GREEN:
                    value = image->getPixelG(x, y);
                    break;
                case Channel::BLUE:
                    value = image->getPixelB(x, y);
                    break;
                case Channel::LUMINANCE:
                    value = calculateLuminance(
                        image->getPixelR(x, y),
                        image->getPixelG(x, y),
                        image->getPixelB(x, y)
                    );
                    break;
            }
            
            histogram[value]++;
        }
    }
    
    return histogram;
}

std::vector<double> Histogram::normalizeHistogram(const std::array<int, 256>& histogram, int height) {
    int maxValue = *std::max_element(histogram.begin(), histogram.end());
    
    std::vector<double> normalizedHistogram(256);
    
    if (maxValue > 0) {
        for (int i = 0; i < 256; ++i) {
            normalizedHistogram[i] = (static_cast<double>(histogram[i]) / maxValue) * height;
        }
    }
    
    return normalizedHistogram;
}

void Histogram::stretchHistogram(std::unique_ptr<Image>& image) {
    auto histogram = calculateHistogram(image, Channel::LUMINANCE);
    
    int minValue = 255;
    int maxValue = 0;
    
    for (int i = 0; i < 256; ++i) {
        if (histogram[i] > 0) {
            minValue = std::min(minValue, i);
            maxValue = std::max(maxValue, i);
        }
    }
    
    if (minValue == 0 && maxValue == 255) {
        return;
    }
    
    std::array<int, 256> lut;
    float scale = 255.0f / (maxValue - minValue);
    
    for (int i = 0; i < 256; ++i) {
        if (i < minValue) {
            lut[i] = 0;
        } else if (i > maxValue) {
            lut[i] = 255;
        } else {
            // Linear scaling
            lut[i] = std::clamp(static_cast<int>((i - minValue) * scale), 0, 255);
        }
    }
    
    applyLUT(image, lut);
}

std::array<int, 256> Histogram::createEqualizationLUT(const std::unique_ptr<Image>& image, Channel channel, int totalPixels) {
    std::array<int, 256> lut;

    auto histogram = calculateHistogram(image, channel);
    auto cdf = calculateCumulativeHistogram(histogram);
    int cdfMin = findMinNonZero(cdf);

    for (int i = 0; i < 256; ++i) {
        float normalized = static_cast<float>(cdf[i] - cdfMin) / (totalPixels - cdfMin);
        lut[i] = std::clamp(static_cast<int>(normalized * 255.0f), 0, 255);
    }

    return lut;
}

void Histogram::equalizeHistogram(std::unique_ptr<Image>& image) {
    int totalPixels = image->width() * image->height();
    
    std::array<int, 256> lutR, lutG, lutB;
    
    lutR = createEqualizationLUT(image, Channel::RED, totalPixels);
    lutG = createEqualizationLUT(image, Channel::GREEN, totalPixels);
    lutB = createEqualizationLUT(image, Channel::BLUE, totalPixels);
    
    int width = image->width();
    int height = image->height();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = image->getPixelR(x, y);
            int g = image->getPixelG(x, y);
            int b = image->getPixelB(x, y);
            
            r = lutR[r];
            g = lutG[g];
            b = lutB[b];
            
            image->setPixel(x, y, r, g, b);
        }
    }
}

int Histogram::calculateLuminance(int r, int g, int b) {
    // Use the standard formula for luminance (same as in Greyscale)
    return static_cast<int>(0.3 * r + 0.6 * g + 0.1 * b);
}

std::array<int, 256> Histogram::calculateCumulativeHistogram(const std::array<int, 256>& histogram) {
    std::array<int, 256> cdf{};
    
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }
    
    return cdf;
}

int Histogram::findMinNonZero(const std::array<int, 256>& histogram) {
    for (int i = 0; i < 256; ++i) {
        if (histogram[i] > 0) {
            return histogram[i];
        }
    }
    return 0;
}

void Histogram::applyLUT(std::unique_ptr<Image>& image, const std::array<int, 256>& lut) {
    int width = image->width();
    int height = image->height();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = image->getPixelR(x, y);
            int g = image->getPixelG(x, y);
            int b = image->getPixelB(x, y);
            
            r = lut[r];
            g = lut[g];
            b = lut[b];
            
            image->setPixel(x, y, r, g, b);
        }
    }
}
