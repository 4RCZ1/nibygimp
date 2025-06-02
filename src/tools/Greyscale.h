#ifndef GREYSCALE_H
#define GREYSCALE_H

#include <memory>
#include <array>
#include "../image/Image.h"

class Greyscale {
public:
  // Konwersja do skali szarości
  static void convertToGreyscale(std::unique_ptr<Image>& image);

  // Funkcje modyfikujące obraz przy użyciu LUT
  static void adjustBrightness(std::unique_ptr<Image>& image, float value);
  static void adjustContrast(std::unique_ptr<Image>& image, float factor);
  static void adjustGamma(std::unique_ptr<Image>& image, float gamma);

private:
  // Funkcje tworzące tablice LUT
  static std::array<int, 256> createBrightnessLUT(float value);
  static std::array<int, 256> createContrastLUT(float factor);
  static std::array<int, 256> createGammaLUT(float gamma);

  // Funkcja aplikująca LUT do obrazu
  static void applyLUT(std::unique_ptr<Image>& image, const std::array<int, 256>& lut);
};

#endif //GREYSCALE_H