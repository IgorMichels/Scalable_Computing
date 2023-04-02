import os
import numpy as np
import matplotlib.pyplot as plt

from time import time
from glob import glob
from itertools import product
from random import random, randint, choice, shuffle

class Car:
    def __init__(self,
                 speedLimitsCar : tuple[float, float],
                 accelerationLimitsCar : tuple[float, float],
                 probCrash : float,
                 probChangeLane : float,
                 actualLane : int,
                 numLanes : int,
                 maxSpeed : int,
                 highwayExtension : int,
                 direction : str
                 ):
        
        self.minSpeed = speedLimitsCar[0]
        self.maxSpeed = speedLimitsCar[1]
        self.minAcceleration = accelerationLimitsCar[0]
        self.maxAcceleration = accelerationLimitsCar[1]
        self.probCrash = probCrash
        self.willCrash = False
        self.probChangeLane = probChangeLane
        self.actualLane = actualLane
        self.numLanes = numLanes
        self.direction = direction
        self.maxSpeedLane = maxSpeed
        self.highwayExtension = highwayExtension
        self.pos = 0
        self.currSpeed = randint(self.minSpeed, self.maxSpeedLane)
        self.isCrashed = False
        self.plate = next(self.generatePlate())

    def generatePlate(self):
        letter1 = list('QWERTYUIOPASDFGHJKLZXCVBNM')
        letter2 = list('QWERTYUIOPASDFGHJKLZXCVBNM')
        letter3 = list('QWERTYUIOPASDFGHJKLZXCVBNM')
        letter4 = list('QWERTYUIOPASDFGHJKLZXCVBNM')
        number1 = list('1234567890')
        number2 = list('1234567890')
        number3 = list('1234567890')
        shuffle(letter1)
        shuffle(letter2)
        shuffle(letter3)
        shuffle(letter4)
        shuffle(number1)
        shuffle(number2)
        shuffle(number3)
        for l1, l2, l3, l4, n1, n2, n3 in product(letter1, letter2, letter3, letter4, number1, number2, number3):
            yield l1 + l2 + l3 + n1 + l4 + n2 + n3

    def crash(self):
        self.isCrashed = True

    def tryCrash(self, highwayStatus : np.array):
        # primeiro verifica se há algum carro ao seu lado
        # se sim, vai em direção ao mesmo, tentando colidir
        if np.sum(highwayStatus[self.pos, self.actualLane:self.numLanes] == 1) >= 2:
            self.pos += self.currSpeed
            self.actualLane += 1
            return
        elif np.sum(highwayStatus[self.pos, :self.actualLane] == 1) >= 1:
            self.pos += self.currSpeed
            self.actualLane -= 1
            return

        # se não, busca em que pista está o carro que está
        # imediatamente atrás e freia bruscamente mudando 
        # para aquela pista
        self.currSpeed += self.minAcceleration
        if self.currSpeed < self.minSpeed: self.currSpeed = self.minSpeed
        for i in range(self.pos, 0, -1):
            if np.sum(highwayStatus[i, self.actualLane + 1:self.numLanes] == 1) >= 1:
                self.pos += self.currSpeed
                self.actualLane += 1
                return
            elif highwayStatus[i, self.actualLane] == 1:
                self.pos += self.currSpeed
                return
            elif np.sum(highwayStatus[i, :self.actualLane] == 1) >= 1:
                self.pos += self.currSpeed
                self.actualLane -= 1
                return

        # não há carros atrás
        self.pos += self.currSpeed
        return
    
    def avoidCrash(self, highwayStatus : np.array):
        breaking = randint(self.minAcceleration, 0)
        breaking += self.currSpeed
        if self.actualLane != self.numLanes - 1 and not (highwayStatus[self.pos:self.pos + breaking, self.actualLane + 1] < 0).any():
            self.actualLane += 1
            self.currSpeed += breaking
            if self.currSpeed > self.maxSpeed: self.currSpeed = self.maxSpeed
            if self.currSpeed < self.minSpeed: self.currSpeed = self.minSpeed
            
            self.pos += self.currSpeed
            return

        elif self.actualLane != 0 and not (highwayStatus[self.pos:self.pos + breaking, self.actualLane - 1] < 0).any():
            self.actualLane -= 1
            self.currSpeed += breaking
            if self.currSpeed > self.maxSpeed: self.currSpeed = self.maxSpeed
            if self.currSpeed < self.minSpeed: self.currSpeed = self.minSpeed
            
            self.pos += self.currSpeed
            return
        
        elif self.currSpeed == 0:
            aux = min(abs(self.minAcceleration), abs(self.maxAcceleration))
            i = 1
            while i <= aux:
                if highwayStatus[self.pos + i, self.actualLane] != 0:
                    i -= 1
                    break

                i += 1

            if i == aux + 1: i = aux
            self.pos += i
            return

        else:
            self.currSpeed += self.minAcceleration
            if self.currSpeed < 0: self.currSpeed = 0
            
            self.pos += self.currSpeed
            return
        
    def greenFlag(self):
        if random() < self.probChangeLane and self.numLanes != 1:
            if self.actualLane == 0: self.actualLane += 1
            elif self.actualLane == self.numLanes - 1: self.actualLane -= 1
            else: self.actualLane += choice([-1, 1])

        self.currSpeed += randint(self.minAcceleration, self.maxAcceleration)
        if self.currSpeed > self.maxSpeed: self.currSpeed = self.maxSpeed
        if self.currSpeed < self.minSpeed: self.currSpeed = self.minSpeed
        
        self.pos += self.currSpeed
        return

    def updateCar(self, highwayStatus : np.array):
        if self.isCrashed: return
        if random() < self.probCrash: self.willCrash = True

        # carro vai causar uma colisão
        if self.willCrash: return self.tryCrash(highwayStatus)
        
        # não causou colisão, vamos ver se há uma a frente
        if (highwayStatus[self.pos + 1:, self.actualLane] < 0).any(): return self.avoidCrash(highwayStatus)

        # caminho limpo
        return self.greenFlag()

