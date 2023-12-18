import json
import os
import pygad

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
    mistakes = sum([1 if solution[node_a] == solution[node_b] else 0 for node_a, node_b in adjacency_list])
    return mistakes


def fitness_func(ga_instance, solution, solution_index):
    colors = list(set(solution))
    return 1 / ((coloring_mistakes(solution) * (number_of_nodes ** 2)) + len(colors) * number_of_nodes)


def process_color_list(color_list):
    for i in reversed(range(number_of_nodes)):
        if not color_list.__contains__(i):
            for idx, element in enumerate(color_list):
                if element > i:
                    color_list[idx] -= 1
    return color_list


def minimizer(ga_instance, offspring_mutation):
    for node in offspring_mutation:
        for i in range(number_of_nodes):
            colors = {node[j] for j in full_adjacency_list[i]}  # set of colors of the neighbours
            for c in range(number_of_nodes):
                if c not in colors:
                    node[i] = c
                    break


def main(file_path):
    process_graph_file(file_path)
    ga_instance = pygad.GA(num_generations=1000,
                            num_parents_mating=4,
                            fitness_func=fitness_func,
                            sol_per_pop=100,
                            gene_type=int,
                            num_genes=number_of_nodes,
                            init_range_low=0,
                            init_range_high=number_of_nodes - 1,
                            parent_selection_type="sss",
                            keep_parents=1,
                            crossover_type="single_point",
                            mutation_type="random",
                            mutation_percent_genes=10,
                            on_mutation=minimizer,
                            # parallel_processing=['', 6],
                            )
    ga_instance.run()
    # ga_instance.plot_fitness()
    solution, solution_fitness, solution_idx = ga_instance.best_solution()
    # if ga_instance.best_solution_generation != -1:
    #     print(f"Best fitness value reached after {ga_instance.best_solution_generation} generations.")
    # print(f"Parameters of the best solution : {process_color_list(solution)}")
    # print(f"Fitness value of the best solution = {solution_fitness}")
    # print(f"Index of the best solution : {solution_idx}")
    # print(f"And the solution has {len(set(solution))} unique colors and {coloring_mistakes(solution)} coloring mistakes")
    data = {
        'file_name': str(file_path),
        'number_of_colors': len(set(solution)),
        'colors': [float(a) for a in process_color_list(solution)]
    }
    return data