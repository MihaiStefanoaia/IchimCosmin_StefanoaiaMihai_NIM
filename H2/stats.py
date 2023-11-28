import os
import matplotlib.pyplot as plt
import random
import json

paths = os.listdir('results')

check = [paths[int(random.uniform(0,1)*len(paths))] for _ in range(100)]

# jsons = [json.load(open('results/' + path)) for path in check]zx
jsons = []
for path in paths:
    if path[0] == '.':
        continue
    with open('results/' + path) as f:
        j = json.load(f)
    if j['results']['status'] == 'PASS':
        jsons.append(j)

evolutions = [j['best_evolution'] for j in jsons]

x = [i * 500 for i in range(101)]

for e in evolutions:
    plt.plot(x[:len(e)],e)

plt.show()