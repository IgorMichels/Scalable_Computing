import psycopg2
import pandas.io.sql as psql

class Connect:
    def __init__(self, host, database, user, password, query=False):
        # Informa os dados da conexão
        self.user     = user
        self.host     = host
        self.passw    = password
        self.database = database
        self.conn     = psycopg2.connect(host=self.host, database=self.database,
                                         user=self.user, password=self.passw)
        self.cursor = self.conn.cursor()
        if not query: self._start()
        
    # Inicia a conexão, resetando o banco
    def _start(self):
        create = '''
            CREATE SCHEMA IF NOT EXISTS STATS;

            DROP TABLE IF EXISTS STATS.colision;
            CREATE TABLE STATS.colision
            (
                highway INT,
                plate CHAR(3),
                speed FLOAT,
                plate_other_car CHAR(3),
                speed_other_car FLOAT
            );

            DROP TABLE IF EXISTS STATS.overspeed;
            CREATE TABLE STATS.overspeed
            (
                highway INT,
                plate CHAR(3),
                speed FLOAT,
                highway_max_speed INT,
                can_crash INT
            );

            DROP TABLE IF EXISTS STATS.statistics;
            CREATE TABLE STATS.statistics
            (
                cars_count INT,
                highway_count INT,
                overspeed_cars INT,
                possible_crashes INT
            );

            DROP TABLE IF EXISTS STATS.dangerous_driving;
            CREATE TABLE STATS.dangerous_driving
            (
                highway INT,
                plate CHAR(3)
            );

            DROP TABLE IF EXISTS STATS.cars_forbidden;
            CREATE TABLE STATS.cars_forbidden
            (
                highway INT,
                plate CHAR(3)
            );

            DROP TABLE IF EXISTS STATS.historic_info;
            CREATE TABLE STATS.historic_info
            (
                highway INT,
                mean_speed FLOAT,
                accidents INT,
                mean_crossing_time FLOAT
            );

            DROP TABLE IF EXISTS STATS.top100;
            CREATE TABLE STATS.top100
            (
                plate CHAR(3),
                highways_passed INT
            );

            DROP TABLE IF EXISTS STATS.analysis_time;
            CREATE TABLE STATS.analysis_time
            (
                analysis VARCHAR(28),
                time FLOAT,
                update_time TIMESTAMP
            );
        '''
        
        self.cursor.execute(create)
        self.conn.commit()
        
    # Fecha a conexão
    def close(self):
        self.conn.close()

    # Inserção no banco
        '''
          :param df_rows: lista dos registros do dataframe para serem convertidos em tupla
        '''
    # Insere os carros com risco de colisão
    def insert_colision(self, df_rows):
        self.cursor.execute('DELETE FROM STATS.colision')
        for data in df_rows:
            query = '''
                INSERT INTO STATS.colision (
                    highway,
                    plate,
                    speed,
                    plate_other_car,
                    speed_other_car
                )
                VALUES (%s, %s, %s, %s, %s)
            '''
            self.cursor.execute(query, tuple(data))
        self.conn.commit()

    # Insere os carros acima da velocidade
    def insert_overspeed(self, df_rows):
        self.cursor.execute('DELETE FROM STATS.overspeed')
        for data in df_rows:
            query = '''
                INSERT INTO STATS.overspeed (
                    highway,
                    plate,
                    speed,
                    highway_max_speed,
                    can_crash
                )
                VALUES (%s, %s, %s, %s, %s)
            '''
            self.cursor.execute(query, tuple(data))
        self.conn.commit()

    # Insere estatísticas gerais da simulação
    def insert_statistics(self, df_rows):
        self.cursor.execute('DELETE FROM STATS.statistics')
        for data in df_rows:
            query = '''
                INSERT INTO STATS.statistics (
                    cars_count,
                    highway_count,
                    overspeed_cars,
                    possible_crashes
                )
                VALUES (%s, %s, %s, %s)
            '''
            self.cursor.execute(query, tuple(data))
        self.conn.commit()

    # Insere carros com direção perigosa
    def insert_dangerous_driving(self, df_rows):
        self.cursor.execute('DELETE FROM STATS.dangerous_driving')
        for data in df_rows:
            query = '''
                INSERT INTO STATS.dangerous_driving (
                    highway,
                    plate
                )
                VALUES (%s, %s)
            '''
            self.cursor.execute(query, tuple(data))
        self.conn.commit()

    # Insere carros proibidos de circular
    def insert_cars_forbidden(self, df_rows):
        self.cursor.execute('DELETE FROM STATS.cars_forbidden')
        for data in df_rows:
            query = '''
                INSERT INTO STATS.cars_forbidden (
                    highway,
                    plate
                )
                VALUES (%s, %s)
            '''
            self.cursor.execute(query, tuple(data))
        self.conn.commit()

    # Inseri informações históricas das rodovias
    def insert_historic_info(self, df_rows):
        self.cursor.execute('DELETE FROM STATS.historic_info')
        for data in df_rows:
            query = '''
                INSERT INTO STATS.historic_info (
                    highway,
                    mean_speed,
                    accidents,
                    mean_crossing_time
                )
                VALUES (%s, %s, %s, %s)
            '''
            self.cursor.execute(query, tuple(data))
        self.conn.commit()

    # Insere top 100 carros que mais passaram por rodovias
    def insert_top100(self, df_rows):
        self.cursor.execute('DELETE FROM STATS.top100')
        for data in df_rows:
            query = '''
                INSERT INTO STATS.top100 (
                    plate,
                    highways_passed
                )
                VALUES (%s, %s)
            '''
            self.cursor.execute(query, tuple(data))
        self.conn.commit()

    # Insere o tempo para computar cada análise
    def insert_analysis_time(self, analysis, time):
        self.cursor.execute(f'''INSERT INTO STATS.analysis_time (analysis, time, update_time) VALUES ('{analysis}', {time}, NOW())''')
        
    # Consulta no banco
    # Consulta os carros com risco de colisão
    def select_colision(self):
        query = '''
            SELECT
                highway AS "Highway",
                plate AS "Plate 1",
                speed AS "Speed 1",
                plate_other_car AS "Plate 2",
                speed_other_car AS "Speed 2"
            FROM STATS.colision;
        '''
        return psql.read_sql_query(query, self.conn)
    
    # Consulta os carros acima da velocidade
    def select_overspeed(self):
        query = '''
            SELECT
                highway AS "Highway",
                plate AS "Plate",
                speed AS "Speed",
                highway_max_speed AS "Max speed",
                can_crash AS "Can crash"
            FROM STATS.overspeed;
        '''
        return psql.read_sql_query(query, self.conn)
    
    # Consulta estatísticas gerais da simulação
    def select_statistics(self):
        query = '''
            SELECT
                cars_count AS "Cars",
                highway_count AS "Highways",
                overspeed_cars AS "Cars overspeed",
                possible_crashes AS "Possible Crashes"
            FROM STATS.statistics;
        '''
        data = psql.read_sql_query(query, self.conn).T
        data.columns = ['Total']
        return data
    
    # Consulta carros com direção perigosa
    def select_dangerous_driving(self):
        query = '''
            SELECT DISTINCT
                highway AS "Highway",
                plate AS "Plate"
            FROM STATS.dangerous_driving;
        '''
        return psql.read_sql_query(query, self.conn)
    
    # Consulta carros proibidos de circular
    def select_cars_forbidden(self):
        query = '''
            SELECT DISTINCT
                highway AS "Highway",
                plate AS "Plate"
            FROM STATS.cars_forbidden;
        '''
        return psql.read_sql_query(query, self.conn)
    
    # Consulta informações históricas das rodovias
    def select_historic_info(self):
        query = '''
            SELECT
                highway AS "Highway",
                mean_speed AS "Mean speed",
                accidents AS "Accidents",
                mean_crossing_time AS "Mean crossing time"
            FROM STATS.historic_info;
        '''
        return psql.read_sql_query(query, self.conn)
    
    # Consulta top 100 carros que mais passaram por rodovias
    def select_top100(self):
        query = '''
            SELECT
                plate AS "Plate",
                highways_passed AS "Highways"
            FROM STATS.top100;
        '''
        return psql.read_sql_query(query, self.conn)
    
    # Consulta o  tempo para computar cada análise
    def select_analysis_time(self):
        query = '''
            WITH aux AS (
                SELECT
                    analysis,
                    time,
                    ROW_NUMBER() OVER(PARTITION BY analysis ORDER BY update_time DESC) AS n
                FROM stats.analysis_time
            )
            SELECT
                analysis AS "Analysis",
                time AS "Time"
            FROM aux
            WHERE n = 1
        '''
        return psql.read_sql_query(query, self.conn)
    
    # Consulta a média de tempo para computar cada análise
    def select_mean_analysis_time(self):
        query = '''
            SELECT
                analysis AS "Analysis",
                AVG(time) AS "Mean time"
            FROM stats.analysis_time
            GROUP BY analysis;
        '''
        return psql.read_sql_query(query, self.conn)
