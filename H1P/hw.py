import matplotlib.pyplot as plt
import numpy as np

def grey(n):
    ret = [0x0,]
    for i in range(n):
        a = [(x | (1 << i)) for x in ret]
        a.reverse()
        ret = ret + a
    return ret


def population(x: int):
    return sum([1 if x & (i << i) != 0 else 0 for i in range(32)])


def hamming_dist(a: int, b: int):
    return population(a ^ b)

def first_better(arr, tgt):
    for a in arr:
        if a > tgt:
            return a
    return tgt



if __name__ == '__main__':
    vals = [x**3 - 60 * (x**2) + 900 * x + 100 for x in range(32)]
    for i in range(len(vals)):
        print(f"f({i}) = {vals[i]}")
    best_improvement = [max([vals[i ^ (1 << o)] for o in range(5)] + [vals[i]]) for i in range(32)]
    first_improvement = [first_better([vals[i ^ (1 << o)] for o in range(5)],vals[i]) for i in range(32)]
    translate_grey = grey(5)
    vals_grey = [vals[i] for i in translate_grey]
    plt.plot(vals)
    plt.plot(best_improvement)

    first_improvement = [first_better([first_improvement[i ^ (1 << o)] for o in range(5)],first_improvement[i]) for i in range(32)]
    plt.plot(first_improvement)

    plt.show()