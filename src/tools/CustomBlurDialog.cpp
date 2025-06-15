#include "CustomBlurDialog.h"
#include <QMessageBox>

CustomBlurDialog::CustomBlurDialog(QWidget *parent)
    : QDialog(parent) {
    setupUI();
    setWindowTitle("Custom Matrix Blur");
    setModal(true);
    resize(400, 500);
}

void CustomBlurDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Instruction label
    m_instructionLabel = new QLabel(
        "Configure a custom 3x3 matrix for linear convolution blur.\n"
        "Use the preset buttons for common effects or enter custom values.\n"
        "For blur effects, ensure the sum of all values equals 1."
    );
    m_instructionLabel->setWordWrap(true);
    m_instructionLabel->setStyleSheet("color: #666; font-size: 11px; margin: 10px;");
    mainLayout->addWidget(m_instructionLabel);
    
    // Matrix widget
    m_matrixWidget = new MatrixMaskWidget(3, this);
    mainLayout->addWidget(m_matrixWidget);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_previewButton = new QPushButton("Preview Matrix");
    m_previewButton->setToolTip("Show matrix values in a readable format");
    
    m_cancelButton = new QPushButton("Cancel");
    m_okButton = new QPushButton("Apply");
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_previewButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(m_okButton, &QPushButton::clicked, this, &CustomBlurDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &CustomBlurDialog::onReject);
    connect(m_previewButton, &QPushButton::clicked, this, &CustomBlurDialog::onPreviewMatrix);
    
    // Set default to Gaussian blur
    m_matrixWidget->setGaussianBlur();
}

std::vector<std::vector<double>> CustomBlurDialog::getMatrix() const {
    return m_matrixWidget->getMatrix();
}

void CustomBlurDialog::setMatrix(const std::vector<std::vector<double>>& matrix) {
    m_matrixWidget->setMatrix(matrix);
}

void CustomBlurDialog::onAccept() {
    auto matrix = getMatrix();
    
    // Check if matrix has reasonable values
    double sum = 0.0;
    bool hasNonZero = false;
    
    for (const auto& row : matrix) {
        for (double val : row) {
            sum += val;
            if (std::abs(val) > 1e-6) {
                hasNonZero = true;
            }
        }
    }
    
    if (!hasNonZero) {
        QMessageBox::warning(this, "Invalid Matrix", 
                           "The matrix contains only zeros. Please enter valid values.");
        return;
    }
    
    // Warning for non-normalized blur matrices
    if (std::abs(sum - 1.0) > 0.1) {
        int ret = QMessageBox::question(this, "Matrix Sum Warning",
                                      QString("The sum of matrix values is %1.\n"
                                             "For blur effects, the sum should be close to 1.0.\n"
                                             "Do you want to continue anyway?").arg(sum),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    accept();
}

void CustomBlurDialog::onReject() {
    reject();
}

void CustomBlurDialog::onPreviewMatrix() {
    auto matrix = getMatrix();
    QString matrixText = "Current Matrix:\n\n";
    
    double sum = 0.0;
    for (int i = 0; i < 3; ++i) {
        matrixText += "[ ";
        for (int j = 0; j < 3; ++j) {
            matrixText += QString("%1").arg(matrix[i][j], 8, 'f', 3);
            if (j < 2) matrixText += ", ";
            sum += matrix[i][j];
        }
        matrixText += " ]\n";
    }
    
    matrixText += QString("\nSum: %1").arg(sum, 0, 'f', 6);
    
    if (std::abs(sum - 1.0) < 0.01) {
        matrixText += " ✓ (Good for blur)";
    } else if (std::abs(sum) < 0.01) {
        matrixText += " (Edge detection)";
    } else {
        matrixText += " (Custom effect)";
    }
    
    QMessageBox::information(this, "Matrix Preview", matrixText);
}

#include "CustomBlurDialog.moc"
