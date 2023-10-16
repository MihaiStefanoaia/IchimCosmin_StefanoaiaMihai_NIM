import random
import math
import matplotlib.pyplot as plt

def randn(rate,size):
    ret = 0
    for _ in range(size):
        if random.random() < rate:
            ret += 1
    return ret

def mc_randn(r, size, iters):

    mc = [randn(r,size) for _ in range(iters)]
    mean = sum(mc) / len(mc)
    sd = math.sqrt(1 / (len(mc)-1) * sum([(x-mean) ** 2 for x in mc]))
    return sd

rd = [random.random() for _ in range(1000)]
rd_mean = sum(rd) / len(rd)
rd_sd = math.sqrt(1 / (len(rd)-1) * sum([(x-rd_mean) ** 2 for x in rd]))
print(rd_mean, rd_sd)


max_sd = []
p = 10
for i in range(p):
    iters = 100
    mc_iters = 10000
    size = 2 ** i

    sd_mc = [(r / iters, mc_randn(r / iters,size,mc_iters)) for r in range(iters+1)]
    max_sd.append(max([s[1] for s in sd_mc]))
    print(max_sd[-1])

    plt.plot([s[0] for s in sd_mc],[s[1] for s in sd_mc])
plt.show()
plt.plot([i for i in range(p)],max_sd)
plt.show()
print([max_sd[i+1]/max_sd[i] for i in range(p-1)])


