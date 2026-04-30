#include <iostream>
#include <vector>
#include <cmath>
#include "TrafficGraph.h"
#include "ColonyManager.h"

/**
 * @brief Entry point for the Dual-Colony ACO algorithm.
 * 
 * This program solves the Dynamic Traveling Salesperson Problem (DTSP)
 * using a novel dual-colony approach with both worker and scout ants.
 */
int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "Dual-Colony ACO for Dynamic TSP\n";
    std::cout << "========================================\n\n";

    try {
        // Initialize graph with a small test case
        int num_cities = 5;
        TrafficGraph graph(num_cities, 0.1f);  // 10% evaporation rate

        // Create a simple distance matrix for testing
        std::vector<std::vector<float>> distances(num_cities, std::vector<float>(num_cities, 0.0f));
        for (int i = 0; i < num_cities; ++i) {
            for (int j = i + 1; j < num_cities; ++j) {
                float dist = (i + 1) * (j + 1) * 1.5f;  // Simple formula for demo
                distances[i][j] = dist;
                distances[j][i] = dist;
            }
        }
        graph.initializeDistances(distances);

        // Initialize colony manager
        ColonyManager colony(graph, 
                           10,    // 10 worker ants
                           5,     // 5 scout ants
                           1.0f,  // alpha
                           2.0f,  // beta
                           0.1f,  // evaporation rate
                           0.2f,  // scout ratio
                           1.0f); // pheromone deposit factor

        std::cout << "Graph initialized with " << num_cities << " cities\n";
        std::cout << "Worker ants: " << colony.getNumWorkerAnts() << "\n";
        std::cout << "Scout ants: " << colony.getNumScoutAnts() << "\n\n";

        // Run the algorithm for 100 iterations
        std::cout << "Running algorithm for 100 iterations...\n";
        colony.runAlgorithm(100);

        // Display results
        std::cout << "\nAlgorithm completed!\n";
        std::cout << "Best tour time found: " << colony.getBestTourTime() << "\n";
        
        const std::vector<int>& best_tour = colony.getBestTourPath();
        std::cout << "Best tour path: ";
        for (int city : best_tour) {
            std::cout << city << " ";
        }
        std::cout << "\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
