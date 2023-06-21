import psycopg2

conn = psycopg2.connect(database = '', 
                        user = '', 
                        host = 'localhost',
                        password = '',
                        port = 5432
                       )
    
cur = conn.cursor()
cur.execute('SELECT * FROM stats.analysis_time;')
rows = cur.fetchall()
for row in rows: print(row)

cur.close()
conn.close()