import subprocess
import time
import os

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
          10: 0.0001,
          30: 0.1
        },
        'michalewicz' : {
            2: -1.6,
            10: -9,
            30: -29
        }
    }
    for f in ['rastrigin','griewangk', 'rosenbrock', 'michalewicz']:
        for d in [2, 10, 30]:
            for p in [100, 500, 1000]:
                for hcs in ['none', 'first', 'best']:
                    subprocesses = {}
                    for hcf in [0, 1, 5]:
                        for i in range(1):
                            ran_any = False
                            for m in [0.001, 0.002, 0.00625, 0.01]:
                                cp = 65
                                sp = 50
                                filename = f'aaaa/f_{f}_d_{d}_p_{p}_hcs_{hcs}_hcf_{hcf}_m_{m}_{i}'
                                if os.path.isfile(filename + '.json'):
                                    print(f'{filename} skipped')
                                    continue
                                outfile = open(filename + '.txt', 'w')

                                subprocesses[filename] = subprocess.Popen(
                                    ['./genetic-engine', '-d', str(d), '-t', str(tolerances[f][d]), '-f', f, '-p', str(p), '-m', '0.000625',
                                        '-g', '1000', '-c', '2', '-fq', '10', '-cp', str(cp), '-sp', str(sp), '-hc', hcs, '-hcf', str(hcf), '--log_file',
                                        (filename + '.json')],
                                    stdout=outfile)
                                print(filename)
                                ran_any = True
                            if ran_any:
                                time.sleep(2.0)

                    for fn, pr in subprocesses.items():
                        pr.wait()
                        print(f'{fn} done')
