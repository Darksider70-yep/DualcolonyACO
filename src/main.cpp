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

        constexpr int standard_workers = 50;
        constexpr int standard_scouts = 0;
        constexpr int mmas_workers = 50;
        constexpr int mmas_scouts = 0;
        constexpr int dual_workers = 45;
        constexpr int dual_scouts = 5;

        constexpr float standard_scout_ratio = 0.0f;
        constexpr float mmas_scout_ratio = 0.0f;
        constexpr float dual_scout_ratio = 0.1f;

        constexpr float mmas_tau_min = 0.01f;
        constexpr float mmas_tau_max = 10.0f;

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

        std::vector<std::vector<double>> standard_runs(
            total_runs, std::vector<double>(total_iterations, 0.0));
        std::vector<std::vector<double>> mmas_runs(
            total_runs, std::vector<double>(total_iterations, 0.0));
        std::vector<std::vector<double>> dual_runs(
            total_runs, std::vector<double>(total_iterations, 0.0));

        const auto clampPheromoneBounds = [&graph](float tau_min, float tau_max) {
            const int cities = graph.getNumCities();
            for (int i = 0; i < cities; ++i) {
                for (int j = 0; j < cities; ++j) {
                    const float clamped =
                        std::clamp(graph.getPheromoneLevel(i, j), tau_min, tau_max);
                    graph.setPheromoneLevel(i, j, clamped);
                }
            }
        };

        for (int run = 0; run < total_runs; ++run) {
            std::cout << "Phase 1/3 (Standard ACO, 50 Workers / 0 Scouts) - Run "
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
            std::cout << "Phase 2/3 (MMAS Baseline, 50 Workers / 0 Scouts) - Run "
                      << (run + 1)
                      << "/" << total_runs << "...\n";

            graph.resetPheromones();
            graph.randomizePhaseShifts();

            ColonyManager colony(graph,
                                 mmas_workers,
                                 mmas_scouts,
                                 alpha,
                                 beta,
                                 evaporation,
                                 mmas_scout_ratio,
                                 pheromone_deposit_factor);

            for (int iteration = 0; iteration < total_iterations; ++iteration) {
                colony.runIteration();
                clampPheromoneBounds(mmas_tau_min, mmas_tau_max);
                mmas_runs[run][iteration] = colony.getBestTourTime();
            }
        }

        for (int run = 0; run < total_runs; ++run) {
            std::cout << "Phase 3/3 (Dual-Colony, 45 Workers / 5 Scouts) - Run "
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
        std::vector<double> mmas_mean(total_iterations, 0.0);
        std::vector<double> dual_mean(total_iterations, 0.0);

        for (int iteration = 0; iteration < total_iterations; ++iteration) {
            double std_sum = 0.0;
            double mmas_sum = 0.0;
            double dual_sum = 0.0;

            for (int run = 0; run < total_runs; ++run) {
                const double std_value = standard_runs[run][iteration];
                const double mmas_value = mmas_runs[run][iteration];
                const double dual_value = dual_runs[run][iteration];

                std_sum += std_value;
                mmas_sum += mmas_value;
                dual_sum += dual_value;
            }

            const double inv_runs = 1.0 / static_cast<double>(total_runs);
            std_mean[iteration] = std_sum * inv_runs;
            mmas_mean[iteration] = mmas_sum * inv_runs;
            dual_mean[iteration] = dual_sum * inv_runs;
        }

        std::ofstream csv("scripts/journal_baseline_ch150.csv");
        if (!csv.is_open()) {
            throw std::runtime_error("Failed to open scripts/journal_baseline_ch150.csv for writing");
        }
        csv << "Iteration,Std_ACO,MMAS,Dual_Colony\n";

        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            csv << iteration << ","
                << std_mean[static_cast<std::size_t>(iteration - 1)] << ","
                << mmas_mean[static_cast<std::size_t>(iteration - 1)] << ","
                << dual_mean[static_cast<std::size_t>(iteration - 1)] << "\n";
        }
        csv.close();

        std::cout << "Scalability baseline written to scripts/journal_baseline_ch150.csv\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
