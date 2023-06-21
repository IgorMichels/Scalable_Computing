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

print()
cur.execute('SELECT analysis, AVG(time) FROM stats.analysis_time GROUP BY analysis;')
rows = cur.fetchall()
for row in rows: print(row)

cur.close()
conn.close()