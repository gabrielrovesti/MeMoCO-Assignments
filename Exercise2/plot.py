import matplotlib.pyplot as plt
import numpy as np
import os

def analyze_performance(data_dir):
    # Categories for analysis
    categories = ['small', 'medium', 'large']
    performance_data = {cat: [] for cat in categories}
    
    # Collect data from all instances
    for category in categories:
        cat_dir = os.path.join(data_dir, category)
        if not os.path.exists(cat_dir):
            continue
            
        for file in os.listdir(cat_dir):
            if file.endswith('.dat'):
                filepath = os.path.join(cat_dir, file)
                with open(filepath, 'r') as f:
                    # Extract instance size and performance metrics
                    lines = f.readlines()
                    size = int(lines[0])  # First line contains number of nodes
                    # Additional metrics can be extracted from metadata
                    performance_data[category].append({
                        'size': size,
                        'filename': file
                    })

    # Create performance visualization
    plt.figure(figsize=(15, 10))
    
    # Size distribution boxplot
    plt.subplot(2, 2, 1)
    sizes = [data['size'] for cat in categories for data in performance_data[cat]]
    plt.boxplot([data['size'] for data in performance_data[cat]] for cat in categories)
    plt.title('Instance Size Distribution by Category')
    plt.ylabel('Number of Nodes')
    plt.xticks(range(1, len(categories) + 1), categories)

    # Additional plots can include:
    # - Solution quality vs problem size
    # - Computation time analysis
    # - Parameter sensitivity analysis
    
    plt.tight_layout()
    plt.savefig('performance_analysis.png')
    plt.close()

    # Generate summary statistics
    print("\nPerformance Analysis Summary:")
    for category in categories:
        sizes = [data['size'] for data in performance_data[category]]
        if sizes:
            print(f"\n{category.upper()} instances:")
            print(f"Number of instances: {len(sizes)}")
            print(f"Average size: {np.mean(sizes):.2f} nodes")
            print(f"Size range: {min(sizes)} - {max(sizes)} nodes")

if __name__ == "__main__":
    data_dir = "data"  # Path to your data directory
    analyze_performance(data_dir)