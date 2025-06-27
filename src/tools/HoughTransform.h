#ifndef HOUGHTRANSFORM_H
#define HOUGHTRANSFORM_H

#include <memory>
#include <vector>
#include <cmath>
#include <utility>
#include "../image/Image.h"

class HoughTransform {
public:
    // Hough Transform for line detection - returns new image with Hough space
    static std::unique_ptr<Image> houghLineDetection(const std::unique_ptr<Image>& image, int thetaDensity = 1, bool skipEdgeDetection = false);
    
    // Draw detected lines on the original image - modifies the image in place
    static void drawDetectedLines(std::unique_ptr<Image>& image, int thetaDensity = 1, bool skipEdgeDetection = false, int threshold = 100);
    
    // Get peak lines from Hough space
    static std::vector<std::pair<double, double>> getPeakLines(const std::vector<std::vector<int>>& houghSpace, 
                                                               int thetaSize, int rhoMax, int threshold, int thetaDensity = 1);

    // Simple edge detection using Laplacian operator - modifies in place
    static void applyEdgeDetection(std::unique_ptr<Image>& image);

private:
    // Apply Laplacian operator for edge detection on 2D array
    static void applyLaplacianOnArray(std::vector<std::vector<int>>& arr, int width, int height);
};

#endif // HOUGHTRANSFORM_H
