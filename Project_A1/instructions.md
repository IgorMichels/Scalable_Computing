## Requisitos
Para conseguir executar o código em Python, é necessária a instalação das seguintes bibliotecas:
* numpy
* intertools
* random
* datetime
* os
* glob

O código fonte em C++ utiliza bibliotecas padrões, sendo desnecessária qualquer instalação.

## Compilação e execução 
Pelo terminal navegue até o diretório onde está o arquivo `main.py` e `main.cpp`. Execute o mock pelo comando
```sh
python main.py
```

Compile o código fonte por
```sh
clang++ -std=c++11 main.cpp -o main
```
Após o término da compilação, você terá o executável `main`. 

Para executar simultaneamente os programas, abra dois terminais. No primeiro terminal, rode o executável. Posteriormente, no segundo terminal, rode o mock em Python.

## Descrição dos arquivos no repositório

* A pasta **mockData** contém os arquivos texto que alimentam o mock, com as informações de modelos de carros no arquivo `cars.txt`, uma lista de nomes no arquivo
`names.txt`, uma lista de sobrenomes no arquivo `surnames.txt` e o arquivo `extraInfoCars.txt` sendo gerado pelo mock, com as informações sobre o carro de cada placa
gerada.

* Os arquivos `Car.py` e `Highway.py` contém as classes que fornecem as informações adicionais do serviço externo para o sistema de monitoramento de rodovia (mock).
* O arquivo `main.py` contém o script que gera os dados para o sistema de monitoramento de rodovia (mock).
* O arquivo `structs.h` é o arquivo que define estruturas que serão usadas no arquivo `main.cpp` para armazenar dados.
* O arquivo `transformers.cpp` é o arquivo que contém os transformadores responsáveis por gerar as análises necessárias: número de de rodovias presentes na simulação,
número total de veículos presentes na simulação, veículos acima do limite de velocidade, informações de cada veículo, propriedades de cada veículo, e veículos que
apresentam risco de colisão.
* O arquivo `API.h` traz a classe externalAPI, que é a API de acesso ao serviço externo de informações adicionais.
* O arquivo `readFiles.h` traz a header responsável por fazer a leitura dos dados gerados pelo mock e extração das informações necessárias.
* O arquivo `main.cpp` traz o script necessário para funcionamento do sistema como um todo, já com todas as prioridades definidas.

## Código com RPC

A compilação e execução do código em C++ permanece a mesma. Para o simulador, devemos trocar o endereço IP do servidor no arquivo `mock_server.py` (linha 27, dentro da pasta `MockData`) e no arquivo `simulate.py` (linha 38). Tendo feitas tais alterações, o servidor está configurado e o cliente estará apto a se comunicar com o mesmo.

Por fim, para executar o programa, basta executar o comando `python main.py -h={a} -t={b} -s={c}`, onde $a$ é o código do primeira rodovia, $b$ é o total de rodovias presente na simulação e $c$ é o número de iterações desejado para cada rodovia. Por default, no caso de $c = 0$ são consideradas 5000 iterações.