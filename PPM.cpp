// Image.cpp
#include "PPM.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QImage>

PPM::PPM() = default;

PPM::PPM(int width, int height) : Image(width, height) {}

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
            int i = 3;
            int r, g, b;
            while (i> 0) {
                QString _line = in.readLine();
                QStringList values = _line.split(" ", Qt::SkipEmptyParts);
                for (const QString& value : values) {
                    bool ok;
                    const int val = value.toInt(&ok);
                    if (ok) {
                        switch (i) {
                        case 3:
                            r = val;
                            break;
                        case 2:
                            g = val;
                            break;
                        case 1:
                            b = val;
                            break;
                        default:;
                        }
                        i--;
                    }
                }
            }

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