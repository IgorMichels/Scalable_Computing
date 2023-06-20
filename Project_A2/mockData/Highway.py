import numpy as np
import threading

from Car import Car
from random import random, shuffle
from itertools import product
from db_connection import *
from communication import SendHighwayInfo
from communication import SendCarInfo
from datetime import datetime

import sys
sys.path.append('../')

class Highway:
    def __init__(self,
                 code : int,
                 numLanesN : int,
                 numLanesS : int,
                 maxSpeed : float,
                 probNewCar : float,
                 probChangeLane : float,
                 speedLimitsCar : tuple,
                 accelerationLimitsCar : tuple,
                 probCrash : float,
                 cleanLaneEpochs : int,
                 highwayExtension : int
                 ):
    
        self.highwayCode = code
        self.numLanesN = numLanesN
        self.numLanesS = numLanesS
        self.maxSpeed = maxSpeed
        self.probNewCar = probNewCar
        self.probChangeLane = probChangeLane
        self.speedLimitsCar = speedLimitsCar
        self.accelerationLimitsCar = accelerationLimitsCar
        self.probCrash = probCrash
        self.cleanLaneEpochs = cleanLaneEpochs
        self.highwayExtension = highwayExtension
        self.highwayStatusSouth = np.zeros((self.highwayExtension + 1, self.numLanesS, 2), dtype = int)
        self.highwayStatusNorth = np.zeros((self.highwayExtension + 1, self.numLanesN, 2), dtype = int)
        self.carsSouth = list()
        self.carsNorth = list()
        self.actualEpoch = 0
        self.currPlates = list()
        self.interval_ip = int(random() * .45 * self.highwayExtension)
        self.interval_fp = int((1 - random() * .5) * self.highwayExtension)
        self.max_risk_events = max(0, (self.interval_fp - self.interval_ip) // self.speedLimitsCar[1] - 1)

        SendHighwayInfo.delay(self.highwayCode, self.maxSpeed, self.highwayExtension, self.speedLimitsCar[1],
                              self.interval_ip, self.interval_fp, self.max_risk_events, datetime.now())

    def createPlate(self):
        letters = list('QWERTYUIOPASDFGHJKLZXCVBNM')
        numbers1 = list('1234567890')
        numbers2 = list('1234567890')
        while True:
            shuffle(letters)
            shuffle(numbers1)
            shuffle(numbers2)
            for l, n1, n2 in product(letters, numbers1, numbers2):
                plate = f'{l}{n1}{n2}'
                if plate in self.currPlates: continue
                yield plate

    def updateHighwayStatus(self,
                            direction : str):
        direction = direction.upper()
        if direction.startswith('S'):
            highwayStatus = self.highwayStatusSouth
            numLanes = self.numLanesS
            cars = self.carsSouth
        else:
            highwayStatus = self.highwayStatusNorth
            numLanes = self.numLanesN
            cars = self.carsNorth
        
        for i in range(highwayStatus.shape[1]):
            if np.sum(highwayStatus[:, i, 0] < 0) > 0:
                args = np.argsort(highwayStatus[:, i, 0])
                j = 0
                while highwayStatus[args[j], i, 0] < 0:
                    highwayStatus[args[j], i, 0] += 1
                    j += 1

        positions = {}
        positions[self.highwayCode] = {}
        positions[self.highwayCode]['old'] = {}
        positions[self.highwayCode]['new'] = {}
        
        threads = list()
        for car in cars:
            positions[self.highwayCode]['old'][car.plate] = [car.pos, car.actualLane]
            # car.updateCar(highwayStatus)
            threads.append(threading.Thread(target = car.updateCar, args = (highwayStatus, )))
            threads[-1].start()
        
        for thread in threads: thread.join()
        
        for car in cars:
            # remove carro da posição anterior se ele não está batido
            if not car.isCrashed:
                highwayStatus[positions[self.highwayCode]['old'][car.plate][0], positions[self.highwayCode]['old'][car.plate][1], 0] = 0
                highwayStatus[positions[self.highwayCode]['old'][car.plate][0], positions[self.highwayCode]['old'][car.plate][1], 1] = 0
                
            positions[self.highwayCode]['new'][car.plate] = [car.pos, car.actualLane]

        # se um carro "saltou" outro então houve colisão
        for i, car in enumerate(cars):
            if car.isCrashed: continue
            if car.plate in positions[self.highwayCode]['old'] and positions[self.highwayCode]['old'][car.plate][1] != positions[self.highwayCode]['new'][car.plate][1]: continue # trocou de pista
            for j, car2 in enumerate(cars):
                if car.plate == car2.plate: continue
                if car.direction != car2.direction: continue
                if car2.plate in positions[self.highwayCode]['old'] and positions[self.highwayCode]['old'][car2.plate][1] != positions[self.highwayCode]['new'][car2.plate][1]: continue # trocou de pista
                if positions[self.highwayCode]['old'][car.plate][1] != positions[self.highwayCode]['old'][car2.plate][1]: continue # não estão na mesma pista
                if positions[self.highwayCode]['old'][car.plate][0] < positions[self.highwayCode]['old'][car2.plate][0] and positions[self.highwayCode]['new'][car.plate][0] > positions[self.highwayCode]['new'][car2.plate][0]:
                    # bateram
                    if car2.isCrashed:
                        car.pos = car2.pos
                        car.crash()
                        if car.direction == 'S': SendCarInfo.delay(car.plate, car.pos, car.actualLane, car.highwayCode, datetime.now())
                        else: SendCarInfo.delay(car.plate, car.highwayExtension - car.pos, car.numLanesS + car.actualLane, car.highwayCode, datetime.now())
                
                    else:
                        crashPosition = (positions[self.highwayCode]['old'][car2.plate][0] + positions[self.highwayCode]['new'][car2.plate][0]) // 2
                        car.pos = crashPosition
                        car.crash()
                        if car.direction == 'S': SendCarInfo.delay(car.plate, car.pos, car.actualLane, car.highwayCode, datetime.now())
                        else: SendCarInfo.delay(car.plate, car.highwayExtension - car.pos, car.numLanesS + car.actualLane, car.highwayCode, datetime.now())

                        car2.pos = crashPosition
                        car2.crash()
                        if car2.direction == 'S': SendCarInfo.delay(car2.plate, car2.pos, car2.actualLane, car2.highwayCode, datetime.now())
                        else: SendCarInfo.delay(car2.plate, car2.highwayExtension - car2.pos, car2.numLanesS + car2.actualLane, car2.highwayCode, datetime.now())
                
                if positions[self.highwayCode]['old'][car.plate][0] > positions[self.highwayCode]['old'][car2.plate][0] and positions[self.highwayCode]['new'][car.plate][0] < positions[self.highwayCode]['new'][car2.plate][0]:
                    # bateram
                    if car2.isCrashed:
                        car.pos = car2.pos
                        car.crash()
                        if car.direction == 'S': SendCarInfo.delay(car.plate, car.pos, car.actualLane, car.highwayCode, datetime.now())
                        else: SendCarInfo.delay(car.plate, car.highwayExtension - car.pos, car.numLanesS + car.actualLane, car.highwayCode, datetime.now())
                    else:
                        crashPosition = (positions[self.highwayCode]['old'][car.plate][0] + positions[self.highwayCode]['new'][car.plate][0]) // 2
                        car.pos = crashPosition
                        car.crash()
                        if car.direction == 'S': SendCarInfo.delay(car.plate, car.pos, car.actualLane, car.highwayCode, datetime.now())
                        else: SendCarInfo.delay(car.plate, car.highwayExtension - car.pos, car.numLanesS + car.actualLane, car.highwayCode, datetime.now())
                        car2.pos = crashPosition
                        car2.crash()
                        if car2.direction == 'S': SendCarInfo.delay(car2.plate, car2.pos, car2.actualLane, car2.highwayCode, datetime.now())
                        else: SendCarInfo.delay(car2.plate, car2.highwayExtension - car2.pos, car2.numLanesS + car2.actualLane, car2.highwayCode, datetime.now())

        drop = list()
        for i, car in enumerate(cars):
            pos, lane = car.pos, car.actualLane
            if pos < 0 or pos > self.highwayExtension or (car.isCrashed and highwayStatus[car.pos, car.actualLane, 0] == 0):
                drop.append(i)
                continue
            
            if highwayStatus[pos, lane, 0] == 0:
                highwayStatus[pos, lane, 0] = 1
                highwayStatus[pos, lane, 1] = car.currSpeed
            elif not car.isCrashed:
                highwayStatus[pos, lane, 0] = -self.cleanLaneEpochs
                highwayStatus[pos, lane, 1] = 0
                car.crash()
                for j, car2 in enumerate(cars):
                    pos2, lane2 = car2.pos, car2.actualLane
                    if pos2 == pos and lane2 == lane:
                        car2.crash()
                        break

        for i in drop[::-1]:
            removed_car = cars.pop(i)
            removed_plate = removed_car.plate
            self.currPlates.remove(removed_plate)

        # adiciona novos carros
        for i in range(numLanes):
            if highwayStatus[0, i, 0] != 0: continue
            if random() < self.probNewCar:
                newCar = Car(self.speedLimitsCar,
                             self.accelerationLimitsCar,
                             self.probCrash,
                             self.probChangeLane,
                             i,
                             numLanes,
                             self.maxSpeed,
                             self.highwayExtension,
                             direction[0],
                             self.highwayCode,
                             next(self.createPlate()),
                             self.numLanesS
                            )
                cars.append(newCar)
                self.currPlates.append(newCar.plate)
                highwayStatus[0, i, 0] = 1
                highwayStatus[0, i, 1] = newCar.currSpeed

    def simulate(self):
        t1 = threading.Thread(target = self.updateHighwayStatus, args = ('S', ))
        t2 = threading.Thread(target = self.updateHighwayStatus, args = ('N', ))
        t1.start()
        t2.start()
        t1.join()
        t2.join()
        #self.updateHighwayStatus('S')
        #self.updateHighwayStatus('N')

        self.actualEpoch += 1

    def simEpochs(self,
                  epochs : int):
        for _ in range(epochs):
            self.simulate()
