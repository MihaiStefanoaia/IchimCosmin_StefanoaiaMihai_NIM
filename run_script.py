import subprocess
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
    subprocesses = []
    for f in ['rastrigin', 'griewangk', 'rosenbrock', 'michalewicz']:
        for d in [2, 10, 30]:
            for p in [100, 500, 1000]:
                for cp in [45, 55, 65]:
                    for sp in [40, 50, 60]:
                        outfile = open(f'results/f_{f}_d_{d}_p_{p}_cp_{cp}_sp_{sp}.txt', 'w')

                        subprocesses += [subprocess.Popen(
                            ['./genetic-engine', '-d', str(d), '-t', str(tolerances[f][d]), '-f', f, '-p', str(p), '-m', '0.000625',
                             '-g', '1500', '-c', '2', '-fq', '10', '-cp', str(cp), '-sp', str(sp), '--log_file',
                             f'results/f_{f}_d_{d}_p_{p}_cp_{cp}_sp_{sp}.json'],
                            stdout=outfile)]
                        print(f'started a run with function {f} at {d} dimensions')

    for p in subprocesses:
        p.wait()
        print('done')
