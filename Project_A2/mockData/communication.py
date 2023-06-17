from celery import Celery
from db_connection import *

# Configuração do Celery
app = Celery('communication', broker = 'amqp://guest@localhost//', backend = 'rpc://')

@app.task
def SendCarInfo(plate, pos, lane, highway, time):
    print(f'Carro {plate} estava na pista {lane} e posição {pos} da rodovia {highway} às {time}.')
    dic_data = {'plate' : plate,
                'pos' : pos,
                'lane' :lane,
                'highway' :highway,
                'time' : time}
    
    db_cars.insert_one(dic_data)

@app.task
def SendHighwayInfo(code, maxSpeed, extension):
    dic_data = {'highway_code' : code,
                'max_speed' : maxSpeed,
                'extension' : extension}
    
    db_highways.insert_one(dic_data)

# rabbitmq-server
# celery -A communication worker --loglevel=info