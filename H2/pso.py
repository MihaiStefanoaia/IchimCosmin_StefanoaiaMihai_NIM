import torch
from tqdm import tqdm
from functions import funclib, tolerances
from argparse import ArgumentParser
import sys
import random
import time
from pprint import pprint
from line_profiler_pycharm import profile

def spread_adjust(z: torch.Tensor, factor: int = 3):
    z = z.mul_(2 * factor).sub_(factor)  # get the numbers from [0,1) -> [-factor,factor)
    return z.sigmoid_()  # apply sigmoid to the new numbers - this changes the distribution of the numbers
@profile
def pso(func_name, adjust_factor,
        inertia, self_bias, global_bias, exploration_factor,
        batches, batch_it, 
        population, dimensions, tolerance, 
        device, use_tqdm=False):
    if batch_it <= 0 or batches <= 0:
        batches = 1

    log = dict()
    log['parameters'] = {
        'function': func_name,
        'self_bias': self_bias,
        'global_bias': global_bias,
        'exploration_factor': exploration_factor,
        'population': population,
        'total_iterations': batches * batch_it if batch_it > 0 else -1,
        'tolerance': tolerance,
        'dimensions': dimensions,
        'logging_frequency': batch_it,
        'inertia': inertia,
        'adjust_factor': adjust_factor
    }
    log['results'] = {
        'status': None,  # pass or fail
        'last_value': None,
        'iterations_run': None
    }
    log['best_evolution'] = []

    if func_name not in funclib:
        raise RuntimeError(f'Function "{func_name}" not found in library')
    function, domain_lo, domain_hi = funclib[func_name]

    # initialize the population
    x = torch.rand(size=(population, dimensions)).to(device)
    if adjust_factor > 0:
        x = spread_adjust(x, adjust_factor)
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
    global_best = x[most_fit_i].expand(population, -1)
    global_best_val = pb_values[most_fit_i]
    log['best_evolution'].append(global_best_val.item())

    # if batch_it is above 0: run the algorithm {batch} times for the specified number of iterations
    # if batch_it is below 0: run until it reaches tolerance
    runs = 0
    while (batch_it <= 0 and global_best_val > tolerance) or runs == 0:
        for batch in range(batches):
            it = batch_it if batch_it > 0 else 1000
            if use_tqdm:
                it = tqdm(range(it))
            else:
                it = range(it)
            for i in it:

                rand_personal_w = torch.rand_like(velocity).to(device)
                rand_global_w = torch.rand_like(velocity).to(device)
                rand_explore = torch.rand_like(velocity).to(device).mul_(abs(domain_hi - domain_lo)).add_(min(domain_hi, domain_lo))

                # rand_personal_w = torch.zeros_like(velocity,device=device).uniform_()
                # rand_global_w = torch.zeros_like(velocity,device=device).uniform_()
                # rand_explore = torch.zeros_like(velocity,device=device).uniform_().mul_(abs(domain_hi - domain_lo)).add_(min(domain_hi, domain_lo))


                # update the velocity and the particle positions; calculate the function
                velocity = (inertia * velocity +
                            self_bias * rand_personal_w * (personal_best - x) +
                            global_bias * rand_global_w * (global_best - x) +
                            exploration_factor * rand_explore)
                x.add_(velocity)
                y = function(x)
                
                mask = y < pb_values
                pb_values = mask * y + torch.logical_not(mask) * pb_values

                mask = mask.view(mask.shape[0], 1)
                mask = mask.expand(population, dimensions)
                personal_best = mask * x + torch.logical_not(mask) * personal_best

                most_fit_i = torch.argmin(pb_values)
                # global_best_val = pb_values[most_fit_i]
                # global_best = personal_best[most_fit_i].expand(population, -1)

                onehot_mask = torch.nn.functional.one_hot(most_fit_i,num_classes=population).float()
                global_best_val = pb_values.dot(onehot_mask)
                onehot_mask = onehot_mask.view(onehot_mask.shape[0], 1)
                onehot_mask = onehot_mask.expand(population,dimensions)
                global_best = (personal_best * onehot_mask).sum(dim=0).expand(population, -1)


            log['best_evolution'].append(global_best_val.item())

            if global_best_val.item() < tolerance:
                log['results']['iterations_run'] = runs * batches * batch_it + (batch + 1) * batch_it
                break

        runs += 1

    log['results']['status'] = 'PASS' if global_best_val.item() < tolerance else 'FAIL'
    log['results']['last_value'] = global_best_val.item()
    if log['results']['iterations_run'] is None:
        log['results']['iterations_run'] = runs * batches * batch_it

    return log
    

if __name__ == '__main__':
    for key in funclib.keys():
        a = pso(key, 0,
                0.9, 0.15, 0.15, 0,
                15, 500,
                100000, 30, tolerances[key][30],
                torch.device('cuda'), True)
        pprint(a)
        break
