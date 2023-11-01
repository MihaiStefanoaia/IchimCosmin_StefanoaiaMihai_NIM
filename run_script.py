import subprocess
import time
import os
import numpy as np

if __name__ == '__main__':
    tolerances = {
        'rastrigin' : {
            2 : 0.001,
            10: 0.1,
            30: 5,
        },
        'rosenbrock' : {
            2: 0.001,
            10: 0.1,
            30: 1,
        },
        'griewangk' : {
          2: 0,
          10: 0.001,
          30: 0.1
        },
        'michalewicz' : {
            2: -1.6,
            10: -9,
            30: -29
        }
    }
    for f in ['michalewicz']:
        d = 10
        for p in [100, 500, 1000]:
            for hcs in ['none', 'first', 'best']:
                for hcf in [0, 1, 5]:
                    for c in [1, 2, 3, 10]:
                        ran_any = False
                        for m in np.arange(0.00035, 0.005, 0.0005):
                            subprocesses = {}
                            for cp in range(10, 80, 10):
                                for sp in range(10, 80, 10):
                                    filename = f'results/f_{f}_d_{d}_m_{m}_p_{p}_c_{c}_hcs_{hcs}_hcf_{hcf}_cp_{cp}_sp_{sp}'
                                    if os.path.isfile(filename + '.json'):
                                        print(f'{filename} skipped')
                                        continue
                                    outfile = open(filename + '.txt', 'w')

                                    subprocesses[filename] = subprocess.Popen(
                                        ['./genetic-engine', '-d', str(d), '-t', str(tolerances[f][d]), '-f', f, '-p', str(p), '-m', str(m),
                                            '-g', '400', '-c', str(c), '-fq', '10', '-cp', str(cp), '-sp', str(sp), '-hc', hcs, '-hcf', str(hcf), '--log_file',
                                            (filename + '.json')],
                                        stdout=outfile)
                                    print(filename)
                                    ran_any = True
                            # if ran_any:
                            #     time.sleep(2.0)
                            for fn, pr in subprocesses.items():
                                pr.wait()
                                print(f'{fn} done')