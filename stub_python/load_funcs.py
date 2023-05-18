import mysql.connector


def build_Schemas(cursor):
    query = ("""
    create schema IF NOT EXISTS MOCK_SERVER;
    use MOCK_SERVER;

    drop table if exists API;
    create table if not exists API (
    plate char(7) PRIMARY KEY,
    model varchar(50),
    name varchar(50),
    year int
    );

    drop table if exists MONITOR;
    create table if not exists MONITOR  (
    id INT AUTO_INCREMENT PRIMARY KEY,
    HighWay int,
    MaxSpeedHighway int,
    MaxSpeedCar int,
    plate char(7),
    actualLane int,
    pos int,
    lastPos int,
    penultimatePos int);
    """)
    cursor.execute(query)

def insert(dic, cursor):
    
    dado = dic.copy()
    def put_QUOTE(string):
        return "'" + string + "'" # adiciona aspas em valores do tipo string
    for k in ['plate','model','name']:
        dado[k] = put_QUOTE(dado[k])
        
    keys_MONITOR = ['HighWay',
             'MaxSpeedHighway',
             'MaxSpeedCar',
             'plate',
             'actualLane',
             'pos',
             'lastPos',
             'penultimatePos']

    query_MONITOR = """ insert into MONITOR ("""
    _columns = ''
    _values = ''
    for key in keys_MONITOR: # adiciona chave e valor na tabela MONITOR
        _columns += key + ','
        _values += str(dado[key]) + ','
    _columns = _columns[:-1]
    _values = _values[:-1]
    query_MONITOR += _columns + ') values (' + _values + ');'
    
    cursor.execute(query_MONITOR)
    
    keys_API = ['plate',
             'model',
             'name',
             'year']

    query_API = """ insert ignore into API ("""
    _columns = ''
    _values = ''
    for key in keys_API: # adciona chave e valor na tabela API
        _columns += key + ','
        _values += str(dado[key]) + ','
    _columns = _columns[:-1]
    _values = _values[:-1]
    query_API += _columns + ') values (' + _values + ');'
    
    cursor.execute(query_API)