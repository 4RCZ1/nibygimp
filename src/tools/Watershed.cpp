//
// Created by 4RCZ1 on 25.06.2025.
//

#include "watershed.h"
#include <climits>
#include <QDebug>

const int VincentSoilleWatershed::INIT;
const int VincentSoilleWatershed::MASK;
const int VincentSoilleWatershed::WATERSHED_LINE;

VincentSoilleWatershed::VincentSoilleWatershed(int connectivity) 
    : connectivity(connectivity), currentLabel(0) {
    if (connectivity != 4 && connectivity != 8) {
        this->connectivity = 8; // Default to 8-connectivity
    }
}

void VincentSoilleWatershed::watershed(std::unique_ptr<Image> &image) {
    m_width = image->width();
    m_height = image->height();
    
    // Step 1: Sort pixels by intensity
    std::vector<Pixel> sortedPixels = sortPixelsByIntensity(image);
    
    // Step 2: Initialize arrays
    initialize(m_width, m_height);
    
    // Step 3: First pass - identify minima
    firstPass(image, sortedPixels);
    
    // Step 4: Immersion simulation
    immersionSimulation(image, sortedPixels);
    
    // Step 5: Create result image
    createResultImage(image);
}

std::vector<Pixel> VincentSoilleWatershed::sortPixelsByIntensity(std::unique_ptr<Image> &image) {
    std::vector<Pixel> pixels;
    pixels.reserve(m_width * m_height);
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int intensity = image->getPixelR(x, y);
            pixels.emplace_back(x, y, intensity);
        }
    }
    
    // Sort by intensity (ascending order)
    std::sort(pixels.begin(), pixels.end(), 
              [](const Pixel& a, const Pixel& b) {
                  return a.intensity < b.intensity;
              });
    
    return pixels;
}

void VincentSoilleWatershed::initialize(int width, int height) {
    currentLabel = 0;
    
    // Initialize labels and distances arrays
    labels.assign(height, std::vector<int>(width, INIT));
    distances.assign(height, std::vector<int>(width, 0));
}

void VincentSoilleWatershed::firstPass(std::unique_ptr<Image> &image, const std::vector<Pixel>& sortedPixels) {
    for (const auto& pixel : sortedPixels) {
        int x = pixel.x;
        int y = pixel.y;
        
        labels[y][x] = MASK;
        
        // Check if pixel is a local minimum
        if (isLocalMinimum(image, x, y)) {
            currentLabel++;
            labels[y][x] = currentLabel;
            distances[y][x] = 0;
        }
    }
}

void VincentSoilleWatershed::immersionSimulation(std::unique_ptr<Image> &image, const std::vector<Pixel>& sortedPixels) {
    int currentDistance = 1;
    size_t pixelIndex = 0;
    
    while (pixelIndex < sortedPixels.size()) {
        // Get current intensity level
        int currentIntensity = sortedPixels[pixelIndex].intensity;
        std::queue<std::pair<int, int>> fifoQueue;
        
        // Collect all pixels at this intensity level and mark neighbors
        while (pixelIndex < sortedPixels.size() && 
               sortedPixels[pixelIndex].intensity == currentIntensity) {
            
            int x = sortedPixels[pixelIndex].x;
            int y = sortedPixels[pixelIndex].y;
            
            labels[y][x] = MASK;
            
            // Check neighbors for existing labels
            auto neighbors = getNeighbors(x, y);
            for (const auto& neighbor : neighbors) {
                int nx = neighbor.first;
                int ny = neighbor.second;
                
                if (labels[ny][nx] > 0 || labels[ny][nx] == WATERSHED_LINE) {
                    distances[y][x] = 1;
                    fifoQueue.push({x, y});
                    break;
                }
            }
            
            pixelIndex++;
        }
        
        // Extend labeled regions
        currentDistance = 1;
        fifoQueue.push({-1, -1}); // Level separator
        
        while (!fifoQueue.empty()) {
            auto current = fifoQueue.front();
            fifoQueue.pop();
            
            int x = current.first;
            int y = current.second;
            
            // Check for level separator
            if (x == -1 && y == -1) {
                if (!fifoQueue.empty()) {
                    fifoQueue.push({-1, -1});
                    currentDistance++;
                    continue;
                } else {
                    break;
                }
            }
            
            // Process current pixel
            std::set<int> neighborLabels;
            auto neighbors = getNeighbors(x, y);
            
            for (const auto& neighbor : neighbors) {
                int nx = neighbor.first;
                int ny = neighbor.second;
                
                if (distances[ny][nx] < currentDistance && 
                    (labels[ny][nx] > 0 || labels[ny][nx] == WATERSHED_LINE)) {
                    
                    if (labels[ny][nx] > 0) {
                        neighborLabels.insert(labels[ny][nx]);
                    }
                } else if (labels[ny][nx] == MASK && distances[ny][nx] == 0) {
                    distances[ny][nx] = currentDistance + 1;
                    fifoQueue.push({nx, ny});
                }
            }
            
            // Assign label based on neighbors
            if (neighborLabels.size() == 1) {
                labels[y][x] = *neighborLabels.begin();
            } else if (neighborLabels.size() > 1) {
                labels[y][x] = WATERSHED_LINE;
            }
        }
        
        // Handle remaining MASK pixels at this level (create new regions)
        for (size_t i = pixelIndex - 1; i < sortedPixels.size() && 
             sortedPixels[i].intensity == currentIntensity; --i) {
            
            int x = sortedPixels[i].x;
            int y = sortedPixels[i].y;
            
            if (labels[y][x] == MASK) {
                currentLabel++;
                std::queue<std::pair<int, int>> floodQueue;
                floodFill(x, y, currentLabel, floodQueue);
            }
            
            if (i == 0) break; // Prevent underflow
        }
    }
}

