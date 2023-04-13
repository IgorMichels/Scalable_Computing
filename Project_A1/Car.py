import numpy as np

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
