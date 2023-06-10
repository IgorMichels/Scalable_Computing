from tasks import *


# Definir os tópicos
topics = ['topic1', 'topic2', 'topic3']

# Função principal
if __name__ == '__main__':
    # Iniciar os produtores
    for i in range(2):
        for topic in topics:
            produce.delay(topic, f"Message {i+1} for {topic}")

    # Iniciar os consumidores
    for i in range(2):
        consume.delay(topics[i % 3])