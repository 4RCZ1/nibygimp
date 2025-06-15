#ifndef CUSTOMBLURDIALOG_H
#define CUSTOMBLURDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <vector>
#include "MatrixMaskWidget.h"

class CustomBlurDialog : public QDialog {
    Q_OBJECT

public:
    explicit CustomBlurDialog(QWidget *parent = nullptr);
    
    // Get the selected matrix
    std::vector<std::vector<double>> getMatrix() const;
    
    // Set initial matrix
    void setMatrix(const std::vector<std::vector<double>>& matrix);

private slots:
    void onAccept();
    void onReject();
    void onPreviewMatrix();

private:
    void setupUI();
    
    MatrixMaskWidget* m_matrixWidget;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_previewButton;
    QLabel* m_instructionLabel;
};

#endif // CUSTOMBLURDIALOG_H
