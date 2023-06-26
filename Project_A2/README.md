# Pipeline de Processamento de dados

### Instruções

Inicialmente é necessário ter instalada a versão 6.0.6 do MongoDB, a versão 14.8 do PostgreSQL, a versão 3.12.0 do RabbitMQ e a versão 5.3.0 do Celery. Por simplicidade de implementação e executação, optou-se por não utilizar credenciais no PostgreSQL, embora isso seja uma péssima prática para projetos reais. De modo análogo, apenas configuramos um banco com duas coleções dentro do MongoDB.

Para executar o programa de simulação, no ambiente local, é necessário dar início aos dois bancos de dados, por meio dos comandos
`
brew services start mongodb-community@6.0
brew services start postgresql@14
`

de modo análogo, ao término da sessão, devemos encerrar os bancos por meio dos comandos
`
brew services stop mongodb-community@6.0
brew services stop postgresql@14
`

Tendo os bancos de dados prontos, podemos ativar o servidor do RabbitMQ por meio do comando
`
rabbitmq-server
`

e, feito isso, executar, dentro do diretório `mockData`, o comando
`
celery -A communication worker --loglevel=info
`

Finalizados tais passos, temos o ambiente configurado para a execução do simulador e do pipeline de tratamento dos dados. A primeira etapa, nesse sentido, é a execução dos simuladores. Para tanto, dentro desse diretório, execute o comando
`
python main.py -s=SIMS -t=TOTAL -h=HIGHWAY -ri=BOOL1 -rf=BOOL2
`

onde `SIMS` é o número de iterações de cada rodovia (por default, 20), `TOTAL` é o número de rodovias desejado (por default, 1), `HIGHWAY` é o código inicial da rodovia (inteiro, default 100) e `BOOL1` e `BOOL2` são booleanos que indicam, respectivamente, se é desejado limpar o conteúdo do banco instanciado no MongoDB antes e depois de realizar as simulações. Por padrão, definimos `BOOL1=True` e `BOOL2=False`.

Estando com a simulação rodando em um terminal basta executar, em outro terminal, o comando
`
python pipeline.py
`

o qual irá reinstanciar um banco no PostgreSQL e iniciar o processamento dos dados. Para observar os dados no dashboard, basta executar
`
streamlit run dashboard.py
`

### Arquivos

Com o intuito de dar um panorâma geral acerca dos arquivos temos, dentro da pasta `mockData`, as definições dos objetos `Car` e `Highway`, cada um dentro dos respectivos arquivos, bem como a instanciação do banco MongoDB (arquivo `db_connection.py`) e as tarefas que realizam a comunicação dos carros com o broker, levando os dados para serem populados no banco (arquivo `communication.py`).

No diretório atual possuímos, dentro do arquivo `reset_db.py`, alguns comandos com o intuito de limpar os dois bancos de dados, evitando que sejam deixados resíduos entre uma execução e outra e, principalmente, ao término da sessão. O arquivo `conn_postgres.py` possuí a definição de uma classe que é utilizada para a população do banco PostgreSQL com as análises e, além disso, com as queries utilizadas para a realização do Dashboard. O arquivo `pipeline_time_analysis.py` possuí um simples código para salvar o tempo de processamento, de dentro do PostgreSQL, para a realização do relatório final. Por fim, os arquivos `main.py`, `pipeline.py` e `dashboard.py` foram explorados na seção anterior e são responsáveis por, respectivamente, executar as simulações, o processamento dos dados e a visualização dos mesmos.
