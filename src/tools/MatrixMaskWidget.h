#ifndef MATRIXMASKWIDGET_H
#define MATRIXMASKWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>

class MatrixMaskWidget : public QWidget {
    Q_OBJECT

public:
    explicit MatrixMaskWidget(int size = 3, QWidget *parent = nullptr);
    
    // Get the current matrix values
    std::vector<std::vector<double>> getMatrix() const;
    
    // Set matrix values
    void setMatrix(const std::vector<std::vector<double>>& matrix);
    
    // Reset to identity matrix
    void resetToIdentity();
    
    // Reset to zero matrix
    void resetToZero();
    
    // Normalize matrix (sum of all elements = 1)
    void normalizeMatrix();
    
    // Set common blur matrices
    void setGaussianBlur();
    void setBoxBlur();
    void setEdgeDetection();
    void setSharpen();
    void setEmboss();
    
    // Get matrix size
    int getSize() const { return m_size; }

signals:
    void matrixChanged();

private slots:
    void onValueChanged();
    void onResetToIdentity();
    void onResetToZero();
    void onNormalize();
    void onPresetGaussian();
    void onPresetBox();
    void onPresetEdge();
    void onPresetSharpen();
    void onPresetEmboss();

private:
    void setupUI();
    void updateMatrix();

    int m_size;
    QGridLayout* m_gridLayout;
    QVBoxLayout* m_mainLayout;
    std::vector<std::vector<QDoubleSpinBox*>> m_spinBoxes;
    QLabel* m_titleLabel;
    QPushButton* m_resetIdentityBtn;
    QPushButton* m_resetZeroBtn;
    QPushButton* m_normalizeBtn;
    QPushButton* m_gaussianBtn;
    QPushButton* m_boxBtn;
    QPushButton* m_edgeBtn;
    QPushButton* m_sharpenBtn;
    QPushButton* m_embossBtn;
};

#endif // MATRIXMASKWIDGET_H
