import amqp
import asyncio
from celery import Celery

# Configurar o objeto Celery
app = Celery('tasks', broker='amqp://guest:guest@localhost:5672//')

# Definir os tópicos
topics = ['topic1', 'topic2', 'topic3']

# Definir as tarefas de produção
@app.task
def produce(topic, message):
    print(f"Producing message '{message}' for '{topic}'")

    # Publicar a mensagem no tópico
    with app.producer_pool.acquire(block=True) as producer:
        producer.publish(
            {'topic': topic, 'message': message},
            exchange='amq.topic', routing_key=topic
        )

# Definir as tarefas de consumo
@app.task
def consume(topic):
    print(f"Consuming messages from '{topic}'")

    # Consumir as mensagens do tópico
    queue_name = f"queue_{topic}"
    with app.connection() as connection:
        channel = connection.channel()
        channel.exchange_declare(exchange='amq.topic', type='topic', durable=True, auto_delete=False)
        channel.queue_declare(queue=queue_name, durable=True)
        channel.queue_bind(queue=queue_name, exchange='amq.topic', routing_key=topic)

        def callback(channel, method, properties, body):
            process_message(channel, method, properties, body)
            channel.basic_ack(method.delivery_tag)

        async def consume_messages():
            while True:
                print(channel.basic_get(queue=queue_name))
                method, properties, body = await channel.basic_get(queue=queue_name)
                if method is not None:
                    callback(channel, method, properties, body)
                    channel.basic_ack(method.delivery_tag)
                else:
                    await asyncio.sleep(1)  # Esperar antes de verificar novamente as mensagens

        loop = asyncio.get_event_loop()
        loop.create_task(consume_messages())
        loop.run_forever()

# Função de processamento de mensagem
def process_message(channel, method, properties, body):
    topic = method.routing_key
    message = body.decode()
    print(f"Received message '{message}' from '{topic}'")

# # Função principal
# if __name__ == '__main__':
#     # Iniciar os produtores
#     for i in range(2):
#         for topic in topics:
#             produce.delay(topic, f"Message {i+1} for {topic}")

#     # Iniciar os consumidores
#     for i in range(2):
#         consume.delay(topics[i % 3])
