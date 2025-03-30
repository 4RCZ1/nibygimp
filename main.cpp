#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>

#include "PPM.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Create a main widget to hold our layout (stack-allocated)
    QWidget window;
    window.setWindowTitle("Fake GIMP");

    // Create a layout - the window takes ownership when setLayout is called
    auto *layout = new QVBoxLayout(&window);

    // Create buttons - the layout takes ownership
    auto *selectButton = new QPushButton("Select a file");
    auto *saveButton = new QPushButton("Save image");

    auto *imageLabel = new QLabel();
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    std::unique_ptr<Image> image;

    // Connect the second button to show a file dialog
    QObject::connect(selectButton, &QPushButton::clicked, &window, [&window, imageLabel, &image]() {
        QString fileName = QFileDialog::getOpenFileName(
            &window,
            "Open File",
            QString(),  // Default directory
            "PPM Files (*.ppm);;All Files (*.*)"
        );

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
                QPixmap scaledPixmap = pixmap.scaled(
                    qImage.width() * 100,
                    qImage.height() * 100,
                    Qt::KeepAspectRatio,
                    Qt::FastTransformation);

                imageLabel->setPixmap(scaledPixmap);
            } else {
                qDebug() << "Failed to load image";
            }
        }
    });

    QObject::connect(saveButton, &QPushButton::clicked, &window, [&window, &image]() {
        if (!image) {
                qDebug() << "No image to save";
                return;
            }

            QString fileName = QFileDialog::getSaveFileName(
                &window,
                "Save File",
                QString(),
                "PPM Files (*.ppm)"
            );

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
    });

    // Add buttons to layout
    layout->addWidget(saveButton);
    layout->addWidget(selectButton);
    layout->addWidget(imageLabel);

    // Set the layout on the window
    window.setLayout(layout);
    window.resize(800, 600);
    window.show();

    return QApplication::exec();
}
