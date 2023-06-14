import numpy as np

from random import random, randint, choice, shuffle
from communication import SendInfo
from datetime import datetime

class Car:
    def __init__(self,
                 speedLimitsCar : tuple,
                 accelerationLimitsCar : tuple,
                 probCrash : float,
                 probChangeLane : float,
                 actualLane : int,
                 numLanes : int,
                 maxSpeed : int,
                 highwayExtension : int,
                 direction : str,
                 highwayCode : int,
                 plate : str,
                 numlanesS : int
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
        self.lastPos = None
        self.penultimatePos = None
        self.currSpeed = randint(self.minSpeed, self.maxSpeedLane)
        self.isCrashed = False
        self.highwayCode = highwayCode
        self.plate = plate
        self.numLanesS = numlanesS

    def crash(self):
        self.isCrashed = True

    def tryCrash(self,
                 highwayStatus : np.array):
        # primeiro verifica se há algum carro ao seu lado
        # se sim, vai em direção ao mesmo, tentando colidir
        if np.sum(highwayStatus[self.pos, self.actualLane:self.numLanes, 0] == 1) >= 2:
            self.pos += self.currSpeed
            self.actualLane += 1
            if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
            else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
            return
        elif np.sum(highwayStatus[self.pos, :self.actualLane, 0] == 1) >= 1:
            self.pos += self.currSpeed
            self.actualLane -= 1
            if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
            else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
            return

        # se não, busca em que pista está o carro que está
        # imediatamente atrás e freia bruscamente mudando 
        # para aquela pista
        self.currSpeed += self.minAcceleration
        if self.currSpeed < self.minSpeed: self.currSpeed = self.minSpeed
        for i in range(self.pos, 0, -1):
            if np.sum(highwayStatus[i, self.actualLane + 1:self.numLanes, 0] == 1) >= 1:
                self.pos += self.currSpeed
                self.actualLane += 1
                if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
                else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
                return
            elif highwayStatus[i, self.actualLane, 0] == 1:
                self.pos += self.currSpeed
                if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
                else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
                return
            elif np.sum(highwayStatus[i, :self.actualLane, 0] == 1) >= 1:
                self.pos += self.currSpeed
                self.actualLane -= 1
                if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
                else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
                return

        # não há carros atrás
        self.pos += self.currSpeed
        if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
        else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
        return
    
    def changeLane(self):
        if self.actualLane == 0: newLane = self.actualLane + 1
        elif self.actualLane == self.numLanes - 1: newLane = self.actualLane - 1
        else: newLane = self.actualLane + choice([-1, 1])

        return newLane
    
    def tryLane(self,
                highwayStatus : np.array,
                newSpeed : int,
                newLane : int):
        
        if newLane < 0 or newLane == self.numLanes: return False, False
        minPossibleSpeed = self.currSpeed + self.minAcceleration
        if (highwayStatus[self.pos + 1:self.pos + newSpeed, newLane, 0] == 0).all():
            # caminho limpo para seguir com essa velocidade nessa pista
            return newSpeed, newLane
        
        occupieds = list()
        for i in range(1, newSpeed + 1):
            if self.pos + i >= highwayStatus.shape[0]: break
            if highwayStatus[self.pos + i, newLane, 0] != 0:
                occupieds.append(self.pos + i + max(highwayStatus[self.pos + i, newLane, 1] + self.minAcceleration, 0))

        occupied = min(occupieds)
        if self.pos + newSpeed < occupied:
            # tem um carro a frente, mas mesmo ele freando bruscamente
            # a gente pode ir para a posição desejada
            return newSpeed, newLane
        
        if self.pos + minPossibleSpeed < occupied:
            # tem um carro a frente que, se frear bruscamente, a gente bate
            while self.pos + newSpeed >= occupied: newSpeed -= 1

            # vamos reduzir a velocidade até que chegamos num ponto onde o
            # carro da frente tem liberdade para frear
            return newSpeed, newLane
        
        return False, occupied

    def avoidCrash(self,
                   highwayStatus : np.array):
        
        newSpeed = self.currSpeed + randint(self.minAcceleration, self.maxAcceleration)
        if newSpeed > self.maxSpeed: newSpeed = self.maxSpeed
        if newSpeed < self.minSpeed: newSpeed = self.minSpeed
        if random() < self.probChangeLane and self.numLanes != 1: newLane = self.changeLane()
        else: newLane = self.actualLane

        results = list()
        results.append(self.tryLane(highwayStatus, newSpeed, self.actualLane - 1))
        results.append(self.tryLane(highwayStatus, newSpeed, self.actualLane))
        results.append(self.tryLane(highwayStatus, newSpeed, self.actualLane + 1))
        ind = newLane - self.actualLane + 1
        if results[ind][0] != False:
            # podemos seguir na pista desejada
            newSpeed, newLane = results[ind]
            self.currSpeed = newSpeed
            self.actualLane = newLane
            self.pos += self.currSpeed
            if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
            else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
            return
        
        if ind != 1:
            # queremos mudar de pista, mas vamos bater se mudar para a escolhida
            ind -= 2
            ind = abs(ind)
            if results[ind][0] != False:
                # podemos mudar para a pista do outro lado
                newSpeed, newLane = results[ind]
                self.currSpeed = newSpeed
                self.actualLane += ind - 1
                self.pos += self.currSpeed
                if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
                else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
                return
            
        # se não conseguimos uma posição até agora, então vamos escolher a menos pior
        best = 1
        opt = [0, 1, 2]
        shuffle(opt)
        for i in opt:
            if results[i][0] != False and results[best][0] == False: best = i
            elif results[best][0] == False:
                if results[i][1] > results[best][1]: best = i

        # a nova pista vai ser a menos pior
        self.actualLane += best - 1
        self.currSpeed += self.minAcceleration
        self.pos += self.currSpeed
        if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
        else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
        return
        
    def greenFlag(self):
        if random() < self.probChangeLane and self.numLanes != 1: self.actualLane = self.changeLane()
        self.currSpeed += randint(self.minAcceleration, self.maxAcceleration)
        if self.currSpeed > self.maxSpeed: self.currSpeed = self.maxSpeed
        if self.currSpeed < self.minSpeed: self.currSpeed = self.minSpeed
        
        self.pos += self.currSpeed
        if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
        else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
        return

    def updateCar(self,
                  highwayStatus : np.array):
        
        self.penultimatePos = self.lastPos
        self.lastPos = self.pos

        if self.isCrashed:
            if self.direction == 'S': SendInfo.delay(self.plate, self.pos, self.actualLane, self.highwayCode, datetime.now())
            else: SendInfo.delay(self.plate, self.highwayExtension - self.pos, self.numLanesS + self.actualLane, self.highwayCode, datetime.now())
            return
        
        if random() < self.probCrash: self.willCrash = True

        # carro vai causar uma colisão
        if self.willCrash: return self.tryCrash(highwayStatus)
        
        # não causou colisão, vamos ver se há algum carro a frente
        maxDelta = min(self.currSpeed + self.maxAcceleration, self.maxSpeed)
        if (highwayStatus[self.pos + 1:self.pos + maxDelta, self.actualLane, 0] != 0).any(): return self.avoidCrash(highwayStatus)

        # caminho limpo
        return self.greenFlag()
