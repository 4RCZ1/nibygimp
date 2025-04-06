#include "Greyscale.h"
#include <cmath>
#include <algorithm>

void Greyscale::convertToGreyscale(std::unique_ptr<Image>& image) {
    int width = image->width();
    int height = image->height();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Pobranie składowych RGB
            int r = image->getPixelR(x, y);
            int g = image->getPixelG(x, y);
            int b = image->getPixelB(x, y);
            
            // Konwersja do skali szarości (formuła luminancji)
            int gray = static_cast<int>(0.299 * r + 0.587 * g + 0.114 * b);
            
            // Ustawienie tej samej wartości dla wszystkich kanałów
            image->setPixel(x, y, gray, gray, gray);
        }
    }
}

void Greyscale::adjustBrightness(std::unique_ptr<Image>& image, int value) {
    applyLUT(image, createBrightnessLUT(value));
}

void Greyscale::adjustContrast(std::unique_ptr<Image>& image, float factor) {
    applyLUT(image, createContrastLUT(factor));
}

void Greyscale::adjustGamma(std::unique_ptr<Image>& image, float gamma) {
    applyLUT(image, createGammaLUT(gamma));
}

std::array<int, 256> Greyscale::createBrightnessLUT(int value) {
    std::array<int, 256> lut;
    
    for (int i = 0; i < 256; ++i) {
        // Dodaj wartość jasności i ogranicz do zakresu 0-255
        lut[i] = std::clamp(i + value, 0, 255);
    }
    
    return lut;
}

std::array<int, 256> Greyscale::createContrastLUT(float factor) {
    std::array<int, 256> lut;
    
    for (int i = 0; i < 256; ++i) {
        // Wzór: nowa_wartość = (stara_wartość - 128) * factor + 128
        float newValue = (i - 128.0f) * factor + 128.0f;
        lut[i] = std::clamp(static_cast<int>(newValue), 0, 255);
    }
    
    return lut;
}

std::array<int, 256> Greyscale::createGammaLUT(float gamma) {
    std::array<int, 256> lut;
    
    // Zabezpieczenie przed wartościami bliskimi zeru
    if (gamma < 0.0001f) gamma = 0.0001f;
    
    for (int i = 0; i < 256; ++i) {
        // Normalizacja, korekta gamma i ponowne skalowanie
        float normalizedValue = i / 255.0f;
        float corrected = std::pow(normalizedValue, 1.0f / gamma);
        lut[i] = std::clamp(static_cast<int>(corrected * 255.0f), 0, 255);
    }
    
    return lut;
}

void Greyscale::applyLUT(std::unique_ptr<Image>& image, const std::array<int, 256>& lut) {
    int width = image->width();
    int height = image->height();
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Pobierz wartości RGB
            int r = image->getPixelR(x, y);
            int g = image->getPixelG(x, y);
            int b = image->getPixelB(x, y);
            
            // Zastosuj LUT do każdego kanału
            r = lut[r];
            g = lut[g];
            b = lut[b];
            
            // Ustaw nowe wartości
            image->setPixel(x, y, r, g, b);
        }
    }
}