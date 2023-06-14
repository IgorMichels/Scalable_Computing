from celery import Celery

# Configuração do Celery
app = Celery('tasks', broker='amqp://guest@localhost//', backend='rpc://')

# Definição da tarefa
@app.task
def Print(plate):
    print(f'Processando {plate}')

@app.task
def Speed(pos, plate):
    print(f'Carro {plate} está na posição {pos}')

@app.task
def Write(plate):
    f = open('carros.txt', 'a')
    f.write(plate + '\n')
    f.close()

@app.task
def SendInfo(plate, pos, lane, highway, time):
    print(f'Carro {plate} estava na pista {lane} e posição {pos} da rodovia {highway} às {time}.')

## celery -A tasks worker --loglevel=info