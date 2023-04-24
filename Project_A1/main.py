import os
import numpy as np

from time import time
from glob import glob
from random import randint, shuffle

from Highway import Highway

def printStatus(highwayStatus : np.array):
    nlins, ncols, _ = highwayStatus.shape
    print()
    for i in range(nlins):
        for j in range(ncols): print(f'({highwayStatus[i, j, 0]}, {highwayStatus[i, j, 1]})', end = ' ')
        print()

if __name__ == '__main__':
    if 'files' not in os.listdir(): os.mkdir('files')
    if 'extraInfoCars.txt' in os.listdir(): os.remove('extraInfoCars.txt')
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
    while True:
        for hw in highways:
            hw.simulate()
        
        with open('extraInfoCars.txt', 'r') as f: plates = f.readlines()
        
        shuffle(plates)
        with open('mockData/extraInfoCars.txt', 'w') as f: f.writelines(plates)

    tf = time()

    if CLEAN:
        files = glob('files/*.txt')
        for file in files: os.remove(file)

    print(f'Total time: {tf - t:.2f} seconds')