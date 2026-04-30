#include "Ant.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <numeric>

// ========================
// Ant Base Class Implementation
// ========================

Ant::Ant(int start_city, int ant_id)
    : current_city_(start_city), total_path_time_(0.0), current_time_(0.0), ant_id_(ant_id) {
    visited_cities_.push_back(start_city);
}

void Ant::moveToCity(int next_city, double cost) {
    visited_cities_.push_back(next_city);
    current_city_ = next_city;
    total_path_time_ += cost;
    current_time_ += cost;
}

void Ant::reset(int start_city) {
    visited_cities_.clear();
    visited_cities_.push_back(start_city);
    current_city_ = start_city;
    total_path_time_ = 0.0;
    current_time_ = 0.0;
}

int Ant::getCurrentCity() const {
    return current_city_;
}

double Ant::getCurrentTime() const {
    return current_time_;
}

const std::vector<int>& Ant::getVisitedCities() const {
    return visited_cities_;
}

double Ant::getTotalPathTime() const {
    return total_path_time_;
}

int Ant::getAntId() const {
    return ant_id_;
}

bool Ant::hasVisited(int city) const {
    return std::find(visited_cities_.begin(), visited_cities_.end(), city) != visited_cities_.end();
}

const std::vector<int>& Ant::getTour() const {
    return visited_cities_;
}

// ========================
// WorkerAnt Implementation
// ========================

WorkerAnt::WorkerAnt(int start_city, int ant_id, float alpha, float beta)
    : Ant(start_city, ant_id), alpha_(alpha), beta_(beta) {
}

int WorkerAnt::chooseNextCity(const TrafficGraph& graph, double current_time) {
    int num_cities = graph.getNumCities();
    std::vector<double> probabilities(num_cities, 0.0);
    double total_weight = 0.0;

    // Calculate weights for each unvisited city: (Tau^Alpha) * (Eta^Beta)
    // Eta = 1.0 / travel_time (inverse of travel time)
    for (int city = 0; city < num_cities; ++city) {
        if (!hasVisited(city)) {
            // Get pheromone level on the edge
            double tau = graph.getPheromoneLevel(current_city_, city);
            tau = std::max(tau, 0.00001);  // Avoid zero pheromone
            
            // Get travel time for this edge at current time
            double travel_time = graph.getTravelTime(current_city_, city, current_time);
            if (travel_time < 0.00001) travel_time = 0.00001;  // Avoid division by zero
            double eta = 1.0 / travel_time;
            
            // ACO formula: (tau^alpha) * (eta^beta)
            double weight = std::pow(tau, alpha_) * std::pow(eta, beta_);
            probabilities[city] = weight;
            total_weight += weight;
        }
    }

    // Roulette wheel selection
    if (total_weight == 0.0) {
        // If no valid cities, return any unvisited city
        for (int city = 0; city < num_cities; ++city) {
            if (!hasVisited(city)) {
                return city;
            }
        }
        return current_city_;  // Fallback
    }

    // Generate random selection
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, total_weight);
    double spin = dist(rng);

    double cumulative = 0.0;
    for (int city = 0; city < num_cities; ++city) {
        if (!hasVisited(city)) {
            cumulative += probabilities[city];
            if (spin <= cumulative) {
                return city;
            }
        }
    }

    // Fallback: return the last unvisited city
    for (int city = num_cities - 1; city >= 0; --city) {
        if (!hasVisited(city)) {
            return city;
        }
    }
    
    return current_city_;
}

// ========================
// ScoutAnt Implementation
// ========================

ScoutAnt::ScoutAnt(int start_city, int ant_id, float alpha, float beta, float gamma, float exploration_factor)
    : Ant(start_city, ant_id), alpha_(alpha), beta_(beta), gamma_(gamma), exploration_factor_(exploration_factor) {
}

int ScoutAnt::chooseNextCity(const TrafficGraph& graph, double current_time) {
    int num_cities = graph.getNumCities();
    std::vector<double> probabilities(num_cities, 0.0);
    double total_weight = 0.0;

    // Scout ant formula (pheromone avoidance): ((1.0 / (Tau + 0.0001))^Gamma) * (Eta^Beta)
    // This encourages exploration by avoiding high-pheromone edges
    for (int city = 0; city < num_cities; ++city) {
        if (!hasVisited(city)) {
            // Get pheromone level and apply inverse with gamma exponent
            double tau = graph.getPheromoneLevel(current_city_, city);
            double pheromone_factor = std::pow(1.0 / (tau + 0.0001), gamma_);
            
            // Get travel time for this edge at current time
            double travel_time = graph.getTravelTime(current_city_, city, current_time);
            if (travel_time < 0.00001) travel_time = 0.00001;
            double eta = 1.0 / travel_time;
            
            // Scout formula: (1/(tau+epsilon))^gamma * (eta^beta)
            double weight = pheromone_factor * std::pow(eta, beta_);
            
            // Add exploration randomness
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<double> explore_dist(0.0, exploration_factor_);
            weight *= (1.0 + explore_dist(rng));
            
            probabilities[city] = weight;
            total_weight += weight;
        }
    }

    // Roulette wheel selection
    if (total_weight == 0.0) {
        // If no valid cities, return any unvisited city
        for (int city = 0; city < num_cities; ++city) {
            if (!hasVisited(city)) {
                return city;
            }
        }
        return current_city_;
    }

    // Generate random selection
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, total_weight);
    double spin = dist(rng);

    double cumulative = 0.0;
    for (int city = 0; city < num_cities; ++city) {
        if (!hasVisited(city)) {
            cumulative += probabilities[city];
            if (spin <= cumulative) {
                return city;
            }
        }
    }

    // Fallback: return the last unvisited city
    for (int city = num_cities - 1; city >= 0; --city) {
        if (!hasVisited(city)) {
            return city;
        }
    }
    
    return current_city_;
}
