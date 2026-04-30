#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include "TrafficGraph.h"
#include "ColonyManager.h"

/**
 * @brief Entry point for the Dual-Colony ACO algorithm.
 * 
 * This program solves the Dynamic Traveling Salesperson Problem (DTSP)
 * using a novel dual-colony approach with both worker and scout ants.
 */
int main() {
    std::cout << "========================================\n";
    std::cout << "Dual-Colony ACO for Dynamic TSP\n";
    std::cout << "========================================\n\n";

    try {
        // Load Berlin52 TSPLIB benchmark instance (EUC_2D)
        const std::string tsp_file = "data/berlin52.tsp";
        TrafficGraph graph(0, 0.1f);
        std::cout << "Loading TSPLIB data..." << std::endl;
        graph.loadTSPLIB(tsp_file);
        std::cout << "Cities loaded: " << graph.getNumCities() << std::endl;

        if (graph.getNumCities() == 0) {
            std::cout << "ERROR: Could not read the TSP file!" << std::endl;
            return 1;
        }

        // Baseline academic parameters
        constexpr int total_ants = 50;
        constexpr float scout_ratio = 0.1f;
        const int num_scouts = static_cast<int>(total_ants * scout_ratio);  // 10% scouts => 5
        const int num_workers = total_ants - num_scouts;

        ColonyManager colony(graph,
                             num_workers,
                             num_scouts,
                             1.0f,         // alpha
                             2.0f,         // beta
                             0.1f,         // evaporation
                             scout_ratio,  // scout ratio
                             1.0f);

        std::cout << "Graph initialized from: " << tsp_file << "\n";
        std::cout << "Cities: " << graph.getNumCities() << "\n";
        std::cout << "Worker ants: " << colony.getNumWorkerAnts() << "\n";
        std::cout << "Scout ants: " << colony.getNumScoutAnts() << "\n\n";

        std::ofstream csv("scripts/results.csv");
        if (!csv.is_open()) {
            throw std::runtime_error("Failed to open scripts/results.csv for writing");
        }
        csv << "Iteration,Iteration_Best,Global_Best\n";

        constexpr int total_iterations = 500;
        std::cout << "Running algorithm for " << total_iterations << " iterations...\n";
        std::cout << "Starting simulation..." << std::endl;

        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            colony.runIteration();
            csv << iteration << ","
                << colony.getLastIterationBestTime() << ","
                << colony.getBestTourTime() << "\n";
        }
        std::cout << "Simulation complete! File written." << std::endl;

        csv.close();

        // Display results
        std::cout << "\nAlgorithm completed!\n";
        std::cout << "Best tour time found: " << colony.getBestTourTime() << "\n";
        std::cout << "Telemetry written to scripts/results.csv\n";
        
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
