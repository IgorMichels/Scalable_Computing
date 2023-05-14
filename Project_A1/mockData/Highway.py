import numpy as np
import shutil

from Car import Car
from random import random
from datetime import datetime

class Highway:
    def __init__(self,
                 code : int,
                 numLanesN : int,
                 numLanesS : int,
                 maxSpeed : float,
                 probNewCar : float,
                 probChangeLane : float,
                 speedLimitsCar : tuple, #
                 accelerationLimitsCar : tuple, #
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

        oldCarsPositions = list()
        newCarsPositions = list()
        for i, car in enumerate(cars):
            oldPos, oldLane = car.pos, car.actualLane
            oldCarsPositions.append([oldPos, oldLane])
            car.updateCar(highwayStatus)

            # remove carro da posição anterior se ele não está batido
            if not car.isCrashed: 
                highwayStatus[oldPos, oldLane, 0] = 0
                highwayStatus[oldPos, oldLane, 1] = 0
                
            newPos, newLane = car.pos, car.actualLane
            newCarsPositions.append([newPos, newLane])

        # se um carro "saltou" outro então houve colisão
        for i in range(len(cars)):
            if oldCarsPositions[i][1] != newCarsPositions[i][1]: continue # trocou de pista
            for j in range(i + 1, len(cars)):
                if oldCarsPositions[j][1] != newCarsPositions[j][1]: continue # trocou de pista
                if oldCarsPositions[i][1] != oldCarsPositions[j][1]: continue # não estão na mesma pista
                if oldCarsPositions[i][0] < oldCarsPositions[j][0] and newCarsPositions[i][0] > newCarsPositions[j][0]:
                    # bateram
                    crashPosition = (oldCarsPositions[j][0] + newCarsPositions[j][0]) // 2
                    cars[i].pos = crashPosition
                    cars[j].pos = crashPosition
                elif oldCarsPositions[i][0] > oldCarsPositions[j][0] and newCarsPositions[i][0] < newCarsPositions[j][0]:
                    # bateram
                    crashPosition = (oldCarsPositions[i][0] + newCarsPositions[i][0]) // 2
                    cars[i].pos = crashPosition
                    cars[j].pos = crashPosition

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

        for i in drop[::-1]: cars.pop(i)

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
                             direction[0]
                             )
                cars.append(newCar)
                highwayStatus[0, i, 0] = 1
                highwayStatus[0, i, 1] = newCar.currSpeed

    def sendStatus(self):
        fileName = f'{datetime.now()}.txt'
        with open(f'mockData/{self.highwayCode} {fileName}', 'w') as f:
            f.write(f'Highway {self.highwayCode}\n')
            f.write(f'MaxSpeedHighway {self.maxSpeed}\n')
            f.write(f'MaxSpeedCar {self.speedLimitsCar[1]}\n')
            for car in self.carsSouth:
                if car.penultimatePos is not None: f.write(f'{car.plate}, ({car.actualLane}, {car.penultimatePos})\n')
                if car.lastPos is not None: f.write(f'{car.plate}, ({car.actualLane}, {car.lastPos})\n')
                f.write(f'{car.plate}, ({car.actualLane}, {car.pos})\n')
            
            for car in self.carsNorth:
                if car.penultimatePos is not None: f.write(f'{car.plate}, ({self.numLanesS + car.actualLane}, {self.highwayExtension - car.penultimatePos})\n')
                if car.lastPos is not None: f.write(f'{car.plate}, ({self.numLanesS + car.actualLane}, {self.highwayExtension - car.lastPos})\n')
                f.write(f'{car.plate}, ({self.numLanesS + car.actualLane}, {self.highwayExtension - car.pos})\n')

            f.write(f'{datetime.now()}\n')

        shutil.move(f'mockData/{self.highwayCode} {fileName}', f'files/{fileName}')

    def simulate(self):
        self.updateHighwayStatus('S')
        self.updateHighwayStatus('N')
        self.sendStatus()
        self.actualEpoch += 1

    def simEpochs(self,
                  epochs : int):
        for _ in range(epochs):
            self.simulate()
