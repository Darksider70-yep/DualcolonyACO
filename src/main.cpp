#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
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
        // Load kroA100 TSPLIB benchmark instance (EUC_2D)
        const std::string tsp_file = "data/kroA100.tsp";
        constexpr int total_ants = 50;
        constexpr int total_iterations = 750;
        constexpr float alpha = 1.0f;
        constexpr float beta = 2.0f;
        constexpr float evaporation = 0.1f;
        constexpr float pheromone_deposit_factor = 1.0f;

        TrafficGraph graph(0, evaporation);
        std::cout << "Loading TSPLIB data..." << std::endl;
        graph.loadTSPLIB(tsp_file);
        std::cout << "Cities loaded: " << graph.getNumCities() << std::endl;

        if (graph.getNumCities() == 0) {
            std::cout << "ERROR: Could not read the TSP file!" << std::endl;
            return 1;
        }

        std::vector<double> standard_results;
        std::vector<double> dual_results;
        standard_results.reserve(total_iterations);
        dual_results.reserve(total_iterations);

        std::cout << "Graph initialized from: " << tsp_file << "\n";
        std::cout << "Cities: " << graph.getNumCities() << "\n\n";

        // Phase A: Standard baseline (50 workers, 0 scouts)
        std::cout << "Phase A: Standard Baseline (50 Workers, 0 Scouts)\n";
        ColonyManager colony(graph,
                             total_ants,
                             0,
                             alpha,
                             beta,
                             evaporation,
                             0.0f,
                             pheromone_deposit_factor);

        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            colony.runIteration();
            standard_results.push_back(colony.getBestTourTime());
        }

        std::cout << "Phase A best tour time: " << colony.getBestTourTime() << "\n\n";

        // Independent run setup for Phase B
        std::cout << "Resetting pheromones to baseline before Phase B...\n";
        graph.resetPheromones();

        // Phase B: Dual-colony (45 workers, 5 scouts)
        std::cout << "Phase B: Dual-Colony (45 Workers, 5 Scouts)\n";
        colony.configurePopulation(total_ants, 0.1f);  // 10% scouts => 45/5 split

        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            colony.runIteration();
            dual_results.push_back(colony.getBestTourTime());
        }

        std::cout << "Phase B best tour time: " << colony.getBestTourTime() << "\n\n";

        std::ofstream csv("scripts/journal_baseline.csv");
        if (!csv.is_open()) {
            throw std::runtime_error("Failed to open scripts/journal_baseline.csv for writing");
        }
        csv << "Iteration,Standard_ACO,Dual_Colony_ACO\n";
        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            csv << iteration << ","
                << standard_results[static_cast<std::size_t>(iteration - 1)] << ","
                << dual_results[static_cast<std::size_t>(iteration - 1)] << "\n";
        }
        csv.close();

        std::cout << "Head-to-head telemetry written to scripts/journal_baseline.csv\n";
        std::cout << "Standard final global best: " << standard_results.back() << "\n";
        std::cout << "Dual-colony final global best: " << dual_results.back() << "\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
