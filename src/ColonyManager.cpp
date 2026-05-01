#include "ColonyManager.h"
#include <vector>
#include <memory>
#include <limits>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

void ColonyManager::rebuildAntPopulation(int num_workers, int num_scouts) {
    num_workers = std::max(0, num_workers);
    num_scouts = std::max(0, num_scouts);

    worker_ants_.clear();
    scout_ants_.clear();

    std::uniform_int_distribution<std::uint32_t> seed_dist(
        0u, std::numeric_limits<std::uint32_t>::max());

    for (int i = 0; i < num_workers; ++i) {
        worker_ants_.push_back(
            std::make_unique<WorkerAnt>(0, i, alpha_, beta_, seed_dist(rng_)));
    }

    const float gamma = 2.0f;
    for (int i = 0; i < num_scouts; ++i) {
        scout_ants_.push_back(
            std::make_unique<ScoutAnt>(0,
                                       num_workers + i,
                                       alpha_ * 0.3f,
                                       beta_,
                                       gamma,
                                       scout_ratio_,
                                       seed_dist(rng_)));
    }
}

ColonyManager::ColonyManager(TrafficGraph& graph,
                             int num_workers,
                             int num_scouts,
                             float alpha,
                             float beta,
                             float evaporation_rate,
                             float scout_ratio,
                             float pheromone_deposit_factor)
    : traffic_graph_(graph),
      rng_(std::random_device{}()),
      alpha_(alpha),
      beta_(beta),
      evaporation_rate_(std::clamp(evaporation_rate, 0.0f, 1.0f)),
      scout_ratio_(scout_ratio),
      pheromone_deposit_factor_(pheromone_deposit_factor),
      Q_constant_(100.0),  // Pheromone deposition constant
      total_iterations_(0),
      current_iteration_(0),
      best_tour_time_(std::numeric_limits<double>::max()),
      last_iteration_best_time_(std::numeric_limits<double>::max()) {
    const int total_ants = std::max(0, num_workers) + std::max(0, num_scouts);
    if (total_ants > 0) {
        scout_ratio_ = static_cast<float>(std::max(0, num_scouts)) / static_cast<float>(total_ants);
    }

    rebuildAntPopulation(num_workers, num_scouts);
}

ColonyManager::~ColonyManager() {
    // Unique pointers handle cleanup automatically
}

void ColonyManager::initializeSwarm() {
    // Reset all ants to the starting city (depot at index 0)
    for (auto& ant : worker_ants_) {
        ant->reset(0);
    }
    for (auto& ant : scout_ants_) {
        ant->reset(0);
    }
}

