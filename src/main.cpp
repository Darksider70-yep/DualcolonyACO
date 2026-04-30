#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
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
        const std::string tsp_file = "data/kroA100.tsp";
        constexpr int total_runs = 30;
        constexpr int total_ants = 50;
        constexpr int total_iterations = 750;
        constexpr float alpha = 1.0f;
        constexpr float beta = 2.0f;
        constexpr float evaporation = 0.1f;
        constexpr float pheromone_deposit_factor = 1.0f;
        constexpr float standard_scout_ratio = 0.0f;
        constexpr float dual_scout_ratio = 0.1f;

        TrafficGraph graph(0, evaporation);
        graph.loadTSPLIB(tsp_file);
        if (graph.getNumCities() == 0) {
            std::cout << "ERROR: Could not read the TSP file!" << std::endl;
            return 1;
        }

        std::vector<std::vector<double>> standard_runs(
            total_runs, std::vector<double>(total_iterations, 0.0));
        std::vector<std::vector<double>> dual_runs(
            total_runs, std::vector<double>(total_iterations, 0.0));

        for (int run = 0; run < total_runs; ++run) {
            std::cout << "Executing Standard Run " << (run + 1) << "/" << total_runs << "...\n";
            graph.resetPheromones();
            graph.randomizePhaseShifts();

            ColonyManager colony(graph,
                                 total_ants,
                                 0,
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
            std::cout << "Executing Dual Run " << (run + 1) << "/" << total_runs << "...\n";
            graph.resetPheromones();
            graph.randomizePhaseShifts();

            ColonyManager colony(graph,
                                 total_ants,
                                 0,
                                 alpha,
                                 beta,
                                 evaporation,
                                 dual_scout_ratio,
                                 pheromone_deposit_factor);
            colony.configurePopulation(total_ants, dual_scout_ratio);

            for (int iteration = 0; iteration < total_iterations; ++iteration) {
                colony.runIteration();
                dual_runs[run][iteration] = colony.getBestTourTime();
            }
        }

        std::vector<double> std_mean(total_iterations, 0.0);
        std::vector<double> std_dev(total_iterations, 0.0);
        std::vector<double> dual_mean(total_iterations, 0.0);
        std::vector<double> dual_dev(total_iterations, 0.0);

        for (int iteration = 0; iteration < total_iterations; ++iteration) {
            double std_sum = 0.0;
            double std_sq_sum = 0.0;
            double dual_sum = 0.0;
            double dual_sq_sum = 0.0;

            for (int run = 0; run < total_runs; ++run) {
                const double std_value = standard_runs[run][iteration];
                const double dual_value = dual_runs[run][iteration];

                std_sum += std_value;
                std_sq_sum += std_value * std_value;
                dual_sum += dual_value;
                dual_sq_sum += dual_value * dual_value;
            }

            const double inv_runs = 1.0 / static_cast<double>(total_runs);
            std_mean[iteration] = std_sum * inv_runs;
            dual_mean[iteration] = dual_sum * inv_runs;

            const double n = static_cast<double>(total_runs);
            const double std_variance = (total_runs > 1)
                ? std::max(0.0, (std_sq_sum - ((std_sum * std_sum) / n)) / (n - 1.0))
                : 0.0;
            const double dual_variance = (total_runs > 1)
                ? std::max(0.0, (dual_sq_sum - ((dual_sum * dual_sum) / n)) / (n - 1.0))
                : 0.0;

            std_dev[iteration] = std::sqrt(std_variance);
            dual_dev[iteration] = std::sqrt(dual_variance);
        }

        std::ofstream csv("scripts/journal_statistics.csv");
        if (!csv.is_open()) {
            throw std::runtime_error("Failed to open scripts/journal_statistics.csv for writing");
        }
        csv << "Iteration,Std_Mean,Std_Dev,Dual_Mean,Dual_Dev\n";
        for (int iteration = 1; iteration <= total_iterations; ++iteration) {
            csv << iteration << ","
                << std_mean[static_cast<std::size_t>(iteration - 1)] << ","
                << std_dev[static_cast<std::size_t>(iteration - 1)] << ","
                << dual_mean[static_cast<std::size_t>(iteration - 1)] << ","
                << dual_dev[static_cast<std::size_t>(iteration - 1)] << "\n";
        }
        csv.close();

        std::cout << "Statistics written to scripts/journal_statistics.csv\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
