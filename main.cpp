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

#include "src/files/FileManager.h"
#include "src/image/Image.h"
#include "src/image/PPM.h"

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

  window.resize(800, 600);
  window.show();

  return QApplication::exec();
}
