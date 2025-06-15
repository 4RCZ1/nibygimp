#include "MatrixMaskWidget.h"
#include <QGroupBox>
#include <QMessageBox>
#include <cmath>

MatrixMaskWidget::MatrixMaskWidget(int size, QWidget *parent)
    : QWidget(parent), m_size(size) {
    setupUI();
    resetToZero();
}

void MatrixMaskWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // Title
    m_titleLabel = new QLabel(QString("Matrix %1x%1").arg(m_size));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    m_mainLayout->addWidget(m_titleLabel);
    
    // Matrix grid
    QGroupBox* matrixGroup = new QGroupBox("Matrix Values");
    m_gridLayout = new QGridLayout(matrixGroup);
    
    // Initialize spin boxes
    m_spinBoxes.resize(m_size);
    for (int i = 0; i < m_size; ++i) {
        m_spinBoxes[i].resize(m_size);
        for (int j = 0; j < m_size; ++j) {
            m_spinBoxes[i][j] = new QDoubleSpinBox();
            m_spinBoxes[i][j]->setRange(-99.99, 99.99);
            m_spinBoxes[i][j]->setDecimals(3);
            m_spinBoxes[i][j]->setSingleStep(0.1);
            m_spinBoxes[i][j]->setFixedWidth(80);
            
            connect(m_spinBoxes[i][j], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &MatrixMaskWidget::onValueChanged);
            
            m_gridLayout->addWidget(m_spinBoxes[i][j], i, j);
        }
    }
    
    m_mainLayout->addWidget(matrixGroup);
    
    // Control buttons
    QGroupBox* controlGroup = new QGroupBox("Controls");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    
    // First row of buttons
    QHBoxLayout* row1Layout = new QHBoxLayout();
    m_resetIdentityBtn = new QPushButton("Identity");
    m_resetZeroBtn = new QPushButton("Zero");
    m_normalizeBtn = new QPushButton("Normalize");
    
    row1Layout->addWidget(m_resetIdentityBtn);
    row1Layout->addWidget(m_resetZeroBtn);
    row1Layout->addWidget(m_normalizeBtn);
    
    // Second row of buttons (presets)
    QHBoxLayout* row2Layout = new QHBoxLayout();
    m_gaussianBtn = new QPushButton("Gaussian");
    m_boxBtn = new QPushButton("Box Blur");
    m_edgeBtn = new QPushButton("Edge");
    
    row2Layout->addWidget(m_gaussianBtn);
    row2Layout->addWidget(m_boxBtn);
    row2Layout->addWidget(m_edgeBtn);
    
    // Third row of buttons (more presets)
    QHBoxLayout* row3Layout = new QHBoxLayout();
    m_sharpenBtn = new QPushButton("Sharpen");
    m_embossBtn = new QPushButton("Emboss");
    
    row3Layout->addWidget(m_sharpenBtn);
    row3Layout->addWidget(m_embossBtn);
    row3Layout->addStretch();
    
    controlLayout->addLayout(row1Layout);
    controlLayout->addLayout(row2Layout);
    controlLayout->addLayout(row3Layout);
    
    m_mainLayout->addWidget(controlGroup);
    
    // Connect buttons
    connect(m_resetIdentityBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onResetToIdentity);
    connect(m_resetZeroBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onResetToZero);
    connect(m_normalizeBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onNormalize);
    connect(m_gaussianBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onPresetGaussian);
    connect(m_boxBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onPresetBox);
    connect(m_edgeBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onPresetEdge);
    connect(m_sharpenBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onPresetSharpen);
    connect(m_embossBtn, &QPushButton::clicked, this, &MatrixMaskWidget::onPresetEmboss);
}

std::vector<std::vector<double>> MatrixMaskWidget::getMatrix() const {
    std::vector<std::vector<double>> matrix(m_size, std::vector<double>(m_size));
    
    for (int i = 0; i < m_size; ++i) {
        for (int j = 0; j < m_size; ++j) {
            matrix[i][j] = m_spinBoxes[i][j]->value();
        }
    }
    
    return matrix;
}

void MatrixMaskWidget::setMatrix(const std::vector<std::vector<double>>& matrix) {
    if (matrix.size() != static_cast<size_t>(m_size) || 
        matrix[0].size() != static_cast<size_t>(m_size)) {
        return;
    }
    
    for (int i = 0; i < m_size; ++i) {
        for (int j = 0; j < m_size; ++j) {
            m_spinBoxes[i][j]->setValue(matrix[i][j]);
        }
    }
}

void MatrixMaskWidget::resetToIdentity() {
    std::vector<std::vector<double>> identity(m_size, std::vector<double>(m_size, 0.0));
    int center = m_size / 2;
    identity[center][center] = 1.0;
    setMatrix(identity);
}

void MatrixMaskWidget::resetToZero() {
    std::vector<std::vector<double>> zero(m_size, std::vector<double>(m_size, 0.0));
    setMatrix(zero);
}

