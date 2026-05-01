import pandas as pd
import matplotlib.pyplot as plt
import os

def generate_sweep_chart():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(current_dir, "journal_amnesia.csv")

    if not os.path.exists(csv_path):
        print(f"Error: Could not find data file at {csv_path}")
        return

    print("Loading sensitivity sweep telemetry...")
    data = pd.read_csv(csv_path)

    try:
        plt.style.use('seaborn-v0_8-whitegrid')
    except OSError:
        plt.style.use('ggplot')
        
    plt.figure(figsize=(10, 6), dpi=300)

    # Plot the Sweep with a color gradient (Gray to Dark Red)
    plt.plot(data['Iteration'], data['Ratio_0'], color='#bdc3c7', linestyle='--', linewidth=2, label='0% Scouts (Standard)')
    plt.plot(data['Iteration'], data['Ratio_10'], color='#f5b7b1', linewidth=1.5, label='10% Scouts')
    plt.plot(data['Iteration'], data['Ratio_20'], color='#e74c3c', linewidth=2, label='20% Scouts')
    plt.plot(data['Iteration'], data['Ratio_30'], color='#c0392b', linewidth=2.5, label='30% Scouts')
    plt.plot(data['Iteration'], data['Ratio_40'], color='#78281f', linewidth=3, label='40% Scouts')

    # Academic Formatting
    plt.title('Parameter Sensitivity: Scout Ratio under High Traffic Volatility (kroA100)', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Algorithm Iterations', fontsize=12)
    plt.ylabel('Mean Global Best Time (Cost)', fontsize=12)
    plt.legend(loc='upper right', frameon=True, shadow=True, fontsize=11)
    
    output_img = os.path.join(current_dir, "journal_chart_sweep.png")
    plt.savefig(output_img, bbox_inches='tight')
    
    print(f"Success! Parameter sensitivity chart saved to: {output_img}")
    plt.show()

if __name__ == "__main__":
    generate_sweep_chart()