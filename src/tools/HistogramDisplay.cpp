#include "HistogramDisplay.h"

HistogramDisplay::HistogramDisplay(const std::unique_ptr<Image>& image, QWidget* parent)
    : QDialog(parent) {
    
    setWindowTitle("Image Histogram");
    setMinimumSize(650, 500);
    
    setupUI();
    calculateHistograms(image);
}

void HistogramDisplay::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    
    redHistogram = new HistogramWidget(this);
    greenHistogram = new HistogramWidget(this);
    blueHistogram = new HistogramWidget(this);
    luminanceHistogram = new HistogramWidget(this);
    
    redLabel = new QLabel("Red Channel", this);
    redLabel->setStyleSheet("color: red; font-weight: bold;");
    greenLabel = new QLabel("Green Channel", this);
    greenLabel->setStyleSheet("color: green; font-weight: bold;");
    blueLabel = new QLabel("Blue Channel", this);
    blueLabel->setStyleSheet("color: blue; font-weight: bold;");
    luminanceLabel = new QLabel("Luminance", this);
    luminanceLabel->setStyleSheet("font-weight: bold;");
    
    auto* redLayout = new QVBoxLayout();
    redLayout->addWidget(redLabel);
    redLayout->addWidget(redHistogram);
    
    auto* greenLayout = new QVBoxLayout();
    greenLayout->addWidget(greenLabel);
    greenLayout->addWidget(greenHistogram);
    
    auto* blueLayout = new QVBoxLayout();
    blueLayout->addWidget(blueLabel);
    blueLayout->addWidget(blueHistogram);
    
    auto* luminanceLayout = new QVBoxLayout();
    luminanceLayout->addWidget(luminanceLabel);
    luminanceLayout->addWidget(luminanceHistogram);
    
    mainLayout->addLayout(redLayout);
    mainLayout->addLayout(greenLayout);
    mainLayout->addLayout(blueLayout);
    mainLayout->addLayout(luminanceLayout);
    
    auto* closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

void HistogramDisplay::calculateHistograms(const std::unique_ptr<Image>& image) {
    if (!image) {
        return;
    }
    
    auto redData = Histogram::calculateHistogram(image, Histogram::Channel::RED);
    auto greenData = Histogram::calculateHistogram(image, Histogram::Channel::GREEN);
    auto blueData = Histogram::calculateHistogram(image, Histogram::Channel::BLUE);
    auto luminanceData = Histogram::calculateHistogram(image, Histogram::Channel::LUMINANCE);
    
    redHistogram->setHistogramData(redData, Qt::red);
    greenHistogram->setHistogramData(greenData, Qt::green);
    blueHistogram->setHistogramData(blueData, Qt::blue);
    luminanceHistogram->setHistogramData(luminanceData, Qt::black);
}
