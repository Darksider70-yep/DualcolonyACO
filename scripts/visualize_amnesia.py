import pandas as pd
import matplotlib.pyplot as plt
import os

def generate_amnesia_chart():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(current_dir, "journal_amnesia.csv")

    if not os.path.exists(csv_path):
        print(f"Error: Could not find data file at {csv_path}")
        return

    print("Loading Amnesia Test telemetry...")
    data = pd.read_csv(csv_path)

    try:
        plt.style.use('seaborn-v0_8-whitegrid')
    except OSError:
        plt.style.use('ggplot')
        
    plt.figure(figsize=(10, 6), dpi=300)

    # Plot Standard ACO (Mean + Confidence Interval)
    plt.plot(data['Iteration'], data['Std_Mean'], color='#7f8c8d', linestyle='--', linewidth=2, label='Standard ACO (Mean)')
    plt.fill_between(data['Iteration'], 
                     data['Std_Mean'] - data['Std_Dev'], 
                     data['Std_Mean'] + data['Std_Dev'], 
                     color='#7f8c8d', alpha=0.2)

    # Plot Dual-Colony ACO (Mean + Confidence Interval)
    # Using a vibrant, undeniable red to highlight the breakout
    plt.plot(data['Iteration'], data['Dual_Mean'], color='#e74c3c', linewidth=2.5, label='Dual-Colony ACO (Mean)')
    plt.fill_between(data['Iteration'], 
                     data['Dual_Mean'] - data['Dual_Dev'], 
                     data['Dual_Mean'] + data['Dual_Dev'], 
                     color='#e74c3c', alpha=0.2)

    # Academic Formatting
    plt.title('Algorithm Robustness: Extreme Traffic & High Evaporation (kroA100)', fontsize=14, fontweight='bold', pad=15)
    plt.xlabel('Algorithm Iterations', fontsize=12)
    plt.ylabel('Global Best Route Time (Cost)', fontsize=12)
    plt.legend(loc='upper right', frameon=True, shadow=True, fontsize=11)
    
    output_img = os.path.join(current_dir, "journal_chart_amnesia.png")
    plt.savefig(output_img, bbox_inches='tight')
    
    print(f"Success! Final statistical chart saved to: {output_img}")
    plt.show()

if __name__ == "__main__":
    generate_amnesia_chart()