#include "Image.h"

Image::Image(int width, int height) : m_width(width), m_height(height) {
  m_pixels.resize(width * height, QColor());
}

QColor Image::pixelAt(int x, int y) const {
  if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
    return {0, 0, 0};
  }
  return m_pixels[y * m_width + x];
}

int Image::getPixelR(int x, int y) const {
  return pixelAt(x, y).red();
}

int Image::getPixelG(int x, int y) const {
  return pixelAt(x, y).green();
}

int Image::getPixelB(int x, int y) const {
  return pixelAt(x, y).blue();
}

void Image::setPixel(int x, int y, int r, int g, int b) {
  setPixel(x, y, QColor(r, g, b));
}

void Image::setPixel(int x, int y, const QColor& color) {
  if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
    return;
  }
  m_pixels[y * m_width + x] = color;
}


QImage Image::toQImage() const {
  QImage image(m_width, m_height, QImage::Format_RGB32);

  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      image.setPixelColor(x, y, pixelAt(x, y));
    }
  }

  return image;
}