std::vector<std::pair<int, int>> VincentSoilleWatershed::getNeighbors(int x, int y) const {
    std::vector<std::pair<int, int>> neighbors;
    
    if (connectivity == 4) {
        // 4-connected neighbors
        std::vector<std::pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        
        for (const auto& dir : directions) {
            int nx = x + dir.first;
            int ny = y + dir.second;
            
            if (nx >= 0 && nx < m_width && ny >= 0 && ny < m_height) {
                neighbors.push_back({nx, ny});
            }
        }
    } else {
        // 8-connected neighbors
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                
                int nx = x + dx;
                int ny = y + dy;
                
                if (nx >= 0 && nx < m_width && ny >= 0 && ny < m_height) {
                    neighbors.push_back({nx, ny});
                }
            }
        }
    }
    
    return neighbors;
}

bool VincentSoilleWatershed::isLocalMinimum(std::unique_ptr<Image> &image, int x, int y) const {
    int currentIntensity = image->getPixelR(x, y);
    auto neighbors = getNeighbors(x, y);
    
    for (const auto& neighbor : neighbors) {
        int nx = neighbor.first;
        int ny = neighbor.second;
        int neighborIntensity = image->getPixelR(nx, ny);
        
        if (neighborIntensity < currentIntensity) {
            return false;
        }
    }
    
    return true;
}

void VincentSoilleWatershed::floodFill(int x, int y, int label, 
                                       std::queue<std::pair<int, int>>& fifoQueue) {
    fifoQueue.push({x, y});
    labels[y][x] = label;
    
    while (!fifoQueue.empty()) {
        auto current = fifoQueue.front();
        fifoQueue.pop();
        
        auto neighbors = getNeighbors(current.first, current.second);
        for (const auto& neighbor : neighbors) {
            int nx = neighbor.first;
            int ny = neighbor.second;
            
            if (labels[ny][nx] == MASK) {
                labels[ny][nx] = label;
                fifoQueue.push({nx, ny});
            }
        }
    }
}

void VincentSoilleWatershed::createResultImage(std::unique_ptr<Image> &image) const {
    // Find max label for normalization
    int maxLabel = 0;
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (labels[y][x] > maxLabel) {
                maxLabel = labels[y][x];
            }
        }
    }
    
    // Create visualization
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (labels[y][x] == WATERSHED_LINE) {
                // Watershed lines in red
                image->setPixel(x, y, 255, 0, 0);
            } else if (labels[y][x] > 0) {
                // Different regions with different intensities
                int intensity = (labels[y][x] * 255) / maxLabel;
                qDebug() << "Pixel at (" << x << "," << y << "): " << ", label=" << labels[y][x] << ", maxLabel=" << maxLabel;
                image->setPixel(x, y, intensity, intensity, intensity);
            } else {
                // Background in black
                image->setPixel(x, y, 0, 0, 0);
            }
        }
    }
}