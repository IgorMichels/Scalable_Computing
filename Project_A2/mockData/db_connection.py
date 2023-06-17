from pymongo import MongoClient

# Conectando ao servidor do MongoDB
client = MongoClient('mongodb://localhost:27017')

# Selecionando o banco de dados
db = client['mock']

# Selecionando a coleção
db_cars = db['cars']
db_highways = db['highways']