void MatrixMaskWidget::normalizeMatrix() {
    auto matrix = getMatrix();
    double sum = 0.0;
    
    // Calculate sum
    for (int i = 0; i < m_size; ++i) {
        for (int j = 0; j < m_size; ++j) {
            sum += matrix[i][j];
        }
    }
    
    if (std::abs(sum) < 1e-6) {
        QMessageBox::warning(this, "Warning", "Cannot normalize: sum is zero or too close to zero.");
        return;
    }
    
    // Normalize
    for (int i = 0; i < m_size; ++i) {
        for (int j = 0; j < m_size; ++j) {
            matrix[i][j] /= sum;
        }
    }
    
    setMatrix(matrix);
}

void MatrixMaskWidget::setGaussianBlur() {
    if (m_size == 3) {
        std::vector<std::vector<double>> gaussian = {
            {1.0/16, 2.0/16, 1.0/16},
            {2.0/16, 4.0/16, 2.0/16},
            {1.0/16, 2.0/16, 1.0/16}
        };
        setMatrix(gaussian);
    } else {
        // For other sizes, generate Gaussian kernel
        double sigma = m_size / 6.0;
        std::vector<std::vector<double>> gaussian(m_size, std::vector<double>(m_size));
        double sum = 0.0;
        int center = m_size / 2;
        
        for (int x = 0; x < m_size; x++) {
            for (int y = 0; y < m_size; y++) {
                int dx = x - center;
                int dy = y - center;
                double value = std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
                gaussian[x][y] = value;
                sum += value;
            }
        }
        
        // Normalize
        for (int x = 0; x < m_size; x++) {
            for (int y = 0; y < m_size; y++) {
                gaussian[x][y] /= sum;
            }
        }
        
        setMatrix(gaussian);
    }
}

void MatrixMaskWidget::setBoxBlur() {
    double value = 1.0 / (m_size * m_size);
    std::vector<std::vector<double>> box(m_size, std::vector<double>(m_size, value));
    setMatrix(box);
}

void MatrixMaskWidget::setEdgeDetection() {
    if (m_size == 3) {
        std::vector<std::vector<double>> edge = {
            {-1, -1, -1},
            {-1,  8, -1},
            {-1, -1, -1}
        };
        setMatrix(edge);
    } else {
        // For other sizes, create edge detection kernel
        std::vector<std::vector<double>> edge(m_size, std::vector<double>(m_size, -1.0));
        int center = m_size / 2;
        edge[center][center] = (m_size * m_size) - 1;
        setMatrix(edge);
    }
}

void MatrixMaskWidget::setSharpen() {
    if (m_size == 3) {
        std::vector<std::vector<double>> sharpen = {
            { 0, -1,  0},
            {-1,  5, -1},
            { 0, -1,  0}
        };
        setMatrix(sharpen);
    } else {
        // For other sizes, create sharpening kernel
        std::vector<std::vector<double>> sharpen(m_size, std::vector<double>(m_size, 0.0));
        int center = m_size / 2;
        sharpen[center][center] = 5.0;
        if (center > 0) {
            sharpen[center-1][center] = -1.0;
            sharpen[center+1][center] = -1.0;
            sharpen[center][center-1] = -1.0;
            sharpen[center][center+1] = -1.0;
        }
        setMatrix(sharpen);
    }
}

void MatrixMaskWidget::setEmboss() {
    if (m_size == 3) {
        std::vector<std::vector<double>> emboss = {
            {-2, -1,  0},
            {-1,  1,  1},
            { 0,  1,  2}
        };
        setMatrix(emboss);
    } else {
        // For other sizes, create emboss kernel
        std::vector<std::vector<double>> emboss(m_size, std::vector<double>(m_size, 0.0));
        int center = m_size / 2;
        
        for (int i = 0; i < m_size; ++i) {
            for (int j = 0; j < m_size; ++j) {
                if (i < center && j < center) {
                    emboss[i][j] = -1.0;
                } else if (i > center && j > center) {
                    emboss[i][j] = 1.0;
                }
            }
        }
        emboss[center][center] = 1.0;
        setMatrix(emboss);
    }
}

void MatrixMaskWidget::onValueChanged() {
    emit matrixChanged();
}

void MatrixMaskWidget::onResetToIdentity() {
    resetToIdentity();
}

void MatrixMaskWidget::onResetToZero() {
    resetToZero();
}

void MatrixMaskWidget::onNormalize() {
    normalizeMatrix();
}

void MatrixMaskWidget::onPresetGaussian() {
    setGaussianBlur();
}

void MatrixMaskWidget::onPresetBox() {
    setBoxBlur();
}

void MatrixMaskWidget::onPresetEdge() {
    setEdgeDetection();
}

void MatrixMaskWidget::onPresetSharpen() {
    setSharpen();
}

void MatrixMaskWidget::onPresetEmboss() {
    setEmboss();
}

#include "MatrixMaskWidget.moc"
