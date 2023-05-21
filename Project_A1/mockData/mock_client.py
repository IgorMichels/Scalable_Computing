import grpc
import mock_pb2
import mock_pb2_grpc
from datetime import datetime

IP = '192.168.0.45:50051'

class Client:
    def __init__(self, IP):
        self.address = IP

    def send(self, data):
        with grpc.insecure_channel(self.address) as channel:
            stub = mock_pb2_grpc.SendingStub(channel)
            response = stub.Send(mock_pb2.loadRequest(**data))

        print(response.message)

if __name__ == '__main__':
    data = 'highway 100\nMaxSpeed 90\n'
    dado = {'time' : str(datetime.now()),
            'data' : None,
            'plate': 'AAAA000',
            'model': 'Civic',
            'name' : 'Igor',
            'year' : 2020}
    
    client = Client(IP)
    client.send(dado)
