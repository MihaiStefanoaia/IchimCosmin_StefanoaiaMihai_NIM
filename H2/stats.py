import os
import matplotlib.pyplot as plt
import random
import json
import pandas as pd


# i hate that this had to be done
def extract_params(file_name):
    params = file_name.split('_')
    param_dict = {}
    param_dict['function'] = params[1]
    param_dict['adjust'] = float(params[3])
    param_dict['inertia'] = float(params[5])
    param_dict['self_bias'] = float(params[8])
    param_dict['global_bias'] = float(params[11])
    param_dict['exploration'] = float(os.path.splitext(params[13])[0])
    return param_dict


def process_results_folder(folder_path):
    paramFunctionProgress = {}
    for file_name in os.listdir(folder_path):
        file_path = os.path.join(folder_path, file_name)
        if file_name.endswith('.json'):
            with open(file_path, 'r') as json_file:
                data = json.load(json_file)
                params = extract_params(file_name)
                function_name = params['function']
                result_status = data['results']['status']
                last_value = data['results']['last_value']
                for param in params.keys():
                    if param != 'function':
                        paramValue = params.get(param)
                        if paramValue is not None:
                            if param not in paramFunctionProgress:
                                paramFunctionProgress[param] = {}
                            if function_name not in paramFunctionProgress[param]:
                                paramFunctionProgress[param][function_name] = {"success": [], "failure": []}
                            if result_status == "PASS":
                                paramFunctionProgress[param][function_name]["success"].append((paramValue, last_value))
                            else:
                                paramFunctionProgress[param][function_name]["failure"].append((paramValue, last_value))
    return paramFunctionProgress


def success_rates(folder_path):
    paramFunctionProgress = {}
    for file_name in os.listdir(folder_path):
        file_path = os.path.join(folder_path, file_name)
        if file_name.endswith('.json'):
            with open(file_path, 'r') as json_file:
                data = json.load(json_file)
                params = extract_params(file_name)
                function_name = params['function']
                result_status = data['results']['status']
                last_value = data['results']['last_value']
                for param, paramValue in params.items():
                    if param == 'function':
                        if paramValue not in paramFunctionProgress.keys():
                            paramFunctionProgress[paramValue] = {}
                        continue

                    if param not in paramFunctionProgress[function_name]:
                        paramFunctionProgress[function_name][param] = {}
                    if paramValue not in paramFunctionProgress[function_name][param]:
                        paramFunctionProgress[function_name][param][paramValue] = {'success': 0, 'failure': 0}

                    if result_status == "PASS":
                        paramFunctionProgress[function_name][param][paramValue]["success"] += 1
                    else:
                        paramFunctionProgress[function_name][param][paramValue]["failure"] += 1
    return paramFunctionProgress


def plot_rates(results_folder_path):
    rate_dict = success_rates(results_folder_path)
    for function, func_dict in rate_dict.items():
        for param, param_dict in func_dict.items():
            x = sorted(param_dict.keys())
            y = [(param_dict[k]['success'] / param_dict[k]['failure']) * 100 for k in x]
            print(x)
            print(y)
            plt.figure(figsize=(10, 6))
            plt.plot(x, y)
            plt.xlabel(f'Values of "{param}"')
            plt.ylabel('Success rate (in %)')
            plt.title(f'Success rates of function {function} accounting for variable "{param}"')
            plt.savefig(f'images/{function}_{param}_success_rate_plot.png')
            plt.show()


def plot_results(results_folder_path):
    for param, function_data in process_results_folder(results_folder_path).items():
        for function_name, progress_data in function_data.items():
            plt.figure(figsize=(10, 6))
            plt.suptitle(f"Function: {function_name}, Parameter: {param}")
            plt.subplot(1, 2, 1)
            plt.title("Success")
            for param_value, progress in progress_data["success"]:
                plt.plot(param_value, progress, 'go', label="Success")
            plt.xlabel(param)
            plt.ylabel("Function Value")
            plt.grid(True)
            plt.subplot(1, 2, 2)
            plt.title("Failure")
            for param_value, progress in progress_data["failure"]:
                plt.plot(param_value, progress, 'ro', label="Failure")
            plt.xlabel(param)
            plt.ylabel("Function Value")
            plt.grid(True)
            plt.savefig(f'{function_name}_{param}_plot.png')
            plt.show()

# paths = os.listdir('results')
#
# check = [paths[int(random.uniform(0,1)*len(paths))] for _ in range(100)]
#
# # jsons = [json.load(open('results/' + path)) for path in check]
# jsons = []
# for path in paths:
#     if path[0] == '.':
#         continue
#     with open('results/' + path) as f:
#         j = json.load(f)
#     if j['results']['status'] == 'PASS':
#         jsons.append(j)
#
# evolutions = [j['best_evolution'] for j in jsons]
#
# x = [i * 500 for i in range(101)]
#
# for e in evolutions:
#     plt.plot(x[:len(e)],e)
#
# plt.show()


if __name__ == '__main__':
    results_folder_path = 'results'
    from pprint import pprint
    pprint(success_rates(results_folder_path))
    plot_rates(results_folder_path)
    # plot_results(results_folder_path)