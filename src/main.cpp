#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cmath>
#include "TrafficGraph.h"
#include "ColonyManager.h"

/**
 * @brief Entry point for the Dual-Colony ACO algorithm.
 * 
 * This program solves the Dynamic Traveling Salesperson Problem (DTSP)
 * using a novel dual-colony approach with both worker and scout ants.
 */
int main() {
    try {
        const std::string tsp_file = "data/ch150.tsp";
        constexpr int total_runs = 30;
        constexpr int total_iterations = 1000;
        constexpr float alpha = 1.0f;
        constexpr float beta = 2.0f;
        constexpr float evaporation = 0.6f;
        constexpr float pheromone_deposit_factor = 1.0f;

        constexpr int standard_workers = 150;
        constexpr int standard_scouts = 0;
        constexpr int dual_workers = 135;
        constexpr int dual_scouts = 15;

        constexpr float standard_scout_ratio = 0.0f;
        constexpr float dual_scout_ratio = 0.1f;

        TrafficGraph graph(0, evaporation, TrafficGraph::VolatilityMode::HIGH, 1.0f);
        graph.loadTSPLIB(tsp_file);
        if (graph.getNumCities() == 0) {
            std::cout << "ERROR: Could not read the TSP file!" << std::endl;
            return 1;
        }

        std::cout << "Loaded dataset: " << tsp_file
                  << " (" << graph.getNumCities() << " cities)\n";
        std::cout << "Traffic volatility mode: HIGH\n";
        std::cout << "Pheromone evaporation (rho): " << evaporation << "\n";
        std::cout << "Iterations per run (I_max): " << total_iterations << "\n";
        std::cout << "Scaled swarm configuration: N=150\n";

        std::vector<std::vector<double>> standard_runs(
            total_runs, std::vector<double>(total_iterations, 0.0));
        std::vector<std::vector<double>> dual_runs(
            total_runs, std::vector<double>(total_iterations, 0.0));

        for (int run = 0; run < total_runs; ++run) {
            std::cout << "Phase A (Standard ACO Scaled, 150 Workers / 0 Scouts) - Run "
                      << (run + 1)
                      << "/" << total_runs << "...\n";

            graph.resetPheromones();
            graph.randomizePhaseShifts();

            ColonyManager colony(graph,
                                 standard_workers,
                                 standard_scouts,
                                 alpha,
                                 beta,
                                 evaporation,
                                 standard_scout_ratio,
                                 pheromone_deposit_factor);

            for (int iteration = 0; iteration < total_iterations; ++iteration) {
                colony.runIteration();
                standard_runs[run][iteration] = colony.getBestTourTime();
            }
        }

        for (int run = 0; run < total_runs; ++run) {
            std::cout << "Phase B (Dual-Colony Scaled, 135 Workers / 15 Scouts) - Run "
                      << (run + 1)
                      << "/" << total_runs << "...\n";

            graph.resetPheromones();
            graph.randomizePhaseShifts();

            ColonyManager colony(graph,
                                 dual_workers,
                                 dual_scouts,
                                 alpha,
                                 beta,
                                 evaporation,
                                 dual_scout_ratio,
                                 pheromone_deposit_factor);

            for (int iteration = 0; iteration < total_iterations; ++iteration) {
                colony.runIteration();
                dual_runs[run][iteration] = colony.getBestTourTime();
            }
        }

        std::vector<double> std_mean(total_iterations, 0.0);
        std::vector<double> dual_mean(total_iterations, 0.0);

        for (int iteration = 0; iteration < total_iterations; ++iteration) {
            double std_sum = 0.0;
            double dual_sum = 0.0;

            for (int run = 0; run < total_runs; ++run) {
                const double std_value = standard_runs[run][iteration];
                const double dual_value = dual_runs[run][iteration];

                std_sum += std_value;
                dual_sum += dual_value;
            }

            const double inv_runs = 1.0 / static_cast<double>(total_runs);
            std_mean[iteration] = std_sum * inv_runs;
            dual_mean[iteration] = dual_sum * inv_runs;
        }

        std::ofstream csv("scripts/journal_scaled_ch150.csv");
        if (!csv.is_open()) {
            throw std::runtime_error("Failed to open scripts/journal_scaled_ch150.csv for writing");
        }
        csv << "Iteration,Std_ACO_Scaled,Dual_Colony_Scaled\n";

        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            csv << iteration << ","
                << std_mean[static_cast<std::size_t>(iteration - 1)] << ","
                << dual_mean[static_cast<std::size_t>(iteration - 1)] << "\n";
        }
        csv.close();

        std::cout << "Scaled swarm results written to scripts/journal_scaled_ch150.csv\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
