import sys
sys.path.append('mockData/')

from db_connection import *
from conn_postgres import Connect

if __name__ == '__main__':
    db_cars.delete_many({})
    db_highways.delete_many({})
    
    conn = Connect('localhost', '', '', '', query=False)
    conn.close()