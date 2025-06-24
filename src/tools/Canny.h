#ifndef CANNY_H
#define CANNY_H

#include "../image/Image.h"
#include <memory>
#include <vector>

class Canny {
public:
    // Główna funkcja algorytmu Canny
    static void applyCanny(std::unique_ptr<Image>& image, double upperThresh = 50.0, double lowerThresh = 20.0);
    
    // 6 kroków algorytmu Canny zgodnie z instrukcją
    static void step1_convertToGrayscale(std::unique_ptr<Image>& image);
    static void step2_applyGaussianBlur(std::unique_ptr<Image>& image);
    static void step3_computeSobelGradients(std::unique_ptr<Image>& image,
                                           std::vector<std::vector<double>>& gradientX,
                                           std::vector<std::vector<double>>& gradientY);
    static void step4_computeMagnitudeAndDirection(const std::vector<std::vector<double>>& gradientX,
                                                  const std::vector<std::vector<double>>& gradientY,
                                                  std::vector<std::vector<double>>& magnitude,
                                                  std::vector<std::vector<double>>& direction);
    static void step5_nonMaximumSuppression(const std::vector<std::vector<double>>& magnitude,
                                           const std::vector<std::vector<double>>& direction,
                                           std::vector<std::vector<bool>>& strongEdges,
                                           double upperThresh);
    static void step6_hysteresisThresholding(const std::vector<std::vector<double>>& magnitude,
                                            const std::vector<std::vector<double>>& direction,
                                            const std::vector<std::vector<bool>>& strongEdges,
                                            std::vector<std::vector<bool>>& finalEdges,
                                            double lowerThresh);

private:
    // Funkcje pomocnicze
    static std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> generateSobelKernels();
    static int getDirectionSector(double angle);
    static std::pair<std::pair<int, int>, std::pair<int, int>> getNeighborsForDirection(int sector);
};

#endif // CANNY_H
