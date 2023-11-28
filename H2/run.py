import pso
from functions import funclib, tolerances
import numpy as np

import concurrent.futures
import subprocess
import torch
from pprint import pprint
import os
import json


def wrapper(args):
    return pso.pso(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12])


def run_logfile(args):
    return f'results_10/function_{args[0]}_adjust_{args[1]}_inertia_{args[2]}_self_bias_{args[3]}_global_bias_{args[4]}_exploration_{args[5]}.json'


def run_tests():
    re_spread = [-1, 0.5, 1, 3, 6]
    with concurrent.futures.ProcessPoolExecutor(max_workers=10) as executor:
        params = []
        for function in funclib.keys():
            for inertia in np.arange(0.7, 1, 0.05):
                for self_bias in np.arange(0.1, 0.5, 0.1):
                    for global_bias in np.arange(0.05, 0.3, 0.05):
                        for exploration_factor in np.arange(0, 0.05, 0.01):
                            for adjust_factor in re_spread:
                                dimensions = 10
                                params.append((function,adjust_factor,inertia,self_bias,global_bias,exploration_factor,30,250,5000,dimensions,tolerances[function][dimensions],torch.device('cuda'),False))
        runlist = []
        for run in params:
            if not os.path.isfile(run_logfile(run)):
                runlist.append(run)

        futures = {executor.submit(wrapper, run): run for run in runlist}
        for future in concurrent.futures.as_completed(futures):
            parameters = futures[future]
            log = future.result()
            with open(run_logfile(parameters), 'w') as outfile:
                json.dump(log, outfile, indent=2)
                print(f'{run_logfile(parameters)} finished')


        # for log, parameters in zip(executor.map(wrapper, runlist), runlist):
        #     with open(run_logfile(parameters), 'w') as outfile:
        #         json.dump(log, outfile, indent=2)
        #         print(f'{run_logfile(parameters)} finished')

def run_no_multi():
    re_spread = [-1, 0.5, 1, 3, 6]
    re_spread = [-1, 3]
    params = []
    for function in funclib.keys():
        for inertia in np.arange(0.5, 1, 0.05):
            for self_bias in np.arange(0.2, 0.7, 0.1):
                for global_bias in np.arange(0.1, 0.5, 0.1):
                    for exploration_factor in np.arange(0, 0.2, 0.05):
                        for adjust_factor in re_spread:
                            dimensions = 30
                            params.append((function,adjust_factor,inertia,self_bias,global_bias,exploration_factor,100,500,10000,dimensions,tolerances[function][dimensions],torch.device('cuda'),False))
    runlist = []
    for run in params:
        if not os.path.isfile(run_logfile(run)):
            runlist.append(run)
    for run in runlist:
        log = wrapper(run)
        with open(run_logfile(run), 'w') as outfile:
            json.dump(log, outfile, indent=2)
            print(f'{run_logfile(run)} finished')
        


if __name__ == '__main__':
    run_tests()
    # run_no_multi()