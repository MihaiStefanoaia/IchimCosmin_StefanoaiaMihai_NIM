import torch
from tqdm import tqdm
from functions import funclib
from argparse import ArgumentParser
import sys
import random
import time

def spread_adjust(z : torch.Tensor, factor: int = 3):
    z.mul_(2 * factor).sub_(factor) # get the numbers from [0,1) -> [-factor,factor)
    return z.sigmoid_()  # apply sigmoid to the new numbers - this changes the distribuition of the numbers

def pso(function, domain_lo, domain_hi,
        inertia, self_bias, global_bias, 
        batches, batch_it, 
        population, dimensions, tolerance, 
        device, logging_freq, use_tqdm = False):
    if batch_it < 0 or batches <= 0:
        batches = 1
        
    # initialize the population
    x = torch.rand(size=(population, dimensions)).to(device)
    x = spread_adjust(x)
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

    # run the algorithm  for the specified number of iterations if batch_it is above 0, or run until it reaches tolerance if it is below 0
    ran_once = True
    # print(pb_values)
    # print(personal_best)
    while (batch_it <= 0 and global_best_val > tolerance) or ran_once:
        for _ in range(batches):
            it = batch_it if batch_it > 0 else 10000
            if use_tqdm:
                it = tqdm(range(it))
            else:
                it = range(it)
            for i in it:
                rp = torch.rand_like(velocity).to(device)
                rg = torch.rand_like(velocity).to(device)
                velocity = inertia * velocity + self_bias * rp * (personal_best - x) + global_bias * rg * (global_best - x)
                x.add_(velocity)
                y = function(x)
                
                mask = y < pb_values
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
                if use_tqdm:
                    it.set_postfix_str(f"best value = {global_best_val.item()}")
                # time.sleep(0.01)            
            if global_best_val < tolerance:
                break

        ran_once = False
    return x, global_best, global_best_val
    



if __name__ == '__main__':

    func, lo, hi = funclib['rastrigin']
    a = pso(func,lo,hi,
            0.9,0.15,0.15,
            10,500,
            10000,10,0.1,
            torch.device('cuda'),0, True)
    print(a[2])
    pass
