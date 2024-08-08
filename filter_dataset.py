# Contributed by Joseph Suarez, PufferAI
# Download the dataset (I used the mini one) and place the folder
# in the top level of the repo: formatted_json_v2_no_tl_train
# The default output files are INCLUDED in the repo without you
# having to download them. The biggest file has 60 agents. There are
# 5 files with 48-60 agents.

import os
import numpy as np
import json

min_num_vehicles = 48
max_num_vehicles = 64
good_files = []

biggest = -1
biggest_file = ''

# List files here
for f in os.listdir('formatted_json_v2_no_tl_train'):
    if f.endswith('.json'):
        print(f'formatted_json_v2_no_tl_train/{f}')

    with open(f'formatted_json_v2_no_tl_train/{f}', 'r') as f:
        data = f.read()
        data = json.loads(data)
        num_vehicles = 0
        if 'objects' not in data:
            continue

        for obj in data['objects']:
            if obj['type'] != 'vehicle':
                continue

            start = obj['position'][0]
            goal = obj['goalPosition']
            # Compute l2
            dist = np.sqrt((start['x'] - goal['x'])**2 + (start['y'] - goal['y'])**2)
            if dist < 0.2:
                continue

            if obj['valid'][0] == False:
                continue

            num_vehicles += 1

        if num_vehicles > biggest:
            biggest = num_vehicles
            biggest_file = f.name

        print(f'num_vehicles: {num_vehicles}')
        if num_vehicles < min_num_vehicles or num_vehicles > max_num_vehicles:
            continue

        good_files.append(f.name)

import shutil
shutil.rmtree('filtered_data', ignore_errors=True)
os.mkdir('filtered_data')
for file in good_files:
    shutil.copy(file, 'filtered_data')

import shutil
shutil.rmtree('biggest_file', ignore_errors=True)
os.mkdir('biggest_file')
shutil.copy(biggest_file, 'biggest_file')
print(f'biggest: {biggest}')
print(f'biggest_file: {biggest_file}')
