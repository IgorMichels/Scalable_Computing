from tasks import *
import numpy as np

class Car:
    def __init__(self, plate, pos):
        self.plate = plate
        self.pos = pos

cars = {'car1':Car("AAAA111",0),
        'car2':Car("BBBB222",5),
        'car3':Car("CCCC333",1),
        'car4':Car("DDDD444",9)}

def call(car):
    Print.delay(car.plate)
    Speed.delay(car.pos, car.plate)
    Write.delay(car.plate)

if __name__ == '__main__':
    for i in range(10):
        car = cars[np.random.choice(list(cars.keys()))]
        call(car)