void ColonyManager::runIteration() {
    int num_cities = traffic_graph_.getNumCities();

    // Phase 1: Reset all ants to the starting depot
    for (auto& ant : worker_ants_) {
        ant->reset(0);
    }
    for (auto& ant : scout_ants_) {
        ant->reset(0);
    }

    // Phase 2: Tour Construction - All ants build their tours
    for (int step = 0; step < num_cities - 1; ++step) {
        // Worker ants move
        for (auto& ant : worker_ants_) {
            double current_time = ant->getCurrentTime();
            int next_city = ant->chooseNextCity(traffic_graph_, current_time);
            double travel_time = traffic_graph_.getTravelTime(ant->getCurrentCity(), next_city, current_time);
            ant->moveToCity(next_city, travel_time);
        }
        
        // Scout ants move
        for (auto& ant : scout_ants_) {
            double current_time = ant->getCurrentTime();
            int next_city = ant->chooseNextCity(traffic_graph_, current_time);
            double travel_time = traffic_graph_.getTravelTime(ant->getCurrentCity(), next_city, current_time);
            ant->moveToCity(next_city, travel_time);
        }
    }

    // Return to depot to complete the TSP tour
    for (auto& ant : worker_ants_) {
        double return_time = traffic_graph_.getTravelTime(ant->getCurrentCity(), 0, ant->getCurrentTime());
        ant->moveToCity(0, return_time);
    }
    for (auto& ant : scout_ants_) {
        double return_time = traffic_graph_.getTravelTime(ant->getCurrentCity(), 0, ant->getCurrentTime());
        ant->moveToCity(0, return_time);
    }

    // Phase 3: Pheromone evaporation (amnesia mechanism)
    const double evaporation_multiplier = 1.0 - static_cast<double>(evaporation_rate_);
    for (int i = 0; i < num_cities; ++i) {
        for (int j = 0; j < num_cities; ++j) {
            const double current_pheromone =
                static_cast<double>(traffic_graph_.getPheromoneLevel(i, j));
            traffic_graph_.setPheromoneLevel(
                i,
                j,
                static_cast<float>(current_pheromone * evaporation_multiplier));
        }
    }

    // Phase 4: Pheromone Update
    double iteration_best_time = std::numeric_limits<double>::max();
    Ant* best_ant = nullptr;

    // Find the best ant in this iteration and deposit pheromones
    // Worker ants
    for (auto& ant : worker_ants_) {
        double tour_time = ant->getTotalPathTime();
        
        if (tour_time < iteration_best_time) {
            iteration_best_time = tour_time;
            best_ant = ant.get();
        }

        // Deposit pheromones proportional to tour quality
        double pheromone_to_drop = Q_constant_ / tour_time;
        traffic_graph_.depositPheromones(ant->getTour(), pheromone_to_drop);
    }

    // Scout ants
    for (auto& ant : scout_ants_) {
        double tour_time = ant->getTotalPathTime();
        
        if (tour_time < iteration_best_time) {
            iteration_best_time = tour_time;
            best_ant = ant.get();
        }

        // Scout ants contribute less to global pheromones (exploration-oriented)
        double pheromone_to_drop = (Q_constant_ / tour_time) * 0.5;
        traffic_graph_.depositPheromones(ant->getTour(), pheromone_to_drop);
    }

    // Check against Global Best
    last_iteration_best_time_ = iteration_best_time;

    if (iteration_best_time < best_tour_time_) {
        best_tour_time_ = iteration_best_time;
        if (best_ant != nullptr) {
            best_tour_path_ = best_ant->getTour();
        }
    }

    current_iteration_++;
}

void ColonyManager::updateGlobalPheromones() {
    // Already handled in runIteration()
    // This method is kept for API completeness
}

void ColonyManager::runAlgorithm(int num_iterations) {
    initializeSwarm();
    total_iterations_ = num_iterations;

    for (int iter = 0; iter < num_iterations; ++iter) {
        runIteration();
    }
}

const std::vector<int>& ColonyManager::getBestTourPath() const {
    return best_tour_path_;
}

double ColonyManager::getBestTourTime() const {
    return best_tour_time_;
}

double ColonyManager::getLastIterationBestTime() const {
    return last_iteration_best_time_;
}

void ColonyManager::reset() {
    best_tour_time_ = std::numeric_limits<double>::max();
    last_iteration_best_time_ = std::numeric_limits<double>::max();
    best_tour_path_.clear();
    current_iteration_ = 0;
    initializeSwarm();
}

int ColonyManager::getNumWorkerAnts() const {
    return worker_ants_.size();
}

int ColonyManager::getNumScoutAnts() const {
    return scout_ants_.size();
}

int ColonyManager::getCurrentIteration() const {
    return current_iteration_;
}

void ColonyManager::setParameters(float alpha, float beta, float evaporation_rate, float scout_ratio) {
    alpha_ = alpha;
    beta_ = beta;
    evaporation_rate_ = std::clamp(evaporation_rate, 0.0f, 1.0f);
    scout_ratio_ = scout_ratio;

    const int num_workers = static_cast<int>(worker_ants_.size());
    const int num_scouts = static_cast<int>(scout_ants_.size());
    rebuildAntPopulation(num_workers, num_scouts);
    reset();
}

void ColonyManager::configurePopulation(int total_ants, float scout_ratio) {
    if (total_ants <= 0) {
        throw std::invalid_argument("total_ants must be > 0");
    }

    const float clamped_ratio = std::clamp(scout_ratio, 0.0f, 1.0f);
    const int num_scouts = static_cast<int>(std::lround(static_cast<double>(total_ants) * clamped_ratio));
    const int num_workers = total_ants - num_scouts;

    scout_ratio_ = clamped_ratio;
    rebuildAntPopulation(num_workers, num_scouts);
    reset();
}
