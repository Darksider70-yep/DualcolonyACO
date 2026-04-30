import pandas as pd
import matplotlib.pyplot as plt
import os

def generate_convergence_chart():
    # Construct the path to the CSV file
    current_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(current_dir, "results.csv")

    if not os.path.exists(csv_path):
        print(f"Error: Could not find data file at {csv_path}")
        return

    print("Loading telemetry data...")
    # Load the telemetry data
    data = pd.read_csv(csv_path)

    # Set up the academic plot style
    try:
        plt.style.use('seaborn-v0_8-whitegrid')
    except OSError:
        # Fallback if the specific seaborn style is missing in older matplotlib versions
        plt.style.use('ggplot')
        
    plt.figure(figsize=(10, 6), dpi=300)

    # Plot the Iteration Best (lighter, semi-transparent to show variance)
    plt.plot(data['Iteration'], data['Iteration_Best'], 
             color='#7fb3d5', alpha=0.6, linewidth=1, label='Iteration Best (Swarm Variance)')

    # Plot the Global Best (thick, bold line to show convergence)
    plt.plot(data['Iteration'], data['Global_Best'], 
             color='#c0392b', linewidth=2.5, label='Global Best (Optimal Route Found)')

    # Academic Formatting
    plt.title('Dual-Colony ACO Convergence in Dynamic Traffic (Berlin52)', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Algorithm Iterations', fontsize=12)
    plt.ylabel('Total Route Time (Cost)', fontsize=12)
    plt.legend(loc='upper right', frameon=True, shadow=True)
    
    # Save the chart as a high-res image for the research paper
    output_img = os.path.join(current_dir, "convergence_chart.png")
    plt.savefig(output_img, bbox_inches='tight')
    
    print(f"Success! High-resolution chart saved to: {output_img}")
    plt.show()

if __name__ == "__main__":
    generate_convergence_chart()