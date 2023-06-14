from celery import Celery

# Configuração do Celery
app = Celery('communication', broker = 'amqp://guest@localhost//', backend = 'rpc://')

@app.task
def SendInfo(plate, pos, lane, highway, time):
    print(f'Carro {plate} estava na pista {lane} e posição {pos} da rodovia {highway} às {time}.')
    return plate, pos, lane, highway, time

# rabbitmq-server
# celery -A communication worker --loglevel=info