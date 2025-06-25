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
#include "src/tools/EdgeDetection.h" // Dodany include dla wykrywania krawędzi
#include "src/tools/Binarization.h" // Dodany include dla binaryzacji
#include "src/tools/Watershed.h" // Dodany include dla segmentacji wododziałowej

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

  // Dodanie separatora dla sekcji wykrywania krawędzi
  toolsMenu->addSeparator();

  // Dodanie menu dla funkcji wykrywania krawędzi
  QMenu *edgeMenu = toolsMenu->addMenu("Edge Detection");

  QAction *robertsBlurAction = edgeMenu->addAction("Roberts Edge Detection");
  QObject::connect(robertsBlurAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      EdgeDetection::robertsFilter(image);
      updateImageView();
      QMessageBox::information(nullptr, "Edge Detection", "Operator Robertsa został zastosowany.");
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja operatora Prewitta
  QAction *prewittBlurAction = edgeMenu->addAction("Prewitt Edge Detection");
  QObject::connect(prewittBlurAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      EdgeDetection::prewittFilter(image);
      updateImageView();
      QMessageBox::information(nullptr, "Edge Detection", "Operator Prewitta został zastosowany.");
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja operatora Sobela
  QAction *sobelBlurAction = edgeMenu->addAction("Sobel Edge Detection");
  QObject::connect(sobelBlurAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      EdgeDetection::sobelFilter(image);
      updateImageView();
      QMessageBox::information(nullptr, "Edge Detection", "Operator Sobela został zastosowany.");
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });


  // Akcja operatora Laplace'a z wyborem rozmiaru jądra
  QAction *laplacianAction = edgeMenu->addAction("Laplacian Edge Detection");
  QObject::connect(laplacianAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      int kernelSize = QInputDialog::getInt(&window, "Laplacian Edge Detection",
                                          "Kernel size (3, 5, 7, 9):",
                                          3, 3, 9, 2, &ok);
      if (ok) {
        EdgeDetection::laplacianFilter(image, kernelSize);
        updateImageView();
        QMessageBox::information(nullptr, "Edge Detection", "Operator Laplace'a został zastosowany.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja operatora Laplace'a w skali szarości
  QAction *laplacianGrayAction = edgeMenu->addAction("Laplacian Edge Detection (Grayscale)");
  QObject::connect(laplacianGrayAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      int kernelSize = QInputDialog::getInt(&window, "Laplacian Edge Detection (Grayscale)",
                                          "Kernel size (3, 5, 7, 9):",
                                          3, 3, 9, 2, &ok);
      if (ok) {
        EdgeDetection::laplacianFilterGrayscale(image, kernelSize);
        updateImageView();
        QMessageBox::information(nullptr, "Edge Detection", "Operator Laplace'a (skala szarości) został zastosowany.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja negatywnego operatora Laplace'a
  QAction *laplacianNegAction = edgeMenu->addAction("Laplacian Edge Detection (Negative)");
  QObject::connect(laplacianNegAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      int kernelSize = QInputDialog::getInt(&window, "Laplacian Edge Detection (Negative)",
                                          "Kernel size (3, 5, 7, 9):",
                                          3, 3, 9, 2, &ok);
      if (ok) {
        EdgeDetection::laplacianFilterNegative(image, kernelSize);
        updateImageView();
        QMessageBox::information(nullptr, "Edge Detection", "Negatywny operator Laplace'a został zastosowany.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja Laplacian of Gaussian
  QAction *logAction = edgeMenu->addAction("Laplacian of Gaussian (LoG)");
  QObject::connect(logAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok1, ok2, ok3;
      double sigma = QInputDialog::getDouble(&window, "Laplacian of Gaussian",
                                           "Sigma value (0.5 to 3.0):",
                                           1.0, 0.5, 3.0, 2, &ok1);
      if (ok1) {
        int windowSize = QInputDialog::getInt(&window, "Laplacian of Gaussian",
                                            "Window size for thresholding (3, 5, 7):",
                                            3, 3, 7, 2, &ok2);
        if (ok2) {
          double threshold = QInputDialog::getDouble(&window, "Laplacian of Gaussian",
                                                   "Threshold value (0.01 to 0.5):\n(procent zakresu wartości dla progowania)",
                                                   0.1, 0.01, 0.5, 3, &ok3);
          if (ok3) {
            EdgeDetection::laplacianOfGaussian(image, sigma, windowSize, threshold);
            updateImageView();
            QMessageBox::information(nullptr, "Edge Detection", "Laplacian of Gaussian został zastosowany.");
          }
        }
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja uproszczonego Laplacian of Gaussian
  QAction *logSimpleAction = edgeMenu->addAction("Laplacian of Gaussian (Simple)");
  QObject::connect(logSimpleAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      double sigma = QInputDialog::getDouble(&window, "Laplacian of Gaussian (Simple)",
                                           "Sigma value (0.5 to 3.0):",
                                           1.0, 0.5, 3.0, 2, &ok);
      if (ok) {
        EdgeDetection::laplacianOfGaussianSimple(image, sigma);
        updateImageView();
        QMessageBox::information(nullptr, "Edge Detection", "Uproszczony Laplacian of Gaussian został zastosowany.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja algorytmu Canny
  QAction *cannyAction = edgeMenu->addAction("Canny Edge Detection");
  QObject::connect(cannyAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok1, ok2;
      double upperThresh = QInputDialog::getDouble(&window, "Canny Edge Detection",
                                                 "Upper threshold (50.0 to 200.0):",
                                                 100.0, 50.0, 200.0, 1, &ok1);
      if (ok1) {
        double lowerThresh = QInputDialog::getDouble(&window, "Canny Edge Detection",
                                                   "Lower threshold (20.0 to 100.0):",
                                                   50.0, 20.0, 100.0, 1, &ok2);
        if (ok2) {
          EdgeDetection::cannyEdgeDetection(image, upperThresh, lowerThresh);
          updateImageView();
          QMessageBox::information(nullptr, "Edge Detection", "Algorytm Canny został zastosowany.");
        }
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Dodanie separatora dla sekcji binaryzacji
  toolsMenu->addSeparator();

  // Dodanie menu dla funkcji binaryzacji
  QMenu *binarizationMenu = toolsMenu->addMenu("Binarization");

  // Akcja binaryzacji z progiem
  QAction *thresholdBinarizationAction = binarizationMenu->addAction("Threshold Binarization");
  QObject::connect(thresholdBinarizationAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      bool ok;
      int threshold = QInputDialog::getInt(&window, "Threshold Binarization",
                                         "Threshold value (0 to 255):",
                                         128, 0, 255, 1, &ok);
      if (ok) {
        Binarization::thresholdBinarization(image, threshold);
        updateImageView();
        QMessageBox::information(nullptr, "Binarization", "Binaryzacja z progiem została zastosowana.");
      }
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Akcja binaryzacji metodą Otsu
  QAction *otsuBinarizationAction = binarizationMenu->addAction("Otsu Binarization");
  QObject::connect(otsuBinarizationAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      Binarization::otsuBinarization(image);
      updateImageView();
      QMessageBox::information(nullptr, "Binarization", "Binaryzacja metodą Otsu została zastosowana (próg automatycznie obliczony).");
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  // Dodanie separatora dla sekcji segmentacji
  toolsMenu->addSeparator();

  // Dodanie menu dla funkcji segmentacji
  QMenu *segmentationMenu = toolsMenu->addMenu("Segmentation");

  // Akcja segmentacji wododziałowej algorytmem Vincent-Soille'a
  QAction *watershedAction = segmentationMenu->addAction("Watershed Segmentation (Vincent-Soille)");
  QObject::connect(watershedAction, &QAction::triggered, &window, [&image, updateImageView, &window]() {
    if (image) {
      Greyscale::convertToGreyscale(image);
      VincentSoilleWatershed watershed(8);
      watershed.watershed(image);
      updateImageView();
      QMessageBox::information(nullptr, "Segmentation", "Segmentacja wododziałowa została zastosowana.");
    } else {
      QMessageBox::warning(nullptr, "Error", "No image loaded.");
    }
  });

  window.resize(800, 600);
  window.show();

  return QApplication::exec();
}
