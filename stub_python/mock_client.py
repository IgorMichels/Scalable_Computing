import grpc
import mock_pb2
import mock_pb2_grpc

def run():
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = mock_pb2_grpc.SendingStub(channel)
        dado = {'HighWay'        : 100,
                'MaxSpeedHighway': 100,
                'MaxSpeedCar'    : 120,
                'plate'          : 'AAAA000',
                'actualLane'     : 0,
                'pos'            : 10,
                'lastPos'        : 7,
                'penultimatePos' : 5,
                'model'          : 'Astra',
                'name'           : 'Davi',
                'year'           : 2010}
        response = stub.Send(mock_pb2.loadRequest(**dado))
    print(response.message)

if __name__ == '__main__':
    run()

