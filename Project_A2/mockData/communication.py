from celery import Celery
from pprint import pprint
from pymongo import MongoClient

# Configuração do Celery
app = Celery('communication', broker = 'amqp://guest@localhost//', backend = 'rpc://')

# Conectando ao servidor do MongoDB
client = MongoClient("mongodb://localhost:27017")

# Selecionando o banco de dados
db = client["mock"]

# Selecionando a coleção
collection = db["cars"]

@app.task
def SendInfo(plate, pos, lane, highway, time):
    print(f'Carro {plate} estava na pista {lane} e posição {pos} da rodovia {highway} às {time}.')
    dic_data = {"plate": plate,
                "pos": pos,
                "lane":lane,
                "highway":highway,
                "time": time}
    collection.insert_one(dic_data)

# rabbitmq-server
# celery -A communication worker --loglevel=info