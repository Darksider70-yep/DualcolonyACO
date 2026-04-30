#include "TrafficGraph.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>
#include <stdexcept>

// Define PI constant for C++17 compatibility
const double PI = 3.14159265358979323846;

TrafficGraph::TrafficGraph(int num_cities, float evaporation_rate)
    : num_cities_(num_cities), pheromone_evaporation_rate_(evaporation_rate) {
    // Initialize 2D vectors for the graph matrices
    base_distance_.resize(num_cities, std::vector<float>(num_cities, 0.0f));
    current_traffic_multiplier_.resize(num_cities, std::vector<float>(num_cities, 1.0f));
    pheromone_level_.resize(num_cities, std::vector<float>(num_cities, 0.1f));
    edge_phase_shifts_.resize(num_cities, std::vector<float>(num_cities, 0.0f));
    city_coordinates_.resize(num_cities, std::pair<float, float>(0.0f, 0.0f));
    
    // Initialize pseudo-random phase shifts for each edge
    std::mt19937 rng(42);  // Seeded for reproducibility
    std::uniform_real_distribution<float> phase_dist(0.0f, 2.0f * PI);
    for (int i = 0; i < num_cities; ++i) {
        for (int j = 0; j < num_cities; ++j) {
            edge_phase_shifts_[i][j] = phase_dist(rng);
        }
    }
}

TrafficGraph::~TrafficGraph() {
    // Cleanup if needed (vectors handle their own memory)
}

void TrafficGraph::initializeDistances(const std::vector<std::vector<float>>& distances) {
    base_distance_ = distances;
}

void TrafficGraph::triggerTrafficJam(int cityA, int cityB, float severity) {
    if (cityA >= 0 && cityA < num_cities_ && cityB >= 0 && cityB < num_cities_) {
        current_traffic_multiplier_[cityA][cityB] = severity;
        current_traffic_multiplier_[cityB][cityA] = severity;  // Symmetric
    }
}

void TrafficGraph::reduceTraffic() {
    // Reduce traffic multipliers by 5% each iteration (recovery towards 1.0)
    for (int i = 0; i < num_cities_; ++i) {
        for (int j = 0; j < num_cities_; ++j) {
            if (current_traffic_multiplier_[i][j] > 1.0f) {
                current_traffic_multiplier_[i][j] = 1.0f + (current_traffic_multiplier_[i][j] - 1.0f) * 0.95f;
            }
        }
    }
}

void TrafficGraph::evaporatePheromones() {
    for (int i = 0; i < num_cities_; ++i) {
        for (int j = 0; j < num_cities_; ++j) {
            pheromone_level_[i][j] *= (1.0f - pheromone_evaporation_rate_);
        }
    }
}

float TrafficGraph::getEffectiveDistance(int cityA, int cityB) const {
    if (cityA >= 0 && cityA < num_cities_ && cityB >= 0 && cityB < num_cities_) {
        return base_distance_[cityA][cityB] * current_traffic_multiplier_[cityA][cityB];
    }
    return 0.0f;
}

float TrafficGraph::getPheromoneLevel(int cityA, int cityB) const {
    if (cityA >= 0 && cityA < num_cities_ && cityB >= 0 && cityB < num_cities_) {
        return pheromone_level_[cityA][cityB];
    }
    return 0.1f;
}

void TrafficGraph::setPheromoneLevel(int cityA, int cityB, float level) {
    if (cityA >= 0 && cityA < num_cities_ && cityB >= 0 && cityB < num_cities_) {
        pheromone_level_[cityA][cityB] = level;
    }
}

void TrafficGraph::addPheromone(int cityA, int cityB, float amount) {
    if (cityA >= 0 && cityA < num_cities_ && cityB >= 0 && cityB < num_cities_) {
        pheromone_level_[cityA][cityB] += amount;
    }
}

float TrafficGraph::getBaseDistance(int cityA, int cityB) const {
    if (cityA >= 0 && cityA < num_cities_ && cityB >= 0 && cityB < num_cities_) {
        return base_distance_[cityA][cityB];
    }
    return 0.0f;
}

