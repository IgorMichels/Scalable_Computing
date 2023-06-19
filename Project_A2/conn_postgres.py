import psycopg2

class Connect:
    def __init__(self, host, database, user, password):
        
        # Informa os dados da conexão
        self.user     = user     # input('Digite seu usuário: ')
        self.host     = host     # input('Digite o host: ')
        self.passw    = password # input('Digite a senha: ')
        self.database = database # input('Digite o nome do database: ')
        self.con      = psycopg2.connect(host=self.host, database=self.database,
                                         user=self.user, password=self.passw)
        self.cursor = self.con.cursor()
        self._start()
        
    # Inicia a conexão
    def _start(self):
        create = """
            create schema IF NOT EXISTS STATS;

            drop table if exists STATS.carros;
            CREATE TABLE STATS.carros
            (
              plate VARCHAR(10),
              speed INT,
              acceleration INT,
              overSpeed CHAR(1),
              canCrash VARCHAR(10)  
            );

            drop table if exists STATS.top100;
            CREATE TABLE STATS.top100
            (
              plate VARCHAR(10),
              number INT  
            );

            drop table if exists STATS.multas;
            CREATE TABLE STATS.multas
            (
              id SERIAL,
              plate VARCHAR(10)  
            );
            """
        _cur = self.con.cursor()
        _cur.execute(create)
        self.con.commit()
        
    # Fecha a conexão
    def close(self):
        self.con.close()
        
    # Inserção no banco ---------------------------------------------------------------------------
        """
          :param data: tupla com os dados do registro a ser inserido na tabela em questão
        """
    def insert_cars(self, data):
        query = """insert into STATS.carros ({plate,speed,acceleration,overSpeed,canCrash}) 
                    values (%s,%s,%s,%s,%s)"""
        self.cursor.execute(query, data)
        self.con.commit()
        
    def insert_top100(self, data):
        self.cursor.execute("DELETE FROM STATS.top100") # Apaga o ranking antigo
        
        # Insere o novo ranking
        query = """insert into STATS.top100 ({plate,number}) 
                    values (%s,%s)"""
        self.cursor.execute(query, data)
        self.con.commit()
        
    def insert_fine(self, data):
        query = """insert into STATS.multas ({plate}) 
                    values (%s)"""
        self.cursor.execute(query, data)
        self.con.commit()
        
    # ---------------------------------------------------------------------------------------------
    
        