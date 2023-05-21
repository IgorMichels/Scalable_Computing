import mysql.connector
from getpass import getpass

def build_Schemas(cursor):
    query = ("""
    CREATE SCHEMA IF NOT EXISTS MOCK_SERVER;
    USE MOCK_SERVER;

    DROP TABLE IF EXISTS API;
    CREATE TABLE IF NOT EXISTS API (
    plate CHAR(7) PRIMARY KEY,
    model VARCHAR(50),
    name VARCHAR(100),
    year INT
    );

    DROP TABLE IF EXISTS MONITOR;
    CREATE TABLE IF NOT EXISTS MONITOR  (
    time VARCHAR(50) PRIMARY KEY,
    data MEDIUMTEXT);
    """)

    cursor.execute(query, multi = True)

def insert(dic, cursor):
    dado = dic.copy()
    put_QUOTE = lambda data : "'" + data + "'" if type(data) == str else str(data)
    if dado['data'] != '':
        keys = ['time', 'data']
        query = """ INSERT IGNORE INTO MONITOR ("""
        columns = ''
        values = ''
        for key in keys:
            columns += key + ','
            values += put_QUOTE(dado[key]) + ','
        
        columns = columns[:-1]
        values = values[:-1]
        query += columns + ') VALUES (' + values + ');'
        cursor.execute(query)
    else:
        keys = ['plate', 'model', 'name', 'year']
        query = """ INSERT IGNORE INTO API ("""
        columns = ''
        values = ''
        for key in keys: # adciona chave e valor na tabela API
            columns += key + ','
            values += put_QUOTE(dado[key]) + ','
        
        columns = columns[:-1]
        values = values[:-1]
        query += columns + ') VALUES (' + values + ');'
        
        cursor.execute(query)

# Esse earquivo só deve ser executado caso o schema ainda não tenha sido criado
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