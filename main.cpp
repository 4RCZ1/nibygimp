#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>

#include "src/files/FileManager.h"
#include "src/image/Image.h"
#include "src/image/PPM.h"
#include "src/tools/Greyscale.h" // Dodany include
#include "src/tools/Histogram.h" // Dodany include dla histogramu
#include "src/tools/HistogramDisplay.h" // Dodany include dla wyświetlania histogramu
#include "src/tools/Blur.h" // Dodany include dla rozmycia
#include "src/tools/CustomBlurDialog.h" // Dodany include dla niestandardowego rozmycia

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QMainWindow window;
  window.setWindowTitle("Fake GIMP");
  auto fileManager = new FileManager(&window);
  auto *centralWidget = new QWidget(&window);

  auto *layout = new QVBoxLayout(centralWidget);

  auto *imageLabel = new QLabel();
  imageLabel->setAlignment(Qt::AlignCenter);
  imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  std::unique_ptr<Image> image;

  layout->addWidget(imageLabel);

  centralWidget->setLayout(layout);
  window.setCentralWidget(centralWidget);

  QMenuBar *menuBar = window.menuBar();
  QMenu *fileMenu = menuBar->addMenu("File");
  QMenu *toolsMenu = menuBar->addMenu("Tools");

  QAction *openAction = fileMenu->addAction("Open");
  QObject::connect(openAction, &QAction::triggered, &window,
                   [&window, imageLabel, &image, fileManager]() {
                     fileManager->openFile(imageLabel, image);
                   });

  QAction *saveAction = fileMenu->addAction("Save");
  QObject::connect(
      saveAction, &QAction::triggered, &window,
      [&window, &image, fileManager]() { fileManager->saveFile(image); });
  fileMenu->addSeparator();

  // Funkcja pomocnicza do aktualizacji widoku
  auto updateImageView = [&image, imageLabel]() {
    if (image) {
      imageLabel->setPixmap(QPixmap::fromImage(image->toQImage()));
    }
  };

  // Dodanie akcji dla konwersji na skalę szarości
  QAction *grayscaleAction = toolsMenu->addAction("Convert to Grayscale");
  QObject::connect(grayscaleAction, &QAction::triggered, &window, [&image, updateImageView]() {
    if (image) {
      Greyscale::convertToGreyscale(image);
      updateImageView();
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Dodanie akcji dla regulacji jasności
  QAction *brightnessAction = toolsMenu->addAction("Adjust Brightness");
  QObject::connect(brightnessAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      double value = QInputDialog::getDouble(&window, "Adjust Brightness",
                                      "Brightness (-10 to 10):",
                                      1.0, -10.0, 10.0, 2, &ok);
      if (ok) {
        Greyscale::adjustBrightness(image, value);
        updateImageView();
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Dodanie akcji dla regulacji kontrastu
  QAction *contrastAction = toolsMenu->addAction("Adjust Contrast");
  QObject::connect(contrastAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      double factor = QInputDialog::getDouble(&window, "Adjust Contrast",
                                          "Contrast factor (-10 to 10):",
                                          1.0, -10.0, 10.0, 2, &ok);
      if (ok) {
        Greyscale::adjustContrast(image, static_cast<float>(factor));
        updateImageView();
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Dodanie akcji dla regulacji gamma
  QAction *gammaAction = toolsMenu->addAction("Adjust Gamma");
  QObject::connect(gammaAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      double gamma = QInputDialog::getDouble(&window, "Adjust Gamma",
                                         "Gamma value (0.1 to 5.0):",
                                         1.0, 0.1, 5.0, 2, &ok);
      if (ok) {
        Greyscale::adjustGamma(image, static_cast<float>(gamma));
        updateImageView();
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });
  
  // Dodanie separatora dla sekcji histogramu
  toolsMenu->addSeparator();
  
  // Dodanie menu dla funkcji histogramu
  QMenu *histogramMenu = toolsMenu->addMenu("Histogram");
  
  // Akcja wyświetlania histogramu
  QAction *showHistogramAction = histogramMenu->addAction("Show Histogram");
  QObject::connect(showHistogramAction, &QAction::triggered, &window, [&image, &window]() {
    if (image) {
      // Utworzenie i wyświetlenie okna z histogramami
      auto histogramDialog = new HistogramDisplay(image, &window);
      histogramDialog->setAttribute(Qt::WA_DeleteOnClose); // Automatyczne usuwanie okna po zamknięciu
      histogramDialog->show();
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });
  
  // Akcja rozciągania histogramu
  QAction *stretchHistogramAction = histogramMenu->addAction("Stretch Histogram");
  QObject::connect(stretchHistogramAction, &QAction::triggered, &window, [&image, updateImageView]() {
    if (image) {
      Histogram::stretchHistogram(image);
      updateImageView();
      QMessageBox::information(nullptr, "Histogram", "Histogram został rozciągnięty.");
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });
  
  // Akcja wyrównywania histogramu
  QAction *equalizeHistogramAction = histogramMenu->addAction("Equalize Histogram");
  QObject::connect(equalizeHistogramAction, &QAction::triggered, &window, [&image, updateImageView]() {
    if (image) {
      Histogram::equalizeHistogram(image);
      updateImageView();
      QMessageBox::information(nullptr, "Histogram", "Histogram został wyrównany.");
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Dodanie separatora dla sekcji rozmycia
  toolsMenu->addSeparator();
  
  // Dodanie menu dla funkcji rozmycia
  QMenu *blurMenu = toolsMenu->addMenu("Blur");
  
  // Akcja rozmycia Gaussa z wyborem parametrów
  QAction *gaussianBlurAction = blurMenu->addAction("Gaussian Blur");
  QObject::connect(gaussianBlurAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      double sigma = QInputDialog::getDouble(&window, "Gaussian Blur",
                                           "Sigma value (0.5 to 10.0):",
                                           1.0, 0.5, 10.0, 2, &ok);
      if (ok) {
        Blur::gaussianBlur(image, sigma);
        updateImageView();
        QMessageBox::information(nullptr, "Blur", "Rozmycie Gaussa zostało zastosowane.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });
  
  // Akcja rozmycia równomiernego z wyborem parametrów
  QAction *uniformBlurAction = blurMenu->addAction("Uniform Blur");
  QObject::connect(uniformBlurAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      int kernelSize = QInputDialog::getInt(&window, "Uniform Blur",
                                          "Kernel size (3 to 15):",
                                          5, 3, 15, 2, &ok);
      if (ok) {
        Blur::uniformBlur(image, kernelSize);
        updateImageView();
        QMessageBox::information(nullptr, "Blur", "Rozmycie równomierne zostało zastosowane.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });
  
  // Akcja niestandardowego rozmycia z macierzą 3x3
  QAction *customBlurAction = blurMenu->addAction("Custom Matrix Blur");
  QObject::connect(customBlurAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      CustomBlurDialog dialog(&window);
      if (dialog.exec() == QDialog::Accepted) {
        auto matrix = dialog.getMatrix();
        Blur::customMatrixBlur(image, matrix);
        updateImageView();
        QMessageBox::information(nullptr, "Blur", "Niestandardowe rozmycie zostało zastosowane.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  window.resize(800, 600);
  window.show();

  return QApplication::exec();
}
