import numpy as np
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
                    self.pos += self.currSpeed
                    self.actualLane += 1
                    return self.pos, self.actualLane
                elif np.sum(highwayStatus[self.pos, -self.numLanes:self.actualLane]) >= 1:
                    self.pos += self.currSpeed
                    self.actualLane -= 1
                    return self.pos, self.actualLane

                for i in range(self.pos, self.highwayExtension + 1):
                    if np.sum(highwayStatus[i, self.actualLane:]) >= 1:
                        self.currSpeed += self.minAcceleration
                        self.pos += self.currSpeed
                        self.actualLane += 1
                        return self.pos, self.actualLane
                    elif np.sum(highwayStatus[i, -self.numLanes:self.actualLane]) >= 1:
                        self.currSpeed += self.minAcceleration
                        self.pos += self.currSpeed
                        self.actualLane -= 1
                        return self.pos, self.actualLane
                    
        if random() < self.probChangeLane:
            if self.actualLane == 0: self.actualLane += 1
            elif self.actualLane == self.numLanes - 1: self.actualLane -= 1
            else: self.actualLane += choice([-1, 1])

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
    
        pass

    def simulate(self,
                 epochs : int):
        
        pass

if __name__ == '__main__':
    direction = -1
    pos = 25
    numLanes = 4
    highwayStatus = np.zeros((25, 7))
    if direction == 1:
        # sentido sul
        state = highwayStatus[:pos + 1, :numLanes]
    else:
        # sentido norte
        state = highwayStatus[pos - 1:, - numLanes:]

    print(state.shape)