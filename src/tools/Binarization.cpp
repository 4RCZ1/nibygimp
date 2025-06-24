#include "Binarization.h"
#include <algorithm>

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
