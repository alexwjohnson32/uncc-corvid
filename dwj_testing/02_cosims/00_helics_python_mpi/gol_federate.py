import helics as h
import numpy as np
import csv
import sys

def run_gol_chunk(worker_idx, total_workers, my_row_count, row_len, density, broker_ip, steps):
    fedinfo = h.helicsCreateFederateInfo()
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    # Use unique core name to avoid collisions across nodes
    h.helicsFederateInfoSetCoreInitString(fedinfo, f"--broker_address={broker_ip} --federates=1 --name=worker_core_{worker_idx}")
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_delta, 1.0)
    
    fed = h.helicsCreateValueFederate(f"worker_{worker_idx}", fedinfo)
    
    pub_top = h.helicsFederateRegisterGlobalTypePublication(fed, f"worker_{worker_idx}_top", "vector", "")
    pub_bot = h.helicsFederateRegisterGlobalTypePublication(fed, f"worker_{worker_idx}_bot", "vector", "")
    sub_n = h.helicsFederateRegisterSubscription(fed, f"worker_{(worker_idx - 1) % total_workers}_bot", "")
    sub_s = h.helicsFederateRegisterSubscription(fed, f"worker_{(worker_idx + 1) % total_workers}_top", "")

    np.random.seed(worker_idx)
    current_chunk = np.random.choice([0.0, 1.0], size=(my_row_count, row_len), p=[1-density, density])
    history = [[] for _ in range(my_row_count)]

    print(f"Worker {worker_idx} entering execution mode...", flush=True)
    h.helicsFederateEnterExecutingMode(fed)

    for t in range(steps):
        h.helicsPublicationPublishVector(pub_top, current_chunk[0, :].tolist())
        h.helicsPublicationPublishVector(pub_bot, current_chunk[-1, :].tolist())
        
        h.helicsFederateRequestTime(fed, float(t + 1))
        
        raw_n = h.helicsInputGetVector(sub_n)
        raw_s = h.helicsInputGetVector(sub_s)
        row_n = np.array(raw_n) if len(raw_n) == row_len else np.zeros(row_len)
        row_s = np.array(raw_s) if len(raw_s) == row_len else np.zeros(row_len)

        padded = np.vstack([row_n, current_chunk, row_s])
        new_chunk = np.zeros_like(current_chunk)

        for r in range(my_row_count):
            grid_r = r + 1
            for c in range(row_len):
                l, r_idx = (c - 1) % row_len, (c + 1) % row_len
                n_sum = (padded[grid_r-1, l] + padded[grid_r-1, c] + padded[grid_r-1, r_idx] +
                         padded[grid_r, l]   +                       padded[grid_r, r_idx] +
                         padded[grid_r+1, l] + padded[grid_r+1, c] + padded[grid_r+1, r_idx])
                
                if current_chunk[r, c] == 1:
                    new_chunk[r, c] = 1 if 2 <= n_sum <= 3 else 0
                else:
                    new_chunk[r, c] = 1 if n_sum == 3 else 0
        
        for r in range(my_row_count):
            history[r].append(current_chunk[r, :].tolist())
        current_chunk = new_chunk
        
        if worker_idx == 0 and t % 10 == 0:
            print(f"Worker 0 progress: Step {t}/{steps}", flush=True)

    h.helicsFederateFinalize(fed)
    for r in range(my_row_count):
        with open(f"temp_worker_{worker_idx}_row_{r}.csv", 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(history[r])
    h.helicsFederateFree(fed)

if __name__ == "__main__":
    run_gol_chunk(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), 
                  int(sys.argv[4]), float(sys.argv[5]), sys.argv[6], int(sys.argv[7]))