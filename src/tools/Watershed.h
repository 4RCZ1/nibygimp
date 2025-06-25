//
// Created by 4RCZ1 on 25.06.2025.
//

#ifndef WATERSHED_H
#define WATERSHED_H

#include "../image/Image.h"
#include <vector>
#include <queue>
#include <set>
#include <algorithm>

struct Pixel {
    int x, y;
    int intensity;
    
    Pixel(int x, int y, int intensity) : x(x), y(y), intensity(intensity) {}
    
    // For priority queue (min-heap based on intensity)
    bool operator>(const Pixel& other) const {
        return intensity > other.intensity;
    }
};

class VincentSoilleWatershed {
private:
    // Constants
    static const int WATERSHED_LINE = 0;
    static const int MASK = 2;
    static const int INIT = 0;
    
    // Image dimensions
    int m_width, m_height;
    
    // Working arrays
    std::vector<std::vector<int>> labels;
    std::vector<std::vector<int>> distances;
    
    // Current label counter
    int currentLabel;
    
    // Connectivity (4 or 8)
    int connectivity;
    
public:
    VincentSoilleWatershed(int connectivity = 8);
    ~VincentSoilleWatershed() = default;
    
    // Main watershed function
    void watershed(std::unique_ptr<Image> &image);
    
private:
    // Helper functions
    std::vector<Pixel> sortPixelsByIntensity(std::unique_ptr<Image> &image);
    void initialize(int width, int height);
    void firstPass(std::unique_ptr<Image> &image, const std::vector<Pixel>& sortedPixels);
    void immersionSimulation(std::unique_ptr<Image> &image, const std::vector<Pixel>& sortedPixels);
    std::vector<std::pair<int, int>> getNeighbors(int x, int y) const;
    bool isLocalMinimum(std::unique_ptr<Image> &image, int x, int y) const;
    void floodFill(int x, int y, int label, std::queue<std::pair<int, int>>& fifoQueue);
    void createResultImage(std::unique_ptr<Image> &image) const;
};

#endif // WATERSHED_H