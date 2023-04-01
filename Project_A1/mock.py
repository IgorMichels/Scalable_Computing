import numpy as np
import matplotlib.pyplot as plt

from random import random, randint, choice

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
        self.direction = 1 if direction == 'S' else -1
        self.maxSpeedLane = maxSpeed
        self.highwayExtension = highwayExtension
        self.pos = 0
        self.currSpeed = randint(self.minSpeed, self.maxSpeedLane)
        self.isCrashed = False

    def crash(self):
        self.isCrashed = True

    def updateCar(self, highwayStatus):
        if self.isCrashed: return

        if random() < self.probCrash: self.willCrash = True
        if self.willCrash:
            # primeiro verifica se há algum carro ao seu lado
            # se sim, vai em direção ao mesmo, causando a colisão

            # se não, busca em que pista está o carro que está
            # imediatamente atrás e freia bruscamente mudando 
            # para aquela pista
            if self.direction == 1:
                # sentido sul
                if np.sum(highwayStatus[self.pos, self.actualLane:self.numLanes]) >= 2:
                    self.pos += self.currSpeed
                    self.actualLane += 1
                    return self.pos, self.actualLane
                elif np.sum(highwayStatus[self.pos, :self.actualLane]) >= 1:
                    self.pos += self.currSpeed
                    self.actualLane -= 1
                    return self.pos, self.actualLane

                for i in range(self.pos, 0, -1):
                    if np.sum(highwayStatus[i, self.actualLane:self.numLanes]) >= 1:
                        self.currSpeed += self.minAcceleration
                        self.pos += self.currSpeed
                        self.actualLane += 1
                        return self.pos, self.actualLane
                    elif np.sum(highwayStatus[i, :self.actualLane]) >= 1:
                        self.currSpeed += self.minAcceleration
                        self.pos += self.currSpeed
                        self.actualLane -= 1
                        return self.pos, self.actualLane

            else:
                # sentido norte
                if np.sum(highwayStatus[self.pos, self.actualLane:]) >= 2:
                    self.pos -= self.currSpeed
                    self.actualLane += 1
                    return self.pos, self.actualLane
                elif np.sum(highwayStatus[self.pos, -self.numLanes:self.actualLane]) >= 1:
                    self.pos -= self.currSpeed
                    self.actualLane -= 1
                    return self.pos, self.actualLane

                for i in range(self.pos, self.highwayExtension + 1):
                    if np.sum(highwayStatus[i, self.actualLane:]) >= 1:
                        self.currSpeed += self.minAcceleration
                        self.pos -= self.currSpeed
                        self.actualLane += 1
                        return self.pos, self.actualLane
                    elif np.sum(highwayStatus[i, -self.numLanes:self.actualLane]) >= 1:
                        self.currSpeed += self.minAcceleration
                        self.pos -= self.currSpeed
                        self.actualLane -= 1
                        return self.pos, self.actualLane
        
        if random() < self.probChangeLane:
            if self.actualLane == 0: self.actualLane += 1
            elif self.actualLane == self.numLanes - 1: self.actualLane -= 1
            else: self.actualLane += choice([-1, 1])

        # fazer mecanismo para frear caso tenha um acidente a frente
        # bem como ele se acidentar se passar por "cima" de uma colisão
        self.currSpeed += randint(self.minAcceleration, self.maxAcceleration)
        self.pos += self.currSpeed
        return self.pos, self.actualLane

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
        self.highwayStatus = np.zeros((self.highwayExtension + 1, self.numLanesN + self.numLanesS))
        self.cars = list()

    def updateHighwayStatus(self):
        for i in range(self.highwayStatus.shape[1]):
            if np.sum(self.highwayStatus[:, i] < 0) > 0:
                args = np.argsort(self.highwayStatus[:, i])
                j = 0
                while self.highwayStatus[args[j], i] < 0:
                    self.highwayStatus[args[j], i] += 1
                    j += 1

        for i, car in enumerate(self.cars):
            car, pos, lane = car
            update = car.updateCar(self.highwayStatus)
            if update == None: continue

            # remove carro da posição anterior
            self.highwayStatus[pos, lane] = 0
            pos, lane = update
            
            self.cars[i] = [car, pos, lane]
        
        for i, car in enumerate(self.cars):
            car, pos, lane = car
            if pos < 0 or pos > self.highwayExtension:
                self.cars.pop(i)
                continue

            if car.direction == -1:
                print(lane, self.numLanesS, lane + self.numLanesS)
                lane += self.numLanesS
            
            if self.highwayStatus[pos, lane] == 1: self.highwayStatus[pos, lane] = -self.cleanLaneEpochs
            else: self.highwayStatus[pos, lane] = 1

        for i in range(self.numLanesS):
            if random() < self.probNewCar:
                newCar = Car(self.speedLimitsCar,
                             self.accelerationLimitsCar,
                             self.probCrash,
                             self.probChangeLane,
                             i,
                             self.numLanesS,
                             self.maxSpeed,
                             self.highwayExtension,
                             'S'
                             )
                self.cars.append([newCar, 0, i])

        for i in range(self.numLanesN):
            if random() < self.probNewCar:
                newCar = Car(self.speedLimitsCar,
                             self.accelerationLimitsCar,
                             self.probCrash,
                             self.probChangeLane,
                             i,
                             self.numLanesN,
                             self.maxSpeed,
                             self.highwayExtension,
                             'N'
                             )
                self.cars.append([newCar, self.highwayExtension, i])
                

    def simulate(self,
                 epochs : int):
        
        for _ in range(epochs):
            self.updateHighwayStatus()
            print(self.highwayStatus)

if __name__ == '__main__':
    hw = Highway(
                 code = 101,
                 numLanesN = 3,
                 numLanesS = 2,
                 maxSpeed = 1,
                 probNewCar = 0.05,
                 probChangeLane = 0.05,
                 speedLimitsCar = (0, 5),
                 accelerationLimitsCar = (0, 1),
                 probCrash = 0.01,
                 cleanLaneEpochs = 10,
                 highwayExtension = 10
    )

    hw.simulate(1000)