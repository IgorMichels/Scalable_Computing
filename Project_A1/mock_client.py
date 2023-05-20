import grpc
import mock_pb2
import mock_pb2_grpc
from datetime import datetime

data = 'highway 100\n MaxSpeed 90\n'
dado = {'time' : str(datetime.now()),
        'data' : data,
        'plate': 'AAAA000',
        'model': 'Astra',
        'name' : 'Ana',
        'year' : 2010}
IP = '192.168.0.45:50051'

def run(dado, IP):
    with grpc.insecure_channel(IP) as channel:
        stub = mock_pb2_grpc.SendingStub(channel)
        response = stub.Send(mock_pb2.loadRequest(**dado))
    print(response.message)

if __name__ == '__main__':
    run(dado, IP)

