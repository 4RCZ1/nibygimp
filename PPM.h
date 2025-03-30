// MyImage.h
#pragma once

#include "Image.h"

#include <QColor>
#include <qimage.h>
#include <string>
#include <vector>

class PPM : public Image {
public:
    PPM();
    PPM(int width, int height);

    bool load(const QString& filePath) override;
    bool save(const QString &filePath) const override;
};
