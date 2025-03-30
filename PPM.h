// MyImage.h
#pragma once

#include <vector>
#include <string>
#include <QColor>
#include <qimage.h>

class PPM {
public:
    PPM();
    PPM(int width, int height);
    ~PPM() = default;

    [[nodiscard]] bool load(const QString& filePath);
    [[nodiscard]] bool save(const QString& filePath) const;

    [[nodiscard]] QColor pixelAt(int x, int y) const;
    void setPixel(int x, int y, const QColor& color);

    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }

    // Convert to QImage for display in Qt widgets
    [[nodiscard]] QImage toQImage() const;

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<QColor> m_pixels;
};
