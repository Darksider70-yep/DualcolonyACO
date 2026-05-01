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
    try {
        const std::string tsp_file = "data/kroA100.tsp";
        constexpr int total_runs = 30;
        constexpr int total_ants = 50;
        constexpr int total_iterations = 750;
        constexpr float alpha = 1.0f;
        constexpr float beta = 2.0f;
        constexpr float evaporation = 0.1f;
        constexpr float pheromone_deposit_factor = 1.0f;

        struct ScoutRatioConfig {
            int scout_count;
            int ratio_label_percent;
        };
        const std::vector<ScoutRatioConfig> ratio_configs = {
            {0, 0},
            {5, 10},
            {10, 20},
            {15, 30},
            {20, 40},
        };

        TrafficGraph graph(0, evaporation, TrafficGraph::VolatilityMode::HIGH, 1.0f);
        graph.loadTSPLIB(tsp_file);
        if (graph.getNumCities() == 0) {
            std::cout << "ERROR: Could not read the TSP file!" << std::endl;
            return 1;
        }

        std::cout << "Traffic volatility mode: HIGH\n";
        std::cout << "Running scout ratio sweep for 5 configurations.\n";

        std::vector<std::vector<double>> ratio_mean_global_best(
            ratio_configs.size(), std::vector<double>(total_iterations, 0.0));

        for (std::size_t config_index = 0; config_index < ratio_configs.size(); ++config_index) {
            const ScoutRatioConfig config = ratio_configs[config_index];
            const int worker_count = total_ants - config.scout_count;
            const float scout_ratio =
                static_cast<float>(config.scout_count) / static_cast<float>(total_ants);

            for (int run = 0; run < total_runs; ++run) {
                std::cout << "Testing " << config.ratio_label_percent
                          << "% Scout Ratio - Run " << (run + 1)
                          << "/" << total_runs << "...\n";

                graph.resetPheromones();
                graph.randomizePhaseShifts();

                ColonyManager colony(graph,
                                     worker_count,
                                     config.scout_count,
                                     alpha,
                                     beta,
                                     evaporation,
                                     scout_ratio,
                                     pheromone_deposit_factor);

                for (int iteration = 0; iteration < total_iterations; ++iteration) {
                    colony.runIteration();
                    ratio_mean_global_best[config_index][iteration] += colony.getBestTourTime();
                }
            }

            const double inv_runs = 1.0 / static_cast<double>(total_runs);
            for (double& value : ratio_mean_global_best[config_index]) {
                value *= inv_runs;
            }
        }

        std::ofstream csv("scripts/journal_ratio_sweep.csv");
        if (!csv.is_open()) {
            throw std::runtime_error("Failed to open scripts/journal_ratio_sweep.csv for writing");
        }
        csv << "Iteration,Ratio_0,Ratio_10,Ratio_20,Ratio_30,Ratio_40\n";

        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            csv << iteration;
            for (std::size_t config_index = 0; config_index < ratio_mean_global_best.size();
                 ++config_index) {
                csv << "," << ratio_mean_global_best[config_index][static_cast<std::size_t>(iteration - 1)];
            }
            csv << "\n";
        }
        csv.close();

        std::cout << "Scout ratio sweep written to scripts/journal_ratio_sweep.csv\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
