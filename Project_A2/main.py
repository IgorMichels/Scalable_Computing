import threading
from time import time
from random import randint

import sys
sys.path.append('mockData/')

from Highway import Highway
from db_connection import *
 
SIMS = 20
TOTAL = 1
INITIAL = 100
REMOVE_INITIAL = True
REMOVE_FINAL = False
for arg in sys.argv:
    if '-s' in arg:
        arg = arg.split('=')[-1]
        SIMS = int(arg)
    if '-t' in arg:
        arg = arg.split('=')[-1]
        TOTAL = int(arg)
    elif '-h' in arg:
        arg = arg.split('=')[-1]
        INITIAL = int(arg)
    elif '-ri' in arg:
        arg = arg.split('=')[-1]
        REMOVE_INITIAL = arg == '1'
    elif '-rf' in arg:
        arg = arg.split('=')[-1]
        REMOVE_FINAL = arg == '1'

codes = [*range(INITIAL, INITIAL + TOTAL)]
SIMS = SIMS if SIMS != 0 else 5000
if REMOVE_INITIAL:
    db_cars.delete_many({})
    db_highways.delete_many({})

if __name__ == '__main__':
    highways = list()
    for code in codes:
        hw = Highway(
                code = code,
                numLanesN = randint(2, 4),
                numLanesS = randint(2, 4),
                maxSpeed = randint(22, 31), # 80 km/h a 110 km/h
                probNewCar = randint(10, 25) / 100,
                probChangeLane = randint(1, 10) / 100,
                speedLimitsCar = (randint(11, 16), randint(33, 56)), # min entre 40 e 55 km/h e max entre 120 e 200 km/h
                accelerationLimitsCar = (randint(-5, -1), randint(1, 7)),
                probCrash = randint(1, 5) / 100,
                cleanLaneEpochs = randint(3, 10),
                highwayExtension = randint(3000, 5000)
            )
        
        highways.append(hw)
    
    t = time()
    threads = list()
    for hw in highways:
        threads.append(threading.Thread(target = hw.simEpochs, args = (SIMS, )))
        threads[-1].start()
        
    for thread in threads: thread.join()
    tf = time()
    print(f'Total time: {tf - t:.2f} seconds')

    if REMOVE_FINAL:
        db_cars.delete_many({})
        db_highways.delete_many({})
