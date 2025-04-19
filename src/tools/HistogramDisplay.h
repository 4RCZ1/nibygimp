#ifndef HISTOGRAMDISPLAY_H
#define HISTOGRAMDISPLAY_H

#include <QDialog>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <array>
#include <memory>
#include "../image/Image.h"
#include "Histogram.h"

// Widget do rysowania pojedynczego histogramu
class HistogramWidget : public QWidget {
    Q_OBJECT

public:
    HistogramWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(256, 150);
    }

    void setHistogramData(const std::array<int, 256>& data, QColor color) {
        histogramData = data;
        histogramColor = color;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Znajdź maksymalną wartość w histogramie do skalowania
        int maxValue = 1; // Unikaj dzielenia przez zero
        for (int value : histogramData) {
            maxValue = std::max(maxValue, value);
        }
        
        // Rozmiary widgetu
        int width = this->width();
        int height = this->height();
        int bottom = height - 20; // Pozostaw miejsce na oś X
        
        // Rysuj tło
        painter.fillRect(rect(), Qt::white);
        
        // Rysuj ramkę
        painter.setPen(Qt::gray);
        painter.drawRect(0, 0, width - 1, height - 1);
        
        // Rysuj oś X
        painter.drawLine(0, bottom, width, bottom);
        
        // Rysuj znaczniki na osi X
        painter.setPen(Qt::black);
        painter.drawText(0, bottom + 15, "0");
        painter.drawText(width - 20, bottom + 15, "255");
        
        // Ustaw kolor histogramu
        painter.setPen(histogramColor);
        
        // Rysuj histogram
        double binWidth = static_cast<double>(width) / 256;
        for (int i = 0; i < 256; i++) {
            int binHeight = static_cast<int>((static_cast<double>(histogramData[i]) / maxValue) * bottom);
            int x1 = static_cast<int>(i * binWidth);
            int x2 = static_cast<int>((i + 1) * binWidth);
            
            // Rysuj linię histogramu
            if (binHeight > 0) {
                painter.drawLine(x1, bottom, x1, bottom - binHeight);
                if (x2 - x1 > 1) { // Jeśli słupek jest szerszy niż 1 piksel
                    painter.drawLine(x1, bottom - binHeight, x2, bottom - binHeight);
                    painter.drawLine(x2, bottom - binHeight, x2, bottom);
                }
            }
        }
    }

private:
    std::array<int, 256> histogramData{};
    QColor histogramColor = Qt::black;
};

// Główne okno dialogowe wyświetlające histogramy
class HistogramDisplay : public QDialog {
    Q_OBJECT

public:
    explicit HistogramDisplay(const std::unique_ptr<Image>& image, QWidget* parent = nullptr);

private:
    HistogramWidget* redHistogram;
    HistogramWidget* greenHistogram;
    HistogramWidget* blueHistogram;
    HistogramWidget* luminanceHistogram;
    
    QLabel* redLabel;
    QLabel* greenLabel;
    QLabel* blueLabel;
    QLabel* luminanceLabel;
    
    void setupUI();
    void calculateHistograms(const std::unique_ptr<Image>& image);
};

#endif // HISTOGRAMDISPLAY_H
