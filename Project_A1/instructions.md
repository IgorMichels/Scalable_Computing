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

