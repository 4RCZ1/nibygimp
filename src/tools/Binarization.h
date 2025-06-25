#ifndef BINARIZATION_H
#define BINARIZATION_H

#include <memory>
#include "../image/Image.h"

class Binarization {
public:
    // Binaryzacja z progiem zadanym przez użytkownika
    static void thresholdBinarization(std::unique_ptr<Image>& image, int threshold = 128);
    
    // Binaryzacja metodą Otsu (automatyczne znajdowanie progu)
    static void otsuBinarization(std::unique_ptr<Image>& image);

private:
    // Funkcja pomocnicza do konwersji na skalę szarości (jeśli potrzebna)
    static int rgbToGray(int r, int g, int b);
    
    // Funkcje pomocnicze dla metody Otsu
    static std::vector<int> calculateHistogram(std::unique_ptr<Image>& image);
    static int findOtsuThreshold(const std::vector<int>& histogram);
};

#endif // BINARIZATION_H
