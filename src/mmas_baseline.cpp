#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <random>
#include <algorithm>

#include "TrafficGraph.h"
#include "ColonyManager.h"

/**
 * @brief Calculate the standard deviation of a dataset.
 */
double calculateStdDev(const std::vector<double>& data, double mean) {
    double sum_sq_diff = 0.0;
    for (double val : data) {
        sum_sq_diff += (val - mean) * (val - mean);
    }
    return std::sqrt(sum_sq_diff / static_cast<double>(data.size()));
}

int main() {
    try {
        const std::string tsp_file = "data/berlin52.tsp";
        constexpr int num_runs = 30;
        constexpr int iterations_per_run = 500;
        
        // MMAS / Algorithm Configuration
        constexpr int total_ants = 52;     // Typically n ants for n cities
        constexpr float scout_ratio = 0.0f; // STRICTLY 0% Scouts for MMAS baseline
        constexpr float alpha = 1.0f;
        constexpr float beta = 2.0f;
        constexpr float rho = 0.1f;        // Evaporation rate
        
        // MMAS Pheromone Bounds
        // Based on berlin52 heuristics: tau_max ~= 1/(rho * L_opt)
        // With Q=100, tau_max is scaled. We'll use robust bounds.
        constexpr float tau_max = 0.5f;
        constexpr float tau_min = 0.005f;

        std::vector<double> final_best_costs;
        final_best_costs.reserve(num_runs);

        std::cout << "========================================================\n";
        std::cout << "MMAS Baseline Extraction: berlin52\n";
        std::cout << "Configuration: N=" << total_ants << ", Scouts=0%, Iterations=" << iterations_per_run << "\n";
        std::cout << "MMAS Bounds: [" << tau_min << ", " << tau_max << "], rho=" << rho << "\n";
        std::cout << "========================================================\n";

        // Seed generator for the 30 independent runs
        std::random_device rd;
        std::mt19937 run_seeder(rd());

        for (int run = 0; run < num_runs; ++run) {
            // 1. Initialize Graph and Load Dataset
            TrafficGraph graph(0, rho, TrafficGraph::VolatilityMode::NORMAL, 1.0f);
            graph.loadTSPLIB(tsp_file);
            
            // MMAS initialization: Start all edges at tau_max
            for (int i = 0; i < graph.getNumCities(); ++i) {
                for (int j = 0; j < graph.getNumCities(); ++j) {
                    graph.setPheromoneLevel(i, j, tau_max);
                }
            }

            // 2. Initialize Colony Manager
            // Seed each run differently for stochastic variance
            std::uint32_t run_seed = run_seeder();
            ColonyManager colony(graph, total_ants, 0, alpha, beta, rho, scout_ratio);
            
            // Lock into MMAS mode
            colony.enableMMAS(true, tau_min, tau_max);

            // 3. Execute Run
            for (int iter = 0; iter < iterations_per_run; ++iter) {
                colony.runIteration();
            }

            double best_cost = colony.getBestTourTime();
            final_best_costs.push_back(best_cost);

            std::cout << "Run " << std::setw(2) << run + 1 << "/" << num_runs 
                      << " | Global Best: " << std::fixed << std::setprecision(2) << best_cost << "\n";
        }

        // 4. Calculate Final Statistics
        double sum = std::accumulate(final_best_costs.begin(), final_best_costs.end(), 0.0);
        double mean = sum / static_cast<double>(num_runs);
        double std_dev = calculateStdDev(final_best_costs, mean);

        std::cout << "\n========================================================\n";
        std::cout << "FINAL STATISTICAL RESULTS (n=30)\n";
        std::cout << "Dataset: berlin52\n";
        std::cout << "Mean Global Best Cost: " << std::fixed << std::setprecision(4) << mean << "\n";
        std::cout << "Standard Deviation:    " << std::fixed << std::setprecision(4) << std_dev << "\n";
        std::cout << "========================================================\n";

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error during execution: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
