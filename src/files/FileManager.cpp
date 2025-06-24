#include "FileManager.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QPixmap>

FileManager::FileManager(QWidget *parent) : QObject(parent), m_parent(parent) {}

void FileManager::openFile(QLabel *imageLabel, std::unique_ptr<Image> &image) {
  QString fileName =
      QFileDialog::getOpenFileName(m_parent, "Open File",
                                   QString(), // Default directory
                                   "PPM Files (*.ppm);;All Files (*.*)");

  if (!fileName.isEmpty()) {
    qDebug() << "Selected file:" << fileName;

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().toLower() == "ppm") {
      image = std::make_unique<PPM>(0, 0);
    } else {
      qDebug() << "Nieznany format obrazu";
      return;
    }

    if (image->load(fileName)) {
      qDebug() << "Loaded image:" << image->width() << "x" << image->height();
      QImage qImage = image->toQImage();
      QPixmap pixmap = QPixmap::fromImage(qImage);

      imageLabel->setPixmap(pixmap);
    } else {
      qDebug() << "Failed to load image";
    }
  }
}

void FileManager::saveFile(std::unique_ptr<Image> &image) {
  if (!image) {
    qDebug() << "No image to save";
    return;
  }

  QString fileName = QFileDialog::getSaveFileName(
      m_parent, "Save File", QString(), "PPM Files (*.ppm)");

  if (!fileName.isEmpty()) {
    if (!fileName.endsWith(".ppm", Qt::CaseInsensitive)) {
      fileName += ".ppm";
    }

    if (image->save(fileName)) {
      qDebug() << "Image saved successfully to" << fileName;
    } else {
      qDebug() << "Failed to save image";
    }
  }
}