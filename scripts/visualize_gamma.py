import pandas as pd
import matplotlib.pyplot as plt

def generate_gamma_chart():
    # Load your ablation data
    data = {
        'Gamma': [0.5, 1.0, 2.0, 3.0, 5.0],
        'Mean_Cost': [36905.74, 36779.52, 36935.23, 36525.09, 36624.95]
    }
    df = pd.DataFrame(data)

    try:
        plt.style.use('seaborn-v0_8-whitegrid')
    except OSError:
        plt.style.use('ggplot')

    plt.figure(figsize=(8, 5), dpi=300)

    # Plot the line with markers
    plt.plot(df['Gamma'], df['Mean_Cost'], marker='o', markersize=9, 
             color='#2c3e50', linewidth=2.5, linestyle='-', label='Mean Route Cost')

    # Highlight the newly discovered optimal point (Gamma = 3.0)
    plt.plot(3.0, 36525.09, marker='*', markersize=18, color='#e74c3c', 
             label=r'Optimal Avoidance ($\gamma=3.0$)')

    # Academic Formatting
    plt.title(r'Ablation Study: Impact of Scout Avoidance Factor ($\gamma$)', fontsize=13, fontweight='bold', pad=15)
    plt.xlabel(r'Avoidance Factor ($\gamma$)', fontsize=12)
    plt.ylabel('Mean Global Best Route Time (Cost)', fontsize=12)
    plt.xticks(df['Gamma'])
    plt.legend(loc='upper right', frameon=True, shadow=True, fontsize=11)
    
    output_img = "journal_chart_gamma.png"
    plt.savefig(output_img, bbox_inches='tight')
    print(f"Success! Gamma ablation chart saved to: {output_img}")

if __name__ == "__main__":
    generate_gamma_chart()