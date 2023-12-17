import os
import random

adjacency_list = []
full_adjacency_list = {}
number_of_edges = 0
number_of_nodes = 0


class VertexColoring:
    def __init__(self, graph):
        self.graph = graph
        self.num_vertices = len(graph)
        self.Nominate = []
        self.colors = [0] * self.num_vertices
        self.colored = [None] * self.num_vertices
        self.VW = [0] * self.num_vertices
        self.vpurity = [0] * self.num_vertices

    def calculate_degree_of_purity(self, vertex):
        colored_neighbors = sum(1 for neighbor in self.graph[vertex] if self.colored[neighbor] is not None)
        total_neighbors = len(set(self.graph[vertex]))
        return colored_neighbors / total_neighbors if total_neighbors > 0 else 0

    def compute_node_weight(self, vertex):
        return len(set(self.graph[vertex]))

    def calculate_total_votes(self, vertex):
        return sum(self.VW[neighbor] for neighbor in self.graph[vertex])

    def is_superior(self, node_a, node_b):
        purity_a = self.vpurity[node_a]
        purity_b = self.vpurity[node_b]
        neighbors_a = self.compute_node_weight(node_a)
        neighbors_b = self.compute_node_weight(node_b)
        return purity_a > purity_b and neighbors_a > neighbors_b

    def voting_phase(self):
        for vertex in range(self.num_vertices):
            if self.vpurity[vertex] <= 0:
                continue
            if self.colored[vertex] is None:
                currentMaxPurity = self.vpurity[vertex]
                currentMaxCount = len(set(full_adjacency_list[vertex]))
                superior_neighbors = [vertex]
                # print('MAKING CHECK NB')
                check_nb = [n for n in full_adjacency_list[vertex] if self.colored[n] is None and (self.vpurity[n] >= self.vpurity[vertex] and self.compute_node_weight(n) >= self.compute_node_weight(vertex))]

                # print(f'INITIAL check_nb: {check_nb}')
                check_nb.sort(key=lambda v: self.vpurity[v], reverse=True)
                check_nb = [n for n in check_nb if self.vpurity[n] == self.vpurity[check_nb[0]]]
                # print(f'check_nb after first filtering: {check_nb}')
                check_nb.sort(key=lambda v: self.compute_node_weight(v) , reverse=True)
                check_nb = [n for n in check_nb if self.compute_node_weight(n) == self.compute_node_weight(check_nb[0])]
                # print(f'check_nb after second filtering: {check_nb}')
                
                for nb in check_nb:
                    self.VW[nb] = self.compute_node_weight(vertex)
                
                if len(check_nb) == 0:
                    self.Nominate.append(vertex)

                # for neighbour in full_adjacency_list[vertex]:
                #     if self.vpurity[neighbour] >= self.vpurity[vertex] or len(set(full_adjacency_list[neighbour])) >= len(
                #             set(full_adjacency_list[vertex])):
                #         if currentMaxPurity < self.vpurity[neighbour] and currentMaxCount < len(
                #                 set(full_adjacency_list[neighbour])):
                #             currentMaxCount = len(set(full_adjacency_list[neighbour]))
                #             currentMaxPurity = self.vpurity[neighbour]
                #             superior_neighbors = [neighbour]
                #         if currentMaxPurity == self.vpurity[neighbour] and currentMaxCount == len(
                #                 set(full_adjacency_list[neighbour])) and currentMaxPurity != self.vpurity[vertex]:
                #             superior_neighbors.append(neighbour)
                # if superior_neighbors:
                #     for i in superior_neighbors:
                #         self.VW[i] += self.compute_node_weight(vertex)
                # else:
                #     self.Nominate.append(vertex)

    def coloring_phase(self):
        if self.Nominate:
            for vertex in self.Nominate:
                self.colors[vertex] = self.color_vertex(vertex)
        else:
            max_votes = max(self.VW)
            candidates = [i for i, votes in enumerate(self.VW) if votes == max_votes]
            selected_vertex = candidates[random.randint(0, len(candidates)-1)]
            self.colors[selected_vertex] = self.color_vertex(selected_vertex)
        for v in range(number_of_nodes - 1):
            if self.vpurity[v] == 0:
                self.colors[v] = self.color_vertex(v)


    def update_phase(self):
        for vertex in range(self.num_vertices):
            for neighbor in self.graph[vertex]:
                self.vpurity[neighbor] = self.calculate_degree_of_purity(neighbor)

    def color_vertex(self, vertex):
        for color in range(number_of_nodes-1):
            if color not in [self.colors[nb] for nb in full_adjacency_list[vertex]]:
                self.colored[vertex] = True
                return color

    def coloring_mistakes(self):
        global adjacency_list
        mistakes = 0
        for i in range(len(self.colors)):
            for node_a, node_b in adjacency_list:
                mistakes += 1 if self.colors[node_a] == self.colors[node_b] else 0
        return mistakes

    def minimization_pass(self):
        order = list(range(number_of_nodes))
        random.shuffle(order)
        for i in order:
            colors = {self.colors[j] for j in full_adjacency_list[i]}  # set of colors of the neighbours
            for c in range(number_of_nodes):
                if c not in colors:
                    self.colors[i] = c
                    break

    def run_algorithm(self, iterations):
        while None in self.colored:
            self.voting_phase()
            self.coloring_phase()
            self.update_phase()
            self.minimization_pass() 




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


if __name__ == '__main__':
    for file in os.listdir('graphFiles'):
        print(file)
        process_graph_file(os.path.join('graphFiles', file))
        instanta_algoritm = VertexColoring(full_adjacency_list)
        instanta_algoritm.run_algorithm(100)
        print(instanta_algoritm.colors)
        print(len(set(instanta_algoritm.colors)))