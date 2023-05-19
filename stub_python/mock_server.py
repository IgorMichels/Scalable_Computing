import mysql.connector
from load_funcs import *
from concurrent import futures
import grpc
import mock_pb2
import mock_pb2_grpc

class Sending(mock_pb2_grpc.SendingServicer):
    def __init__(self):
        print('Insira suas credenciais do mysql:')
        user = input('usuário: ')
        password = getpass('senha: ')
        host = input('host: ')
        database = input('database: ')
        self.cnx = mysql.connector.connect(user = user,
                                           password = password,
                                           host = host,
                                           database = database)
        
        self.cursor = self.cnx.cursor()
        self.counter = 0
    def Send(self, request, context):
        dado = {'time'           : request.time,
                'data'           : request.data,
                'plate'          : request.plate,
                'model'          : request.model,
                'name'           : request.name,
                'year'           : request.year}
        insert(dado, self.cursor)
        self.counter += 1
        self.cnx.commit()
        return mock_pb2.loadResponse(message = 'Received')

def serve():
    print('Iniciando servidor')
    server = grpc.server(futures.ThreadPoolExecutor(max_workers = 10))
    mock_pb2_grpc.add_SendingServicer_to_server(Sending(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    print('Servidor iniciado')
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
