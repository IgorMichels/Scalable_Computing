from conn_postgres import Connect
import sys

highways = 0
instances = 0
for arg in sys.argv:
    if '-t' in arg: highways = arg.split('=')[-1]
    elif '-i' in arg: instances = arg.split('=')[-1]

conn = Connect('localhost', '', '', '', query=True)

times = conn.select_mean_analysis_time()
times['Highways'] = highways
times['Instances'] = instances
times.to_csv(f'pipeline_times/{highways}_{instances}.csv')