import os
import numpy as np

from time import time
from random import randint

import sys
sys.path.append('mockData/')

from Highway import Highway

SIMS = 5000
code = 101
for arg in sys.argv:
    if '-s' in arg:
        arg = arg.split('=')[-1]
        SIMS = int(arg)
    elif '-h' in arg:
        arg = arg.split('=')[-1]
        code = int(arg)

SIMS = SIMS if SIMS != 0 else 5000

if __name__ == '__main__':
    if 'files' not in os.listdir(): os.mkdir('files')
    hw = Highway(
            code = code,
            numLanesN = randint(2, 4),
            numLanesS = randint(2, 4),
            maxSpeed = randint(10, 15),
            probNewCar = randint(25, 50) / 100,
            probChangeLane = randint(1, 10) / 100,
            speedLimitsCar = (randint(1, 6), randint(10, 20)),
            accelerationLimitsCar = (randint(-3, -1), randint(1, 3)),
            probCrash = randint(1, 5) / 100,
            cleanLaneEpochs = randint(3, 10),
            highwayExtension = randint(1000, 1500)
        )
    
    t = time()
    for _ in range(SIMS):
        hw.simulate()

    tf = time()
    print(f'Total time: {tf - t:.2f} seconds')