from gpudrive import SimManager
from sim_utils.create import SimCreator

from time import perf_counter
import argparse
import csv
import yaml

def run_benchmark(sim: SimManager, config: dict):
    shape = sim.shape_tensor().to_torch()
    num_agents, num_roads = shape[0].flatten().tolist()
    num_envs = shape.shape[0]

    start = perf_counter()
    for i in range(0):
        sim.reset(i)
    time_to_reset = perf_counter() - start

    start = perf_counter()
    for i in range(91):
        sim.step()
    time_to_step = perf_counter() - start

    fps = (91 / time_to_step) * num_envs
    afps = fps * num_agents


    print(f"num_agents: {num_agents}, num_roads: {num_roads}, num_envs: {num_envs}, time_to_reset: {time_to_reset}, time_to_step: {time_to_step}, fps: {fps}, afps: {afps}, exec_mode: {config['sim_manager']['exec_mode']}, datasetInitOptions: {config['parameters']['datasetInitOptions']}")
    # check if benchmark_results.csv exists
    try:
        with open('benchmark_results.csv', mode='r') as file:
            pass
    except FileNotFoundError:
        with open('benchmark_results.csv', mode='w') as file:
            writer = csv.writer(file)
            writer.writerow(['num_agents', 'num_roads', 'num_envs', 'time_to_reset', 'time_to_step', 'fps', 'afps', 'exec_mode', 'datasetInitOptions'])

    with open('benchmark_results.csv', mode='a') as file:
        writer = csv.writer(file)
        writer.writerow([num_agents, num_roads, num_envs, time_to_reset, time_to_step, fps, afps, config['sim_manager']['exec_mode'], config['parameters']['datasetInitOptions']])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='GPUDrive Benchmarking Tool')
    parser.add_argument('--datasetPath', type=str, help='Path to the dataset', default='/home/aarav/gpudrive/config.yml')
    parser.add_argument('--numEnvs', type=int, help='Number of environments', default=1)
    args = parser.parse_args()
    with open(args.datasetPath, 'r') as file:
        config = yaml.safe_load(file)
    config['sim_manager']['num_worlds'] = args.numEnvs
    sim = SimCreator(config)
    run_benchmark(sim, config)