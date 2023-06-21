import psycopg2

class Connect:
    def __init__(self, host, database, user, password, query=False):
        
        # Informa os dados da conexão
        self.user     = user     # input('Digite seu usuário: ')
        self.host     = host     # input('Digite o host: ')
        self.passw    = password # input('Digite a senha: ')
        self.database = database # input('Digite o nome do database: ')
        self.conn     = psycopg2.connect(host=self.host, database=self.database,
                                         user=self.user, password=self.passw)
        self.cursor = self.conn.cursor()
        if not query: self._start()
        
    # Inicia a conexão
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

    # Inserção no banco ---------------------------------------------------------------------------
        '''
          :param df_rows: lista dos registros do dataframe para serem convertidos em tupla
        '''

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

    def insert_analysis_time(self, analysis, time):
        self.cursor.execute(f'''INSERT INTO STATS.analysis_time (analysis, time, update_time) VALUES ('{analysis}', {time}, NOW())''')
        
    # ---------------------------------------------------------------------------------------------
    
    def select_colision(self):
        self.cursor.execute('SELECT * FROM STATS.colision;')
        return self.cursor.fetchall()
    
    def select_overspeed(self):
        self.cursor.execute('SELECT * FROM STATS.overspeed;')
        return self.cursor.fetchall()
    
    def select_statistics(self):
        self.cursor.execute('SELECT * FROM STATS.statistics;')
        return self.cursor.fetchall()
    
    def select_dangerous_driving(self):
        self.cursor.execute('SELECT * FROM STATS.dangerous_driving;')
        return self.cursor.fetchall()
    
    def select_cars_forbidden(self):
        self.cursor.execute('SELECT * FROM STATS.cars_forbidden;')
        return self.cursor.fetchall()
    
    def select_historic_info(self):
        self.cursor.execute('SELECT * FROM STATS.historic_info;')
        return self.cursor.fetchall()
    
    def select_top100(self):
        self.cursor.execute('SELECT * FROM STATS.top100;')
        return self.cursor.fetchall()
