#ifndef COLONY_MANAGER_H
#define COLONY_MANAGER_H

#include <vector>
#include <memory>
#include <random>
#include "TrafficGraph.h"
#include "Ant.h"

/**
 * @class ColonyManager
 * @brief Manages the dual-colony ACO algorithm.
 * 
 * Orchestrates the interaction between WorkerAnt and ScoutAnt colonies,
 * handles pheromone updates, and coordinates iterations of the algorithm
 * to solve the Dynamic Traveling Salesperson Problem.
 */
class ColonyManager {
private:
    TrafficGraph& traffic_graph_;
    std::mt19937 rng_;
    std::vector<std::unique_ptr<WorkerAnt>> worker_ants_;
    std::vector<std::unique_ptr<ScoutAnt>> scout_ants_;

    void rebuildAntPopulation(int num_workers, int num_scouts);
    
    // Algorithm parameters
    float alpha_;                      // Pheromone influence factor
    float beta_;                       // Distance/traffic influence factor
    float evaporation_rate_;           // Pheromone evaporation rate
    float scout_ratio_;                // Ratio of scout ants to total ants
    float pheromone_deposit_factor_;   // Factor for pheromone deposition
    double Q_constant_;                // Pheromone deposition constant
    
    // Statistics and state
    int total_iterations_;
    int current_iteration_;
    double best_tour_time_;
    double last_iteration_best_time_;
    std::vector<int> best_tour_path_;

public:
    /**
     * @brief Constructor for ColonyManager.
     * @param graph Reference to the TrafficGraph.
     * @param num_workers Number of worker ants.
     * @param num_scouts Number of scout ants.
     * @param alpha Pheromone influence parameter.
     * @param beta Distance/traffic influence parameter.
     * @param evaporation_rate Pheromone evaporation rate (0 to 1).
     * @param scout_ratio Ratio of scout ants to total ants.
     * @param pheromone_deposit_factor Factor for pheromone deposition.
     */
    ColonyManager(TrafficGraph& graph, 
                  int num_workers, 
                  int num_scouts,
                  float alpha = 1.0f,
                  float beta = 2.0f,
                  float evaporation_rate = 0.6f,
                  float scout_ratio = 0.2f,
                  float pheromone_deposit_factor = 1.0f);

    /**
     * @brief Destructor for ColonyManager.
     */
    ~ColonyManager();

    /**
     * @brief Initialize the swarm of ants and set starting positions.
     */
    void initializeSwarm();

    /**
     * @brief Execute a single iteration of the algorithm.
     * 
     * This includes:
     * - Running each ant to completion
     * - Updating global pheromones based on tour quality
     * - Applying pheromone evaporation
     * - Resetting ants for the next iteration
     */
    void runIteration();

    /**
     * @brief Update global pheromone levels based on ant tours.
     * 
     * Implements pheromone deposition proportional to tour quality.
     * Updates are applied globally after all ants complete their tours.
     */
    void updateGlobalPheromones();

    /**
     * @brief Execute multiple iterations of the algorithm.
     * @param num_iterations Number of iterations to run.
     */
    void runAlgorithm(int num_iterations);

    /**
     * @brief Get the best tour found so far.
     * @return Vector of city indices representing the best tour.
     */
    const std::vector<int>& getBestTourPath() const;

    /**
     * @brief Get the best tour time found so far.
     * @return Best tour time (total path cost).
     */
    double getBestTourTime() const;

    /**
     * @brief Get the best tour time found in the most recent iteration.
     * @return Iteration best tour time.
     */
    double getLastIterationBestTime() const;

    /**
     * @brief Reset the algorithm state for a new run.
     */
    void reset();

    /**
     * @brief Get the number of worker ants.
     * @return Number of worker ants.
     */
    int getNumWorkerAnts() const;

    /**
     * @brief Get the number of scout ants.
     * @return Number of scout ants.
     */
    int getNumScoutAnts() const;

    /**
     * @brief Get the current iteration number.
     * @return Current iteration count.
     */
    int getCurrentIteration() const;

    /**
     * @brief Set parameters for the algorithm.
     * @param alpha Pheromone influence parameter.
     * @param beta Distance/traffic influence parameter.
     * @param evaporation_rate Pheromone evaporation rate.
     * @param scout_ratio Ratio of scout ants to total ants.
     */
    void setParameters(float alpha, float beta, float evaporation_rate, float scout_ratio);

    /**
     * @brief Reconfigure worker/scout split from a total-ant budget and scout ratio.
     * @param total_ants Total number of ants in the colony.
     * @param scout_ratio Ratio in [0, 1] used to allocate scout ants.
     */
    void configurePopulation(int total_ants, float scout_ratio);
};

#endif // COLONY_MANAGER_H
