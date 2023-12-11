import os
import random
from copy import copy
from typing import Optional

from tqdm import tqdm

adjacency_list = []
full_adjacency_list = {}
number_of_edges = 0
number_of_nodes = 0


def process_graph_file(file_path):
    global adjacency_list
    global number_of_nodes
    global number_of_edges
    file_handler = open(file_path, 'r')
    lines = file_handler.readlines()
    for line in lines:
        if line[0] == 'p':
            number_of_edges = int(line.split(" ")[3])
            number_of_nodes = int(line.split(" ")[2])
        if line[0] == 'e':
            adjacency_list.append((int(line.split(" ")[1]) - 1, int(line.split(" ")[2]) - 1))
            if int(line.split(" ")[1]) - 1 not in full_adjacency_list.keys():
                full_adjacency_list[int(line.split(" ")[1]) - 1] = []
            full_adjacency_list[int(line.split(" ")[1]) - 1].append(int(line.split(" ")[2]) - 1)

            if int(line.split(" ")[2]) - 1 not in full_adjacency_list.keys():
                full_adjacency_list[int(line.split(" ")[2]) - 1] = []
            full_adjacency_list[int(line.split(" ")[2]) - 1].append(int(line.split(" ")[1]) - 1)
    if number_of_edges != 0 and number_of_nodes != 0:
        return number_of_nodes, number_of_edges, adjacency_list


def coloring_mistakes(solution):
    global adjacency_list
    mistakes = sum([1 if solution[node_a] == solution[node_b] else 0 for node_a, node_b in adjacency_list])
    return mistakes


def correction_pass(state: list, initial_queue: Optional[list]):
    global number_of_nodes
    queue = [random.randint(0, number_of_nodes - 1)] if initial_queue is None else initial_queue
    visited = []
    while len(queue) > 0:
        idx = queue.pop(0)
        visited.append(idx)
        adj = [n for n in full_adjacency_list[idx] if n not in queue and n not in visited]
        random.shuffle(adj)
        queue.extend(adj)
        adj_colors = {state[n] for n in full_adjacency_list[idx]}
        while state[idx] in adj_colors:
            state[idx] += 1


def minimization_pass(state: list):
    order = list(range(number_of_nodes))
    random.shuffle(order)
    for i in order:
        colors = {state[j] for j in full_adjacency_list[i]}  # set of colors of the neighbours
        for c in range(number_of_nodes):
            if c not in colors:
                state[i] = c
                break


def mutation_pass(state: list, mutation_rate: float):
    ret = []
    for i in range(len(state)):
        if random.uniform(0, 1) < mutation_rate:
            state[i] = random.randint(0, len(state) - 1)
            ret.append(i)
    return ret


def graph_color_nca(iterations: int, mutation_rate: float):
    global number_of_nodes
    state = [random.randint(0, number_of_nodes - 1) for _ in range(number_of_nodes)]
    best_state = None
    it = tqdm(range(iterations))
    for i in it:
        queue = mutation_pass(state, mutation_rate)
        correction_pass(state, queue)
        minimization_pass(state)
        if best_state is None or len(set(best_state)) > len(set(state)):
            best_state = copy(state)
        it.set_postfix_str(f'best state: {len(set(best_state))} colors, current state: {len(set(state))} colors')
    return best_state


if __name__ == '__main__':
    for file in os.listdir('../graphFiles'):
        process_graph_file(os.path.join('../graphFiles', file))
        solution = graph_color_nca(500, 15 / number_of_nodes)
        print(f"The solution has {len(set(solution))} unique colors and {coloring_mistakes(solution)} coloring mistakes")

