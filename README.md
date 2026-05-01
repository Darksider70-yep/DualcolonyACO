—The Dynamic Traveling Salesperson Problem
(DTSP) presents significant challenges for traditional routing
algorithms, particularly when edge costs fluctuate continuously
due to time-dependent traffic conditions. Standard Ant Colony
Optimization (ACO) architectures often suffer from pheromone
stagnation in such environments, trapping the swarm in obsolete,
suboptimal routes. This paper proposes a novel Dual-Colony
ACO system designed specifically for continuous, sine-wave
traffic disruptions. By partitioning the swarm into “Worker”
ants (exploiting reinforced trails) and “Scout” ants (actively
avoiding pheromones to map shadow networks), the algorithm
dynamically adapts to real-time volatility. Benchmarked against
the TSPLIB berlin52 and kroA100 datasets over 30 independent
statistical runs, the proposed approach demonstrates superior
global convergence compared to standard ACO baselines. Fur
thermore, a parameter sensitivity analysis and boundary con
dition stress test (Colony Amnesia) are presented, rigorously
defining the mathematical limits of exploratory swarms under
extreme stochastic congestion.
