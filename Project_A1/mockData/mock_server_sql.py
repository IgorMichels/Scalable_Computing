import mysql.connector
from sql_functions import *
from concurrent import futures
import grpc
import mock_pb2
import mock_pb2_grpc

class Server(mock_pb2_grpc.SendingServicer):
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
        insert(request, self.cursor)
        self.counter += 1
        self.cnx.commit()
        return mock_pb2.loadResponse(message = 'Received')

def serve():
    print('Iniciando servidor')
    server = grpc.server(futures.ThreadPoolExecutor(max_workers = 10))
    mock_pb2_grpc.add_SendingServicer_to_server(Server(), server)
    port = server.add_insecure_port('192.168.0.45:50051')
    server.start()
    print(f'Servidor iniciado na porta {port}')
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
