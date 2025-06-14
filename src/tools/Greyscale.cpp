#include "Greyscale.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

void Greyscale::convertToGreyscale(std::unique_ptr<Image> &image) {
  int width = image->width();
  int height = image->height();

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int r = image->getPixelR(x, y);
      int g = image->getPixelG(x, y);
      int b = image->getPixelB(x, y);

      int gray = static_cast<int>(0.3 * r + 0.6 * g + 0.1 * b);

      image->setPixel(x, y, gray, gray, gray);
    }
  }
}

void Greyscale::adjustBrightness(std::unique_ptr<Image> &image, float value) {
  applyLUT(image, createBrightnessLUT(value));
}

void Greyscale::adjustContrast(std::unique_ptr<Image> &image, float factor) {
  applyLUT(image, createContrastLUT(factor));
}

void Greyscale::adjustGamma(std::unique_ptr<Image> &image, float gamma) {
  applyLUT(image, createGammaLUT(gamma));
}

std::array<int, 256> Greyscale::createBrightnessLUT(float value) {
  std::array<int, 256> lut;
  for (int i = 0; i < 256; ++i) {
    float x = i / 255.0f;  // Normalize to [0,1]

    float numerator = std::exp(value * (x - 0.5f)) - std::exp(-value * 0.5f);
    float denominator = std::exp(value * 0.5f) - std::exp(-value * 0.5f);

    float result = numerator / denominator;

    // ostatecznie zawsze normalizuje tak, aby wynik był w zakresie [0, 1], więc nie da sie zmniejszyć kontrastu
    lut[i] = std::clamp(static_cast<int>(result * 255.0f), 0, 255);
  }

  return lut;
}

std::array<int, 256> Greyscale::createContrastLUT(float factor) {
  std::array<int, 256> lut;
  float midpoint = 0.5f;
  float steepness = factor * 5.0f;

  // Oblicz offset i scale dla normalizacji
  float offset = 1.0f / (1.0f + std::exp(steepness * midpoint));
  float scale = 1.0f / (1.0f / (1.0f + std::exp(-steepness * (1.0f - midpoint))) - offset);

  for (int i = 0; i < 256; ++i) {
    float x = i / 255.0f; // Normalize to [0,1]

    // Sigmoida i normalizacja (tylko gdy podnosimy kontrast)
    float sigmoid = 1.0f / (1.0f + std::exp(-steepness * (x - midpoint)));
    float result = (sigmoid - offset) * scale;
    if (factor > -1.0f && factor < 1.0f) {
      lut[i] = static_cast<int>(sigmoid * 255.0f);
    } else {
      lut[i] = std::clamp(static_cast<int>(result * 255.0f), 0, 255);
    }
  }

  return lut;
}

std::array<int, 256> Greyscale::createGammaLUT(float gamma) {
  // poprawienie niedoskonałości kamery
  std::array<int, 256> lut;

  // Zabezpieczenie przed wartościami bliskimi zeru
  if (gamma < 0.0001f)
    gamma = 0.0001f;

  for (int i = 0; i < 256; ++i) {
    // Normalizacja, korekta gamma i ponowne skalowanie
    float normalizedValue = i / 255.0f;
    float corrected = std::pow(normalizedValue, 1.0f / gamma);
    lut[i] = std::clamp(static_cast<int>(corrected * 255.0f), 0, 255);
  }

  return lut;
}

void Greyscale::applyLUT(std::unique_ptr<Image> &image,
                         const std::array<int, 256> &lut) {
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