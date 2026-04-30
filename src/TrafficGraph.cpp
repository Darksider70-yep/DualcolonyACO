#include "TrafficGraph.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <fstream>
#include <sstream>
#include <random>
#include <stdexcept>

// Define PI constant for C++17 compatibility
const double PI = 3.14159265358979323846;

TrafficGraph::TrafficGraph(int num_cities, float evaporation_rate)
    : num_cities_(num_cities),
      pheromone_evaporation_rate_(evaporation_rate),
      initial_pheromone_level_(0.1f) {
    // Initialize 2D vectors for the graph matrices
    base_distance_.resize(num_cities, std::vector<float>(num_cities, 0.0f));
    current_traffic_multiplier_.resize(num_cities, std::vector<float>(num_cities, 1.0f));
    pheromone_level_.resize(num_cities, std::vector<float>(num_cities, initial_pheromone_level_));
    edge_phase_shifts_.resize(num_cities, std::vector<float>(num_cities, 0.0f));
    city_coordinates_.resize(num_cities, std::pair<float, float>(0.0f, 0.0f));

    randomizePhaseShifts();
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

void TrafficGraph::resetPheromones() {
    for (int i = 0; i < num_cities_; ++i) {
        for (int j = 0; j < num_cities_; ++j) {
            pheromone_level_[i][j] = initial_pheromone_level_;
        }
    }
}

void TrafficGraph::randomizePhaseShifts() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> phase_dist(0.0f, static_cast<float>(2.0 * PI));

    for (int i = 0; i < num_cities_; ++i) {
        edge_phase_shifts_[i][i] = 0.0f;
        for (int j = i + 1; j < num_cities_; ++j) {
            const float phase = phase_dist(rng);
            edge_phase_shifts_[i][j] = phase;
            edge_phase_shifts_[j][i] = phase;
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
    return initial_pheromone_level_;
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

    auto trim = [](const std::string& input) {
        std::size_t start = 0;
        while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
            ++start;
        }

        std::size_t end = input.size();
        while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
            --end;
        }

        return input.substr(start, end - start);
    };

    std::string line;
    int parsed_dimension = -1;
    bool found_coord_section = false;

    while (std::getline(file, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty()) {
            continue;
        }

        if (trimmed == "NODE_COORD_SECTION") {
            found_coord_section = true;
            break;
        }

        if (trimmed.rfind("DIMENSION", 0) == 0) {
            std::size_t colon_pos = trimmed.find(':');
            std::string dim_part = (colon_pos != std::string::npos) ? trimmed.substr(colon_pos + 1) : trimmed.substr(9);
            dim_part = trim(dim_part);
            if (dim_part.empty()) {
                throw std::runtime_error("Invalid TSPLIB DIMENSION line in: " + filename);
            }

            parsed_dimension = std::stoi(dim_part);
        }
    }

    if (!found_coord_section) {
        throw std::runtime_error("TSPLIB NODE_COORD_SECTION not found in: " + filename);
    }
    if (parsed_dimension <= 0) {
        throw std::runtime_error("Invalid TSPLIB DIMENSION in: " + filename);
    }

    num_cities_ = parsed_dimension;
    base_distance_.assign(num_cities_, std::vector<float>(num_cities_, 0.0f));
    current_traffic_multiplier_.assign(num_cities_, std::vector<float>(num_cities_, 1.0f));
    pheromone_level_.assign(num_cities_, std::vector<float>(num_cities_, initial_pheromone_level_));
    edge_phase_shifts_.assign(num_cities_, std::vector<float>(num_cities_, 0.0f));
    city_coordinates_.assign(num_cities_, std::pair<float, float>(0.0f, 0.0f));

    std::vector<bool> seen_nodes(static_cast<std::size_t>(num_cities_), false);
    int loaded_nodes = 0;

    while (std::getline(file, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed == "EOF") {
            break;
        }

        std::istringstream iss(trimmed);
        int id = 0;
        double x = 0.0;
        double y = 0.0;
        if (!(iss >> id >> x >> y)) {
            continue;
        }

        if (id < 1 || id > num_cities_) {
            throw std::runtime_error("TSPLIB node id out of range in: " + filename);
        }

        const int index = id - 1;
        if (!seen_nodes[static_cast<std::size_t>(index)]) {
            seen_nodes[static_cast<std::size_t>(index)] = true;
            loaded_nodes++;
        }

        city_coordinates_[index] = {static_cast<float>(x), static_cast<float>(y)};
    }

    if (loaded_nodes < num_cities_) {
        throw std::runtime_error(
            "TSPLIB coordinate count mismatch in: " + filename +
            " (expected " + std::to_string(num_cities_) +
            ", loaded " + std::to_string(loaded_nodes) + ")"
        );
    }

    for (int i = 0; i < num_cities_; ++i) {
        for (int j = i; j < num_cities_; ++j) {
            if (i == j) {
                base_distance_[i][j] = 0.0f;
                continue;
            }

            double dx = static_cast<double>(city_coordinates_[i].first) - static_cast<double>(city_coordinates_[j].first);
            double dy = static_cast<double>(city_coordinates_[i].second) - static_cast<double>(city_coordinates_[j].second);
            float distance = static_cast<float>(std::round(std::sqrt((dx * dx) + (dy * dy))));

            base_distance_[i][j] = distance;
            base_distance_[j][i] = distance;
        }
    }

    randomizePhaseShifts();
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
