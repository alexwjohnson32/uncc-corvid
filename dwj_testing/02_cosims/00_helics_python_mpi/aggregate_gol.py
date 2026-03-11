import pandas as pd
import numpy as np
import glob
import re
import os

def extract_indices(filename):
    """Extracts (worker_idx, row_idx) from 'temp_worker_X_row_Y.csv'"""
    nums = re.findall(r'\d+', filename)
    return int(nums[0]), int(nums[1])

def aggregate():
    # 1. Find and Sort all row files
    files = glob.glob("temp_worker_*_row_*.csv")
    if not files:
        print("No data found! Verify workers are writing 'temp_worker_X_row_Y.csv' files.")
        return

    # Sort naturally: Worker 0 Row 0, Worker 0 Row 1... Worker 1 Row 0...
    files.sort(key=extract_indices)
    
    all_rows_history = []
    for f in files:
        # row_data shape: (Timesteps, RowLen)
        row_data = pd.read_csv(f, header=None).values
        all_rows_history.append(row_data)

    # 2. Reshape into (TotalRows, Timesteps, RowLen)
    history_cube = np.array(all_rows_history)
    
    # 3. Transpose to (Timesteps, TotalRows, RowLen)
    # This puts the "Generation" as the primary index
    history_cube = np.transpose(history_cube, (1, 0, 2))
    
    num_gens = history_cube.shape[0]

    # 4. Write to file with line breaks
    with open("full_history.txt", "w") as f:
        for g in range(num_gens):
            f.write(f"--- GENERATION {g} ---\n")
            
            # Convert the 2D slice for this generation into a string
            gen_slice = history_cube[g]
            for row in gen_slice:
                # Convert [0.0, 1.0...] to "0,1,0..."
                line = ",".join([str(int(val)) for val in row])
                f.write(line + "\n")
            
            # Add the requested line break between generations
            f.write("\n")

    print(f"Success! Processed {num_gens} generations.")
    print(f"Output saved to 'full_history.txt'")

    # VISUAL PREVIEW of the first and last gen
    print("\nGeneration 0 Preview:")
    print_preview(history_cube[0])
    print(f"\nGeneration {num_gens-1} Preview:")
    print_preview(history_cube[-1])

def print_preview(grid_slice):
    for r in range(min(10, grid_slice.shape[0])):
        line = "".join(["#" if val > 0 else "." for val in grid_slice[r, :20]])
        print(line)


if __name__ == "__main__":
    aggregate()
    os.system('rm -rf temp_worker*.csv')