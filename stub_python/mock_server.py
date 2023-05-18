import mysql.connector
from load_funcs import *
from concurrent import futures
import grpc
import mock_pb2
import mock_pb2_grpc


class Sending(mock_pb2_grpc.SendingServicer):
    def __init__(self):
        self.cnx = mysql.connector.connect(user='root', password='B70B4A##', host='127.0.0.1', database='MOCK_SERVER')
        self.cursor = self.cnx.cursor()
        self.counter = 0
    def Send(self, request, context):
        dado = {'HighWay'        : request.HighWay,
                'MaxSpeedHighway': request.MaxSpeedHighway,
                'MaxSpeedCar'    : request.MaxSpeedCar,
                'plate'          : request.plate,
                'actualLane'     : request.actualLane,
                'pos'            : request.pos,
                'lastPos'        : request.lastPos,
                'penultimatePos' : request.penultimatePos,
                'model'          : request.model,
                'name'           : request.name,
                'year'           : request.year}
        insert(dado, self.cursor)
        self.counter += 1
        if (self.counter % 10) == 0:
            self.cnx.commit()
        return mock_pb2.loadResponse(message='Sended')

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    mock_pb2_grpc.add_SendingServicer_to_server(Sending(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    serve()
