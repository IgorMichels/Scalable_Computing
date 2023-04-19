import os
import numpy as np

from time import time
from glob import glob
from random import randint

from Highway import Highway

def printStatus(highwayStatus : np.array):
    nlins, ncols, _ = highwayStatus.shape
    print()
    for i in range(nlins):
        for j in range(ncols): print(f'({highwayStatus[i, j, 0]}, {highwayStatus[i, j, 1]})', end = ' ')
        print()

if __name__ == '__main__':
    CLEAN = False
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
            cleanLaneEpochs = randint(3, 10),
            highwayExtension = randint(1000, 1500)
        ))
    
    t = time()
    for epoch in range(100):
        print('simulando')
        for hw in highways:
            hw.simulate()
            # if hw.highwayCode == 101: printStatus(hw.highwayStatusSouth)

    tf = time()

    if CLEAN:
        files = glob('files/*.txt')
        for file in files: os.remove(file)

    print(f'Total time: {tf - t:.2f} seconds')