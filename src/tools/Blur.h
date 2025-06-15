#ifndef BLUR_H
#define BLUR_H

#include <memory>
#include <vector>
#include <cmath>
#include "../image/Image.h"

class Blur {
public:
    // Funkcja główna dla rozmycia Gaussa
    static void gaussianBlur(std::unique_ptr<Image>& image, double sigma, int kernelSize = 0);
    
    // Funkcja dla rozmycia równomiernego (box blur)
    static void uniformBlur(std::unique_ptr<Image>& image, int kernelSize);
    
    // Funkcja dla niestandardowego rozmycia z zadaną macierzą
    static void customMatrixBlur(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& matrix);

private:
    // Generowanie jądra Gaussa
    static std::vector<std::vector<double>> generateGaussianKernel(double sigma, int size);
    
    // Generowanie jądra równomiernego
    static std::vector<std::vector<double>> generateUniformKernel(int size);
    
    // Obliczanie optymalnego rozmiaru jądra na podstawie sigma
    static int calculateKernelSize(double sigma);
    
    // Funkcja Gaussa
    static double gaussianFunction(int x, int y, double sigma);
    
    // Aplikowanie jądra konwolucji do obrazu
    static void applyConvolution(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& kernel);
    
    // Funkcja pomocnicza do ograniczenia wartości do zakresu 0-255
    static int clamp(int value, int min = 0, int max = 255);
};

#endif // BLUR_H
