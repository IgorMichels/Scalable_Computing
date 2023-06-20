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

            drop table if exists STATS.colision;
                CREATE TABLE STATS.colision
                (
                highway INT,
                plate VARCHAR(10),
                speed FLOAT,
                plate_other_car VARCHAR(10),
                speed_other_car FLOAT
                );

                drop table if exists STATS.overspeed;
                CREATE TABLE STATS.overspeed
                (
                highway INT,
                plate VARCHAR(10),
                speed FLOAT,
                highway_max_speed INT,
                can_crash INT
                );

                drop table if exists STATS.statistics;
                CREATE TABLE STATS.statistics
                (
                cars_count INT,
                highway_count INT,
                overspeed_cars INT,
                possible_crashes INT
                );

                drop table if exists STATS.dangerous_driving;
                CREATE TABLE STATS.dangerous_driving
                (
                highway INT,
                plate VARCHAR(10)
                );

                drop table if exists STATS.cars_forbidden;
                CREATE TABLE STATS.cars_forbidden
                (
                highway INT,
                plate VARCHAR(10)
                );

                drop table if exists STATS.historic_info;
                CREATE TABLE STATS.historic_info
                (
                highway INT,
                mean_speed FLOAT,
                accidents INT,
                mean_crossing_time FLOAT
                );

                drop table if exists STATS.top100;
                CREATE TABLE STATS.top100
                (
                plate VARCHAR(10),
                highways_passed INT
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
          :param df_rows: lista dos registros do dataframe para serem convertidos em tupla
        """

    def insert_colision(self, df_rows):

        for data in df_rows:
            query = """insert into STATS.colision (
                        highway,
                        plate,
                        speed,
                        plate_other_car,
                        speed_other_car) 
                        values (%s,%s,%s,%s,%s)"""
            self.cursor.execute(query, tuple(data))
        self.con.commit()


    def insert_overspeed(self, df_rows):

        for data in df_rows:
            query = """insert into STATS.overspeed (
                        highway,
                        plate,
                        speed,
                        highway_max_speed,
                        can_crash) 
                        values (%s,%s,%s,%s,%s)"""
            self.cursor.execute(query, tuple(data))
        self.con.commit()

    def insert_statistics(self, df_rows):
        self.cursor.execute("DELETE FROM STATS.statistics") # Apaga as estatísticas antigas
        
        for data in df_rows:
            query = """insert into STATS.statistics (
                        cars_count,
                        highway_count,
                        overspeed_cars,
                        possible_crashes) 
                        values (%s,%s,%s,%s)"""
            self.cursor.execute(query, tuple(data))
        self.con.commit()

    def insert_dangerous_driving(self, df_rows):

        for data in df_rows:
            query = """insert into STATS.dangerous_driving (
                        highway,
                        plate) 
                        values (%s,%s)"""
            self.cursor.execute(query, tuple(data))
        self.con.commit()

    def insert_cars_forbidden(self, df_rows):

        for data in df_rows:
            query = """insert into STATS.cars_forbidden (
                        highway,
                        plate) 
                        values (%s,%s)"""
            self.cursor.execute(query, tuple(data))
        self.con.commit()

    def insert_historic_info(self, df_rows):

        for data in df_rows:
            query = """insert into STATS.historic_info (
                        highway,
                        mean_speed,
                        accidents,
                        mean_crossing_time) 
                        values (%s,%s,%s,%s)"""
            self.cursor.execute(query, tuple(data))
        self.con.commit()

    def insert_top100(self, df_rows):
        self.cursor.execute("DELETE FROM STATS.top100") # Apaga o ranking antigo
        
        for data in df_rows:
            query = """insert into STATS.top100 (
                        plate,
                        highways_passed) 
                        values (%s,%s)"""
            self.cursor.execute(query, tuple(data))
        self.con.commit()
        
    # ---------------------------------------------------------------------------------------------
    
        