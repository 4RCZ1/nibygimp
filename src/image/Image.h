//
// Created by maria on 30.03.2025.
//

#ifndef IMAGE_H
#define IMAGE_H
#include <QImage>

class Image {
protected:
    int m_width = 0;
    int m_height = 0;
    std::vector<QColor> m_pixels;
public:
    Image() = default;
    Image(int width, int height);
    virtual ~Image() = default;

    virtual bool load(const QString& filePath) = 0;
    virtual bool save(const QString& filePath) const = 0;

    QColor pixelAt(int x, int y) const;
    int getPixelR(int x, int y) const;
    int getPixelG(int x, int y) const;
    int getPixelB(int x, int y) const;
    void setPixel(int x, int y, int r, int g, int b);
    void setPixel(int x, int y, const QColor& color);

    QImage toQImage() const;
    int width() const { return m_width; }
    int height() const { return m_height; }
};



#endif //IMAGE_H
