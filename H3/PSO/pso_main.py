import pyswarms as ps
import numpy as np
from pyswarms.utils.functions import single_obj as fx
import json

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
        if line[0] == 'e' and adjacency_list is not None:
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
    mistakes = 0
    for i in range(len(solution)):
        for node_a, node_b in adjacency_list:
            mistakes += 1 if round(solution[i][node_a]) == round(solution[i][node_b]) else 0
    return mistakes


def process_color_list(color_list):
    for i in range(len(color_list)):
        color_list[i] = round(color_list[i])
    for i in reversed(range(number_of_nodes)):
        if not color_list.__contains__(i):
            for idx, element in enumerate(color_list):
                if element > i:
                    color_list[idx] -= 1
    return color_list


def main(graph_file_path):
    process_graph_file(graph_file_path)
    lower_bound = np.zeros(number_of_nodes)
    upper_bound = np.full(number_of_nodes, number_of_nodes - 1)
    bounds = (lower_bound, upper_bound)
    options = {'c1': 0.5, 'c2': 0.3, 'w': 0.9, 'bounds': 'discrete'}

    def graph_coloring_objective_function(solution):
        mistakes = coloring_mistakes(solution)
        color_count = 0
        for i in solution:
            temp_color_set = set()
            for j in i:
                if not temp_color_set.__contains__(round(j)):
                    temp_color_set.add(round(j))
            color_count += len(temp_color_set)
        return 1 / (mistakes ** 3 + color_count ** 2)
    optimizer = ps.single.GlobalBestPSO(n_particles=10000, dimensions=number_of_nodes, bounds=bounds, options=options)
    outcome = optimizer.optimize(graph_coloring_objective_function, iters=100)
    # print(outcome[0])
    # print(outcome[1])
    # print(process_color_list(outcome[1]))
    # print(len(set(process_color_list(outcome[1]))))
    data = {
        'file_name': graph_file_path,
        'number_of_colors': len(set(process_color_list(outcome[1]))),
        'colors': list(process_color_list(outcome[1]))
    }
    return data


if __name__ == '__main__':
    main('..\\graphFiles\\myciel3.col')
