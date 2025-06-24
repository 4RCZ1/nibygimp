#ifndef BINARIZATION_H
#define BINARIZATION_H

#include <memory>
#include "../image/Image.h"

class Binarization {
public:
    // Binaryzacja z progiem zadanym przez użytkownika
    static void thresholdBinarization(std::unique_ptr<Image>& image, int threshold = 128);

private:
    // Funkcja pomocnicza do konwersji na skalę szarości (jeśli potrzebna)
    static int rgbToGray(int r, int g, int b);
};

#endif // BINARIZATION_H