int TrafficGraph::getNumCities() const {
    return num_cities_;
}

float TrafficGraph::getTrafficMultiplier(int cityA, int cityB) const {
    if (cityA >= 0 && cityA < num_cities_ && cityB >= 0 && cityB < num_cities_) {
        return current_traffic_multiplier_[cityA][cityB];
    }
    return 1.0f;
}

void TrafficGraph::loadTSPLIB(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open TSPLIB file: " + filename);
    }

    std::string line;
    int node_count = 0;
    bool reading_coordinates = false;

    // Parse file header
    while (std::getline(file, line)) {
        if (line.find("DIMENSION:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            int dim;
            iss >> key >> dim;
            node_count = dim;
            num_cities_ = dim;
            
            // Reinitialize matrices with new size
            base_distance_.assign(num_cities_, std::vector<float>(num_cities_, 0.0f));
            city_coordinates_.assign(num_cities_, std::pair<float, float>(0.0f, 0.0f));
        }
        else if (line.find("NODE_COORD_SECTION") != std::string::npos) {
            reading_coordinates = true;
            break;
        }
    }

    // Read node coordinates
    int node_id = 0;
    while (std::getline(file, line) && reading_coordinates) {
        if (line.find("EOF") != std::string::npos) {
            break;
        }
        
        std::istringstream iss(line);
        int id;
        float x, y;
        
        if (iss >> id >> x >> y) {
            if (id > 0 && id <= node_count) {
                city_coordinates_[id - 1] = std::make_pair(x, y);
            }
        }
    }
    file.close();

    // Calculate Euclidean distances
    for (int i = 0; i < num_cities_; ++i) {
        for (int j = i; j < num_cities_; ++j) {
            if (i == j) {
                base_distance_[i][j] = 0.0f;
            } else {
                float dx = city_coordinates_[i].first - city_coordinates_[j].first;
                float dy = city_coordinates_[i].second - city_coordinates_[j].second;
                float distance = std::sqrt(dx * dx + dy * dy);
                base_distance_[i][j] = distance;
                base_distance_[j][i] = distance;  // Symmetric
            }
        }
    }
}

double TrafficGraph::calculateRushHourMultiplier(int cityA, int cityB, double current_time) const {
    if (cityA < 0 || cityA >= num_cities_ || cityB < 0 || cityB >= num_cities_) {
        return 1.0;
    }

    // Rush hour parameters
    double amplitude = 1.5;           // Traffic can be 1.5x worse at peak
    double frequency = 12.0;          // 12-hour cycle
    double phase_shift = edge_phase_shifts_[cityA][cityB];
    
    // Sine wave formula: 1.0 + amplitude * sin^2((PI * time / frequency) - phase)
    double angle = (PI * current_time / frequency) - phase_shift;
    double sine_value = std::sin(angle);
    double traffic_multiplier = 1.0 + (amplitude * (sine_value * sine_value));
    
    return traffic_multiplier;
}

double TrafficGraph::getTravelTime(int cityA, int cityB, double current_time) const {
    if (cityA < 0 || cityA >= num_cities_ || cityB < 0 || cityB >= num_cities_) {
        return 0.0;
    }

    double base_time = static_cast<double>(base_distance_[cityA][cityB]);
    double multiplier = calculateRushHourMultiplier(cityA, cityB, current_time);
    
    return base_time * multiplier;
}

void TrafficGraph::depositPheromones(const std::vector<int>& tour, double pheromone_amount) {
    for (std::size_t i = 0; i < tour.size(); ++i) {
        int from = tour[i];
        int to = tour[(i + 1) % tour.size()];  // Wrap around to start for return trip
        
        if (from >= 0 && from < num_cities_ && to >= 0 && to < num_cities_) {
            addPheromone(from, to, static_cast<float>(pheromone_amount));
            addPheromone(to, from, static_cast<float>(pheromone_amount));  // Symmetric
        }
    }
}
