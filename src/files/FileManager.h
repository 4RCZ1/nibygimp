#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QLabel>
#include <memory>
#include "../image/Image.h"
#include "../image/PPM.h"

class FileManager : public QObject {
  Q_OBJECT

public:
  explicit FileManager(QWidget* parent = nullptr);

  void openFile(QLabel* imageLabel, std::unique_ptr<Image>& image);
  void saveFile(std::unique_ptr<Image>& image);

private:
  QWidget* m_parent;
};

#endif