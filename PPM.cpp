// Image.cpp
#include "PPM.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QImage>

PPM::PPM() = default;

PPM::PPM(int width, int height) : m_width(width), m_height(height) {
    m_pixels.resize(width * height, QColor());
}

bool PPM::load(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file:" << filePath;
        return false;
    }

    QTextStream in(&file);

    // Read magic number
    QString magicNumber = in.readLine();
    if (magicNumber != "P3") {
        qDebug() << "Unsupported PPM format (only P3 ASCII supported)";
        return false;
    }

    // Skip comments
    QString line = in.readLine();
    while (line.startsWith('#')) {
        line = in.readLine();
    }

    // Read dimensions
    QStringList dimensions = line.split(" ", Qt::SkipEmptyParts);
    qDebug() << "Dimensions:" << dimensions;
    if (dimensions.size() != 2) {
        qDebug() << "Invalid dimensions line";
        return false;
    }

    m_width = dimensions[0].toInt();
    m_height = dimensions[1].toInt();
    
    // Read max value
    int maxValue = in.readLine().toInt();
    
    // Resize pixel storage
    m_pixels.resize(m_width * m_height);

    // Read pixel data
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            QString _line = in.readLine();
            QStringList values = _line.split(" ", Qt::SkipEmptyParts);
            int r = values[0].toInt();
            int g = values[1].toInt();
            int b = values[2].toInt();

            // Scale values if maxValue is not 255
            if (maxValue != 255) {
                r = r * 255 / maxValue;
                g = g * 255 / maxValue;
                b = b * 255 / maxValue;
            }

            setPixel(x, y, QColor(r, g, b));
        }
    }

    file.close();
    return true;
}

bool PPM::save(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for writing:" << filePath;
        return false;
    }

    QTextStream out(&file);

    // Write header
    out << "P3\n";
    out << "# Created by NibyGIMP custom image class\n";
    out << m_width << " " << m_height << "\n";
    out << "255\n";  // Max value

    // Write pixel data
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            QColor color = pixelAt(x, y);
            out << color.red() << "\n";
            out << color.green() << "\n";
            out << color.blue() << "\n";
        }
    }

    file.close();
    return true;
}

QColor PPM::pixelAt(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return QColor(0, 0, 0);
    }
    return m_pixels[y * m_width + x];
}

void PPM::setPixel(int x, int y, const QColor& color) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }
    m_pixels[y * m_width + x] = color;
}

QImage PPM::toQImage() const {
    QImage image(m_width, m_height, QImage::Format_RGB32);
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            image.setPixelColor(x, y, pixelAt(x, y));
        }
    }
    
    return image;
}