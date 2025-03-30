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
    window.setWindowTitle("FakeGIMP");

    // Create a layout - the window takes ownership when setLayout is called
    auto *layout = new QVBoxLayout(&window);

    // Create buttons - the layout takes ownership
    auto *button2 = new QPushButton("Select a file");

    auto *imageLabel = new QLabel();
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Connect the second button to show a file dialog
    QObject::connect(button2, &QPushButton::clicked, &window, [&window, imageLabel]() {
        QString fileName = QFileDialog::getOpenFileName(
            &window,
            "Open File",
            QString(),  // Default directory
            "PPM Files (*.ppm);;All Files (*.*)"
        );

        if (!fileName.isEmpty()) {
            qDebug() << "Selected file:" << fileName;

            PPM image;
            if (image.load(fileName)) {
                qDebug() << "Loaded image:" << image.width() << "x" << image.height();
                QImage qImage = image.toQImage();
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

    // Add buttons to layout
    layout->addWidget(button2);
    layout->addWidget(imageLabel);

    // Set the layout on the window
    window.setLayout(layout);
    window.resize(800, 600);
    window.show();

    return QApplication::exec();
}
