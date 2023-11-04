import torch
from tqdm import tqdm
from functions import funclib
from argparse import ArgumentParser
import sys
import random
import time

def pso(function, domain_lo, domain_hi, inertia, self_bias, global_bias, max_it, population, dimensions, tolerance, device):
    # initialize the population
    x = torch.rand(size=(population, dimensions)).to(device)
    x = x.mul_(abs(domain_hi - domain_lo))
    x = x.add_(min(domain_hi, domain_lo))

    # initialize the velocities
    velocity = torch.rand(size=(population, dimensions)).to(device)
    velocity = velocity.mul_(2 * abs(domain_hi - domain_lo))
    velocity = velocity.sub_(abs(domain_hi - domain_lo))

    # initialize the personal bests of each member
    personal_best = x
    pb_values = function(x)

    # and the global best
    most_fit_i = torch.argmin(pb_values).to(device)
    global_best = x[most_fit_i].expand(population,-1)
    global_best_val = pb_values[most_fit_i]

    # run the algorithm once for the specified number of iterations if max_it is above 0, or run until it reaches tolerance if it is below 0
    ran_once = True
    # print(pb_values)
    # print(personal_best)
    while (max_it <= 0 and global_best_val > tolerance) or ran_once:
        it = max_it if max_it > 0 else 10000
        it = tqdm(range(it))
        for i in it:
            rp = torch.rand_like(velocity).to(device)
            rg = torch.rand_like(velocity).to(device)
            velocity = inertia * velocity + self_bias * rp * (personal_best - x) + global_bias * rg * (global_best - x)
            x.add_(velocity)
            y = function(x)
            
            mask = pb_values > y
            n_mask = torch.logical_not(mask)
            pb_values = mask * y + n_mask * pb_values
            mask = mask.view(mask.shape[0],1)
            # print(pb_values)
            mask = mask.expand(population,dimensions)
            n_mask = torch.logical_not(mask)
            personal_best = mask * x + n_mask * personal_best
            # print(personal_best)
            # print(mask)
            # exit(0)

            most_fit_i = torch.argmin(pb_values)
            global_best_val = pb_values[most_fit_i]
            global_best = personal_best[most_fit_i].expand(population,-1)
            it.set_postfix_str(f"best value = {global_best_val.item()}")
            # time.sleep(0.01)            


        ran_once = False
    return x, global_best, global_best_val
    



if __name__ == '__main__':

    func, lo, hi = funclib['rastrigin']
    a = pso(func,lo,hi,0.9,0.15,0.15,-1000,100,10,0.1,torch.device('cuda'))
    print(a[1])
    pass
