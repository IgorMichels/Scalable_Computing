import mysql.connector
from load_funcs import *

cnx = mysql.connector.connect(user='root', password='B70B4A##',
                              host='127.0.0.1',
                              database='MOCK_SERVER')
cursor = cnx.cursor()


build_Schemas(cursor)
cursor.close()
cnx.close()
# cnx.commit()


