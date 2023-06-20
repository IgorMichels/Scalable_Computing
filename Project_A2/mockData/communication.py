from celery import Celery
from db_connection import *

# Configuração do Celery
app = Celery('communication', broker = 'amqp://guest@localhost//', backend = 'rpc://')

@app.task
def SendCarInfo(plate, pos, lane, highway, time):
    dic_data = {'plate'   : plate,
                'pos'     : pos,
                'lane'    : lane,
                'highway' : highway,
                'time'    : time}
    
    db_cars.insert_one(dic_data)

@app.task
def SendHighwayInfo(code, maxSpeed, extension, maxSpeedCars, interval_ip, interval_fp, max_risk_events, time):
    dic_data = {'highway'           : code,
                'highway_max_speed' : maxSpeed,
                'highway_extension' : extension,
                'car_max_speed'     : maxSpeedCars,
                'interval_start'    : interval_ip,
                'interval_end'      : interval_fp,
                'max_risk_events'   : max_risk_events,
                'highway_time'      : time}
    
    db_highways.insert_one(dic_data)

# rabbitmq-server
# celery -A communication worker --loglevel=info
# brew services start mongodb-community@6.0
# brew services stop mongodb-community@6.0