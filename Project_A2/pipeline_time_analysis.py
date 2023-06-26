from conn_postgres import Connect
from glob import glob

import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

highways = 0
instances = 0
for arg in sys.argv:
    if '-t' in arg: highways = arg.split('=')[-1]
    elif '-i' in arg: instances = arg.split('=')[-1]

if highways > 0:
    conn = Connect('localhost', '', '', '', query=True)
    times = conn.select_mean_analysis_time()
    times['Highways'] = highways
    times['Instances'] = instances
    times.to_csv(f'pipeline_times/{highways}_{instances}.csv')
else:
    files = glob('pipeline_times/*')
    df = pd.DataFrame()
    for file in files: df = pd.concat([df, pd.read_csv(file)])
    df = df[['Analysis', 'Mean time', 'Highways', 'Instances']] \
        .sort_values(['Highways', 'Analysis'], ignore_index = True)
    
    analysis = np.unique(df['Analysis'].values)
    for a in analysis:
        aux = df[df['Analysis'] == a]
        plt.plot(aux['Highways'], aux['Mean time'])
        plt.title(f'Tempo médio\n{a}')
        plt.xlabel('Rodovias')
        plt.ylabel('Tempo de execução da análise (s)')
        plt.savefig(f'images/{a}.png')
        plt.clf()
