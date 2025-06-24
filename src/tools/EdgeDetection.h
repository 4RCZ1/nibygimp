#ifndef EDGEDETECTION_H
#define EDGEDETECTION_H

#include <memory>
#include <vector>
#include <cmath>
#include "../image/Image.h"

class EdgeDetection {
public:
    // Operator Laplace'a - wykrywanie krawędzi przez drugą pochodną
    static void laplacianFilter(std::unique_ptr<Image>& image, int kernelSize = 3);
    
    // Operator Laplace'a z konwersją na skalę szarości dla lepszej wizualizacji
    static void laplacianFilterGrayscale(std::unique_ptr<Image>& image, int kernelSize = 3);
    
    // Negatywny operator Laplace'a (wykrywa ciemne linie na jasnym tle)
    static void laplacianFilterNegative(std::unique_ptr<Image>& image, int kernelSize = 3);
      // Laplacian of Gaussian (LoG) - metoda z wygładzaniem Gaussowskim
    static void laplacianOfGaussian(std::unique_ptr<Image>& image, double sigma = 1.0, int windowSize = 3, double threshold = 0.1);
    
    // Uproszczona wersja LoG bez progowania (dla testów)
    static void laplacianOfGaussianSimple(std::unique_ptr<Image>& image, double sigma = 1.0);
    
    // Operator Sobel - wykrywanie krawędzi przez gradient
    static void sobelFilter(std::unique_ptr<Image>& image);
    
    // Filtr wykrywania krawędzi z macierzą niestandardową
    static void customEdgeFilter(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& matrix);

private:
    // Generowanie jądra Laplaciana
    static std::vector<std::vector<double>> generateLaplacianKernel(int size);
    
    // Generowanie negatywnego jądra Laplaciana
    static std::vector<std::vector<double>> generateLaplacianNegativeKernel(int size);
    
    // Generowanie jądra Laplacian of Gaussian
    static std::vector<std::vector<double>> generateLoGKernel(double sigma, int size);
      // Aplikowanie algorytmu LoG z progowaniem zgodnie z instrukcją
    static void applyLoGWithThresholding(std::unique_ptr<Image>& image, double sigma, int windowSize, double threshold);
    
    // Generowanie jąder Sobela (Gx i Gy)
    static std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> generateSobelKernels();
    
    // Aplikowanie konwolucji do obrazu (współdzielone z Blur)
    static void applyConvolution(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& kernel);
    
    // Aplikowanie konwolucji Laplaciana z konwersją na skalę szarości
    static void applyLaplacianGrayscaleConvolution(std::unique_ptr<Image>& image, const std::vector<std::vector<double>>& kernel);
    
    // Aplikowanie dwóch jąder jednocześnie (dla Sobela)
    static void applySobelConvolution(std::unique_ptr<Image>& image, 
                                     const std::vector<std::vector<double>>& kernelX,
                                     const std::vector<std::vector<double>>& kernelY);
    
    // Funkcja pomocnicza do ograniczenia wartości do zakresu 0-255
    static int clamp(int value, int min = 0, int max = 255);
};

#endif // EDGEDETECTION_H
