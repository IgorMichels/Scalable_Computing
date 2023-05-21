from concurrent import futures
import grpc
import mock_pb2
import mock_pb2_grpc

def insert(dic):
    if dic['data'] != '':
        with open(f"{dic['time']}.txt", 'w') as f:
            f.writelines(dic['data'])
    else:
        with open('extraCarInfos.txt') as f:
            f.write(f"{dic['plate']},{dic['name']},{dic['model']},{dic['year']}\n")

class Server(mock_pb2_grpc.SendingServicer):
    def __init__(self):
        self.counter = 0

    def Send(self, request, context):
        insert(request)
        self.counter += 1
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
