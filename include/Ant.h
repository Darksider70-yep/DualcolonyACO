#ifndef ANT_H
#define ANT_H

#include <vector>
#include <set>
#include <cstdint>
#include <random>
#include "TrafficGraph.h"

/**
 * @class Ant
 * @brief Abstract base class representing an ant in the colony.
 * 
 * Each ant maintains its current state (position, visited cities, path cost).
 * Derived classes implement specific decision-making strategies via overriding
 * the chooseNextCity() method.
 */
class Ant {
protected:
    int current_city_;
    std::vector<int> visited_cities_;
    double total_path_time_;  // Changed to double for precision
    double current_time_;     // Current simulation time
    int ant_id_;
    std::mt19937 rng_;

public:
    /**
     * @brief Constructor for Ant.
     * @param start_city Initial city for the ant.
     * @param ant_id Unique identifier for this ant.
     * @param seed RNG seed for stochastic route construction.
     */
    Ant(int start_city, int ant_id, std::uint32_t seed);

    /**
     * @brief Virtual destructor for Ant.
     */
    virtual ~Ant() = default;

    /**
     * @brief Pure virtual method for choosing the next city.
     * 
     * Derived classes must implement their own decision logic based on
     * pheromone levels, distances, and traffic conditions.
     * 
     * @param graph Reference to the TrafficGraph.
     * @param current_time Current simulation time for dynamic traffic evaluation.
     * @return Index of the next city to visit.
     */
    virtual int chooseNextCity(const TrafficGraph& graph, double current_time) = 0;

    /**
     * @brief Move the ant to a new city and record the transition.
     * @param next_city The city to move to.
     * @param cost The cost (time) of the move.
     */
    void moveToCity(int next_city, double cost);

    /**
     * @brief Reset the ant's state for a new iteration.
     * @param start_city The starting city for the new tour.
     */
    void reset(int start_city);

    /**
     * @brief Get the current city of the ant.
     * @return Current city index.
     */
    int getCurrentCity() const;

    /**
     * @brief Get the current simulation time.
     * @return Current time.
     */
    double getCurrentTime() const;

    /**
     * @brief Get the list of visited cities in order.
     * @return Vector of visited city indices.
     */
    const std::vector<int>& getVisitedCities() const;

    /**
     * @brief Get the total path time for the current tour.
     * @return Total path time.
     */
    double getTotalPathTime() const;

    /**
     * @brief Get the ant's unique identifier.
     * @return Ant ID.
     */
    int getAntId() const;

    /**
     * @brief Check if a city has been visited.
     * @param city City index to check.
     * @return True if visited, false otherwise.
     */
    bool hasVisited(int city) const;

    /**
     * @brief Get the complete tour path.
     * @return Vector of visited city indices in order.
     */
    const std::vector<int>& getTour() const;
};

/**
 * @class WorkerAnt
 * @brief Worker ants use a balance of pheromone and distance heuristics.
 * 
 * Worker ants are designed to exploit existing pheromone trails while also
 * considering traffic conditions. They have higher pheromone sensitivity.
 */
class WorkerAnt : public Ant {
private:
    float alpha_;  // Pheromone weight
    float beta_;   // Distance/traffic weight

public:
    /**
     * @brief Constructor for WorkerAnt.
     * @param start_city Initial city for the ant.
     * @param ant_id Unique identifier for this ant.
     * @param alpha Pheromone influence parameter.
     * @param beta Distance/traffic influence parameter.
     * @param seed RNG seed for stochastic route construction.
     */
    WorkerAnt(int start_city,
              int ant_id,
              float alpha = 1.0f,
              float beta = 2.0f,
              std::uint32_t seed = std::random_device{}());

    /**
     * @brief Choose the next city using pheromone-distance tradeoff.
     * @param graph Reference to the TrafficGraph.
     * @param current_time Current simulation time.
     * @return Index of the next city to visit.
     */
    int chooseNextCity(const TrafficGraph& graph, double current_time) override;
};

/**
 * @class ScoutAnt
 * @brief Scout ants prioritize exploration of new routes over pheromone trails.
 * 
 * Scout ants are designed to discover new solutions and escape local optima.
 * They have lower pheromone sensitivity and higher randomness.
 */
class ScoutAnt : public Ant {
private:
    float alpha_;               // Pheromone weight (lower than WorkerAnt)
    float beta_;                // Distance/traffic weight (higher than WorkerAnt)
    float gamma_;               // Pheromone avoidance factor for exploration
    float exploration_factor_;  // Randomness factor for exploration

public:
    /**
     * @brief Constructor for ScoutAnt.
     * @param start_city Initial city for the ant.
     * @param ant_id Unique identifier for this ant.
     * @param alpha Pheromone influence parameter (lower for exploration).
     * @param beta Distance/traffic influence parameter.
     * @param gamma Pheromone avoidance factor.
     * @param exploration_factor Randomness for exploration.
     * @param seed RNG seed for stochastic route construction.
     */
    ScoutAnt(int start_city,
             int ant_id,
             float alpha = 0.3f,
             float beta = 2.5f,
             float gamma = 2.0f,
             float exploration_factor = 0.3f,
             std::uint32_t seed = std::random_device{}());

    /**
     * @brief Choose the next city with emphasis on exploration.
     * @param graph Reference to the TrafficGraph.
     * @param current_time Current simulation time.
     * @return Index of the next city to visit.
     */
    int chooseNextCity(const TrafficGraph& graph, double current_time) override;
};

#endif // ANT_H
