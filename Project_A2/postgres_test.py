from conn_postgres import Connect

conn = Connect('localhost', '', '', '', query=True)

def print_query(data):
    for row in data: print(row)
    print()

print_query(conn.select_colision())
print_query(conn.select_overspeed())
print_query(conn.select_statistics())
print_query(conn.select_dangerous_driving())
print_query(conn.select_cars_forbidden())
print_query(conn.select_historic_info())
print_query(conn.select_top100())