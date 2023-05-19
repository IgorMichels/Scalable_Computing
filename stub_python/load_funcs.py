import mysql.connector
from getpass import getpass

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
    time varchar(50) PRIMARY KEY,
    data TEXT);
    """)
    cursor.execute(query, multi = True)

def insert(dic, cursor):
    dado = dic.copy()
    put_QUOTE = lambda string : "'" + string + "'" # adiciona aspas em valores do tipo string
    for k in ['plate','model','name','time','data']:
        dado[k] = put_QUOTE(dado[k])
        
    keys_MONITOR = ['time','data']

    query_MONITOR = """ insert ignore into MONITOR ("""
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

# Esse earquivo só deve ser execurado caso o schema ainda não tenha sido criado
if __name__ == '__main__':
    print('Insira suas credenciais do mysql:')
    user = input('usuário: ')
    password = getpass('senha: ')
    host = input('host: ')
    database = input('database: ')
    cnx = mysql.connector.connect(user = user,
                                  password = password,
                                  host = host,
                                  database = database)
    
    cursor = cnx.cursor()
    build_Schemas(cursor)
    cursor.close()
    cnx.close()