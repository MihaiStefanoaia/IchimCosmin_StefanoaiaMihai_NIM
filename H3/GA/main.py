import os
import pygad

adjacency_matrix = [[]]
number_of_edges = 0
number_of_nodes = 0


def process_graph_file(file_path):
    global adjacency_matrix
    global number_of_nodes
    global number_of_edges
    file_handler = open(file_path, 'r')
    lines = file_handler.readlines()
    for line in lines:
        if line[0] == 'p':
            adjacency_matrix = [[0 for col in range(int(line.split(" ")[2]))] for row in range(int(line.split(" ")[2]))]
            number_of_edges = int(line.split(" ")[3])
            number_of_nodes = int(line.split(" ")[2])
        if line[0] == 'e' and adjacency_matrix is not None:
            adjacency_matrix[int(line.split(" ")[1]) - 1][int(line.split(" ")[2]) - 1] = 1
    if number_of_edges != 0 and number_of_nodes != 0 and adjacency_matrix is not None:
        return number_of_nodes, number_of_edges, adjacency_matrix


def fitness_func(ga_instance, solution, solution_index):
    coloring_mistakes = 0
    colors = []
    for current_index, current_color in enumerate(solution):
        if not colors.__contains__(int(current_color)):
            colors.append(int(current_color))
        for i in range(number_of_nodes):
            if int(solution[i]) == int(current_color) and adjacency_matrix[current_index][i] == 1:
                coloring_mistakes += 1
    return 1 / (coloring_mistakes * number_of_nodes + len(colors))


def process_color_list(color_list):
    for i in reversed(range(number_of_nodes)):
        if not color_list.__contains__(i):
            for idx, element in enumerate(color_list):
                if element > i:
                    color_list[idx] -= 1
    return color_list


if __name__ == '__main__':
    for file in os.listdir('graphFiles'):
        if os.path.isfile(os.path.join('graphFiles', file)):
            process_graph_file(os.path.join('graphFiles', file))
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
                                   mutation_percent_genes=10)
            ga_instance.run()
            ga_instance.plot_fitness()
            solution, solution_fitness, solution_idx = ga_instance.best_solution()
            if ga_instance.best_solution_generation != -1:
                print(f"Best fitness value reached after {ga_instance.best_solution_generation} generations.")
            print(f"Parameters of the best solution : {process_color_list(solution)}")
            print(f"Fitness value of the best solution = {solution_fitness}")
            print(f"Index of the best solution : {solution_idx}")
            coloring_mistakes = 0
            for current_index, current_color in enumerate(solution):
                for i in range(number_of_nodes):
                    if int(solution[i]) == int(current_color) and adjacency_matrix[current_index][i] == 1:
                        coloring_mistakes += 1
            print(f"And the solution has {len(set(solution))} unique colors and {coloring_mistakes} coloring mistakes")
