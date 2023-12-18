import concurrent.futures
import json
import os
import GA.ga_main
import PSO.pso_main


def logfile_path(funcname, filename, iteration):
    return f"outputFiles\\{funcname}_{str(filename.split('.')[0])}_{iteration}.json"


if __name__ == '__main__':
    with concurrent.futures.ProcessPoolExecutor(max_workers=6) as executor:
        directory = 'graphFiles'
        params = []
        for i in range(2):
            print(i)
            for filename in os.listdir(directory):
                params.append(('PSO', filename, i))
                params.append(('GA', filename, i))

        runlist = []
        for run in params:
            if not os.path.isfile(logfile_path(*run)):
                runlist.append(run)

        futures = {executor.submit((PSO.pso_main.main if run[0] == 'PSO' else GA.ga_main.main), os.path.join(directory, run[1])): run for run in runlist}
        for future in concurrent.futures.as_completed(futures):
            parameters = futures[future]
            log = future.result()
            with open(logfile_path(*parameters), 'w') as outfile:
                json.dump(log, outfile, indent=2)
                print(f'{logfile_path(*parameters)} finished')
        # print(filename)
        # file = os.path.join(directory, filename)
        # data = PSO.pso_main.main(file)
        # with open(f"outputFiles\\" + logfile_path('PSO', filename, i), 'w') as outfile:
        #     json.dump(data, outfile)
        # data = GA.ga_main.main(file)
        # with open(f"outputFiles\\" + logfile_path('PSO', filename, i), 'w') as outfile:
        #     json.dump(data, outfile)
