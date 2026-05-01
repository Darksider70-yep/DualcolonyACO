#include <atomic>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "ColonyManager.h"
#include "TrafficGraph.h"

int main() {
    try {
        const std::string tsp_file = "data/kroA100.tsp";
        const std::string output_csv = "scripts/gamma_ablation_kroA100.csv";

        constexpr int total_ants = 50;
        constexpr float scout_ratio = 0.1f;  // 10%
        const int num_scouts =
            static_cast<int>(std::lround(static_cast<double>(total_ants) * scout_ratio));
        const int num_workers = total_ants - num_scouts;

        constexpr int total_runs = 30;
        constexpr int total_iterations = 750;

        constexpr float alpha = 1.0f;
        constexpr float beta = 2.0f;
        constexpr float evaporation = 0.6f;
        constexpr float pheromone_deposit_factor = 1.0f;

        constexpr TrafficGraph::VolatilityMode traffic_mode =
            TrafficGraph::VolatilityMode::NORMAL;  // Moderate dynamic sine-wave traffic
        constexpr float traffic_volatility_multiplier = 1.0f;

        const std::vector<double> gamma_values = {0.5, 1.0, 2.0, 3.0, 5.0};
        const std::vector<std::string> gamma_labels = {
            "Weak avoidance",
            "Linear avoidance",
            "Our baseline squared avoidance",
            "Cubic avoidance",
            "Extreme deterministic avoidance"};

        std::cout << "Gamma ablation study on " << tsp_file << "\n";
        std::cout << "Configuration: N=" << total_ants
                  << ", Workers=" << num_workers
                  << ", Scouts=" << num_scouts
                  << ", Iterations=" << total_iterations
                  << ", Runs/config=" << total_runs << "\n";
        std::cout << "Traffic mode: MODERATE sine-wave (VolatilityMode::NORMAL)\n";

        std::vector<double> mean_global_best(gamma_values.size(), 0.0);

        for (std::size_t g = 0; g < gamma_values.size(); ++g) {
            const double gamma = gamma_values[g];
            std::vector<double> run_best_costs(total_runs, 0.0);
            std::atomic<int> completed_runs{0};

            std::cout << "\nRunning gamma=" << gamma << " (" << gamma_labels[g] << ")\n";

#ifdef DUALCOLONY_OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int run = 0; run < total_runs; ++run) {
                TrafficGraph run_graph(0, evaporation, traffic_mode, traffic_volatility_multiplier);
                run_graph.loadTSPLIB(tsp_file);
                run_graph.resetPheromones();
                run_graph.randomizePhaseShifts();

                ColonyManager colony(run_graph,
                                     num_workers,
                                     num_scouts,
                                     alpha,
                                     beta,
                                     evaporation,
                                     scout_ratio,
                                     pheromone_deposit_factor,
                                     static_cast<float>(gamma));

                for (int iter = 0; iter < total_iterations; ++iter) {
                    colony.runIteration();
                }

                run_best_costs[static_cast<std::size_t>(run)] = colony.getBestTourTime();

                const int done = ++completed_runs;
#ifdef DUALCOLONY_OPENMP
#pragma omp critical
#endif
                std::cout << "  Progress: " << done << "/" << total_runs << "\n";
            }

            const double sum = std::accumulate(run_best_costs.begin(), run_best_costs.end(), 0.0);
            mean_global_best[g] = sum / static_cast<double>(total_runs);
        }

        std::ofstream csv(output_csv);
        if (!csv.is_open()) {
            throw std::runtime_error("Failed to open " + output_csv + " for writing");
        }

        csv << "Gamma,Avoidance_Regime,Mean_Global_Best_Cost\n";
        csv << std::fixed << std::setprecision(6);
        for (std::size_t i = 0; i < gamma_values.size(); ++i) {
            csv << gamma_values[i] << ","
                << gamma_labels[i] << ","
                << mean_global_best[i] << "\n";
        }
        csv.close();

        std::cout << "\nAblation summary written to " << output_csv << "\n";
        std::cout << "Gamma,Avoidance_Regime,Mean_Global_Best_Cost\n";
        std::cout << std::fixed << std::setprecision(6);
        for (std::size_t i = 0; i < gamma_values.size(); ++i) {
            std::cout << gamma_values[i] << ","
                      << gamma_labels[i] << ","
                      << mean_global_best[i] << "\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
