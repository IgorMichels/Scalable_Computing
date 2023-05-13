import os
import shutil
import numpy as np

from time import time
from glob import glob
from random import randint, shuffle

import sys
sys.path.append('mockData/')

from Highway import Highway

CLEAN = True
SIMS = 0
code = 100
for arg in sys.argv:
    if '-c' in arg:
        if arg[-1] == '0': CLEAN = False
        else: CLEAN = True
    elif '-s' in arg:
        arg = arg.split('=')[-1]
        SIMS = int(arg)
    elif '-h' in arg:
        arg = arg.split('=')[-1]
        code = int(arg)

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
            highwayExtension = randint(1000, 1500),
        )
    
    t = time()
    if SIMS == 0:
        while True:
            hw.simulate()
            with open(f'extraInfoCarsLinear.txt', 'r') as f: plates = f.readlines()
            shuffle(plates)
            with open('extraInfoCars.txt', 'a') as f: f.writelines(plates)
            shutil.move('extraInfoCars.txt', 'mockData/extraInfoCars.txt')
    else:
        for _ in range(SIMS):
            hw.simulate()
            with open(f'extraInfoCarsLinear.txt', 'r') as f: plates = f.readlines()
            shuffle(plates)
            with open('extraInfoCars.txt', 'a') as f: f.writelines(plates)
            shutil.move('extraInfoCars.txt', 'mockData/extraInfoCars.txt')

    tf = time()

    if CLEAN:
        files = glob('files/*.txt')
        for file in files: os.remove(file)

    print(f'Total time: {tf - t:.2f} seconds')