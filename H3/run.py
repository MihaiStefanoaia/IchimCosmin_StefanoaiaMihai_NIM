import concurrent.futures
import json
import os
import GA.ga_main
import PSO.pso_main


def logfile_path(funcname, filename, iteration):
    return f"outputFiles\\{funcname}_{str(filename.split('.')[0])}_{iteration}.json"


if __name__ == '__main__':
    directory = 'graphFiles'
    for file in os.listdir(directory):
        print(file)
        if os.path.isfile(logfile_path("GA", file, 0)):
            continue
        log = GA.ga_main.main(os.path.join(directory, file))
        with open(logfile_path("GA", file, 0), 'w') as outfile:
            json.dump(log, outfile, indent=2)
            print(f'{logfile_path("GA", file, 0)} finished')
    exit(0)
    with concurrent.futures.ProcessPoolExecutor(max_workers=1) as executor:
        directory = 'graphFiles'
        params = []
        for i in range(1):
            print(i)
            for filename in os.listdir(directory):
                # params.append(('PSO', filename, i))
                params.append(('GA', filename, i))

        runlist = []
        for run in params:
            if not os.path.isfile(logfile_path(*run)):
                runlist.append(run)

        futures = {executor.submit((PSO.pso_main.main if run[0] == 'PSO' else GA.ga_main.main), os.path.join(directory, run[1])): run for run in runlist}
        for future in concurrent.futures.as_completed(futures):
            parameters = futures[future]
            print(parameters)
            log = future.result()
            print(f'saving to {logfile_path(*parameters)}')
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
