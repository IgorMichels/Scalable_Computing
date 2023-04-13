import os
import numpy as np

from time import time
from glob import glob
from random import randint

from Highway import Highway

if __name__ == '__main__':
    TOTAL_HIGHWAYS = 5
    highwayCodes = np.arange(100, 100 + TOTAL_HIGHWAYS)
    highways = list()
    for code in highwayCodes:
        highways.append(Highway(
            code = code,
            numLanesN = randint(2, 4),
            numLanesS = randint(2, 4),
            maxSpeed = randint(10, 15),
            probNewCar = randint(25, 50) / 100,
            probChangeLane = randint(1, 10) / 100,
            speedLimitsCar = (randint(1, 6), randint(10, 20)),
            accelerationLimitsCar = (randint(-3, -1), randint(1, 3)),
            probCrash = randint(1, 5) / 100,
            cleanLaneEpochs = randint(1, 10),
            highwayExtension = randint(100, 200)
        ))
    
    t = time()
    for epoch in range(5):
        for hw in highways: hw.simulate()

    tf = time()

    files = glob('*.txt')
    for file in files: os.remove(file)

    print(f'Total time: {tf - t:.2f} seconds')