class Highway:
    def __init__(self,
                 code : int,
                 numLanesN : int,
                 numLanesS : int,
                 maxSpeed : float,
                 probNewCar : float,
                 probChangeLane : float,
                 speedLimitsCar : tuple[float, float],
                 accelerationLimitsCar : tuple[float, float],
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
        self.highwayStatusSouth = np.zeros((self.highwayExtension + 1, self.numLanesS))
        self.highwayStatusNorth = np.zeros((self.highwayExtension + 1, self.numLanesN))
        self.carsSouth = list()
        self.carsNorth = list()
        self.actualEpoch = 0

    def updateHighwayStatus(self, direction : str):
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
            if np.sum(highwayStatus[:, i] < 0) > 0:
                args = np.argsort(highwayStatus[:, i])
                j = 0
                while highwayStatus[args[j], i] < 0:
                    highwayStatus[args[j], i] += 1
                    j += 1

        oldCarsPositions = list()
        newCarsPositions = list()
        for i, car in enumerate(cars):
            oldPos, oldLane = car.pos, car.actualLane
            oldCarsPositions.append([oldPos, oldLane])
            car.updateCar(highwayStatus)

            # remove carro da posição anterior se ele não está batido
            if not car.isCrashed: highwayStatus[oldPos, oldLane] = 0
                
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
            if pos < 0 or pos > self.highwayExtension or (car.isCrashed and highwayStatus[car.pos, car.actualLane] == 0):
                drop.append(i)
                continue
            
            if highwayStatus[pos, lane] == 0:
                highwayStatus[pos, lane] = 1
            elif not car.isCrashed:
                highwayStatus[pos, lane] = -self.cleanLaneEpochs
                car.crash()
                for j, car2 in enumerate(cars):
                    pos2, lane2 = car2.pos, car2.actualLane
                    if pos2 == pos and lane2 == lane:
                        car2.crash()
                        break

        for i in drop[::-1]: cars.pop(i)

        # adiciona novos carros
        for i in range(numLanes):
            if highwayStatus[0, i] != 0: continue
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
                highwayStatus[0, i] = 1

    def sendStatus(self):
        with open(f'{self.highwayCode}_{str(self.actualEpoch).zfill(5)}.txt', 'w') as f:
            for car in self.carsSouth: f.write(f'{car.plate}, ({car.actualLane}, {car.pos})\n')
            for car in self.carsNorth: f.write(f'{car.plate}, ({self.numLanesS + car.actualLane}, {self.highwayExtension - car.pos})\n')

    def simulate(self):
        self.updateHighwayStatus('S')
        self.updateHighwayStatus('N')
        self.sendStatus()
        self.actualEpoch += 1

    def simEpochs(self, epochs : int):
        for _ in range(epochs):
            self.simulate()


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