#ifndef TRAFFIC_GRAPH_H
#define TRAFFIC_GRAPH_H

#include <vector>
#include <cstring>
#include <string>
#include <utility>

/**
 * @class TrafficGraph
 * @brief Represents the dynamic traffic network for the DTSP.
 * 
 * Maintains edge weights including base distances, current traffic multipliers,
 * and pheromone levels. Supports dynamic traffic events and pheromone evaporation.
 */
class TrafficGraph {
public:
    enum class VolatilityMode {
        NORMAL,
        HIGH
    };

private:
    int num_cities_;
    std::vector<std::vector<float>> base_distance_;
    std::vector<std::vector<float>> current_traffic_multiplier_;
    std::vector<std::vector<float>> pheromone_level_;
    std::vector<std::vector<float>> edge_phase_shifts_;  // Phase shift for each edge
    std::vector<std::pair<float, float>> city_coordinates_;  // (X, Y) coordinates for Euclidean distance
    float pheromone_evaporation_rate_;
    float initial_pheromone_level_;
    VolatilityMode volatility_mode_;
    float volatility_multiplier_;
    float frequency_multiplier_;

public:
    /**
     * @brief Constructor for TrafficGraph.
     * @param num_cities Number of cities in the graph.
     * @param evaporation_rate Pheromone evaporation rate (0 to 1).
     * @param volatility_mode Traffic volatility profile.
     * @param volatility_multiplier Baseline multiplier for traffic amplitude.
     */
    TrafficGraph(int num_cities,
                 float evaporation_rate = 0.1f,
                 VolatilityMode volatility_mode = VolatilityMode::NORMAL,
                 float volatility_multiplier = 1.0f);

    /**
     * @brief Destructor for TrafficGraph.
     */
    ~TrafficGraph();

    /**
     * @brief Initialize the graph with base distances (symmetric matrix).
     * @param distances 2D vector of base distances.
     */
    void initializeDistances(const std::vector<std::vector<float>>& distances);

    /**
     * @brief Simulate a traffic jam on an edge.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @param severity Traffic multiplier severity (>1.0).
     */
    void triggerTrafficJam(int cityA, int cityB, float severity);

    /**
     * @brief Reduce traffic multipliers on all edges (recovery).
     */
    void reduceTraffic();

    /**
     * @brief Apply pheromone evaporation across the entire graph.
     */
    void evaporatePheromones();

    /**
     * @brief Reset all pheromone levels to the initial baseline value.
     */
    void resetPheromones();

    /**
     * @brief Regenerate random traffic phase shifts for all edges.
     */
    void randomizePhaseShifts();

    /**
     * @brief Get the effective edge weight (distance * traffic multiplier).
     * @param cityA Start city index.
     * @param cityB End city index.
     * @return Effective edge weight.
     */
    float getEffectiveDistance(int cityA, int cityB) const;

    /**
     * @brief Get the pheromone level on an edge.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @return Pheromone level.
     */
    float getPheromoneLevel(int cityA, int cityB) const;

    /**
     * @brief Set the pheromone level on an edge.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @param level Pheromone level to set.
     */
    void setPheromoneLevel(int cityA, int cityB, float level);

    /**
     * @brief Add pheromone to an edge.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @param amount Pheromone amount to add.
     */
    void addPheromone(int cityA, int cityB, float amount);

    /**
     * @brief Get base distance between two cities.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @return Base distance.
     */
    float getBaseDistance(int cityA, int cityB) const;

    /**
     * @brief Get the number of cities in the graph.
     * @return Number of cities.
     */
    int getNumCities() const;

    /**
     * @brief Get the current traffic multiplier for an edge.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @return Traffic multiplier.
     */
    float getTrafficMultiplier(int cityA, int cityB) const;

    /**
     * @brief Load a TSPLIB format file (EUC_2D).
     * @param filename Path to the .tsp file.
     */
    void loadTSPLIB(const std::string& filename);

    /**
     * @brief Calculate the rush hour traffic multiplier using sine wave formula.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @param current_time Current simulation time.
     * @return Traffic multiplier (1.0 + amplitude * sin^2(...)).
     */
    double calculateRushHourMultiplier(int cityA, int cityB, double current_time) const;

    /**
     * @brief Get the travel time for an edge considering traffic.
     * @param cityA Start city index.
     * @param cityB End city index.
     * @param current_time Current simulation time.
     * @return Travel time (base_distance * traffic_multiplier).
     */
    double getTravelTime(int cityA, int cityB, double current_time) const;

    /**
     * @brief Deposit pheromones along a tour.
     * @param tour Vector of city indices representing the tour.
     * @param pheromone_amount Amount of pheromone to deposit.
     */
    void depositPheromones(const std::vector<int>& tour, double pheromone_amount);
};

#endif // TRAFFIC_GRAPH_H
