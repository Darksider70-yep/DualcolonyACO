import pandas as pd
import matplotlib.pyplot as plt
import os

def generate_ch150_chart():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(current_dir, "journal_baseline_ch150.csv")

    if not os.path.exists(csv_path):
        print(f"Error: Could not find data file at {csv_path}")
        return

    print("Loading ch150 scalability telemetry...")
    data = pd.read_csv(csv_path)

    try:
        plt.style.use('seaborn-v0_8-whitegrid')
    except OSError:
        plt.style.use('ggplot')
        
    plt.figure(figsize=(10, 6), dpi=300)

    # Plot Baselines
    plt.plot(data['Iteration'], data['Std_ACO'], color='#7f8c8d', linestyle='--', linewidth=2, label='Standard ACO (Mean)')
    plt.plot(data['Iteration'], data['MMAS'], color='#f39c12', linestyle=':', linewidth=2, label='MMAS Baseline (Mean)')

    # Plot Dual-Colony
    plt.plot(data['Iteration'], data['Dual_Colony'], color='#c0392b', linewidth=2.5, label='Dual-Colony ACO (10% Scouts)')

    # Academic Formatting
    plt.title('Algorithm Scalability on ch150 (150 Cities) with Dynamic Traffic', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Algorithm Iterations', fontsize=12)
    plt.ylabel('Mean Global Best Route Time (Cost)', fontsize=12)
    plt.legend(loc='upper right', frameon=True, shadow=True, fontsize=11)
    
    output_img = os.path.join(current_dir, "journal_chart_ch150.png")
    plt.savefig(output_img, bbox_inches='tight')
    
    print(f"Success! ch150 chart saved to: {output_img}")
    plt.show()

if __name__ == "__main__":
    generate_ch150_chart()