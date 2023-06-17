import sys
from pyspark.sql import SparkSession
from pyspark.sql.window import Window
from pyspark.sql import functions as F

from pprint import pprint
from time import time

sys.path.append('mockData/')
from db_connection import *

def transformer(doc):
    pprint(doc)

def NextDoc(collection):
    doc = collection.find_one()
    ID = doc['_id']
    _ = collection.delete_one({'_id': ID})
    transformer(doc)

def print_df(dataframe, show_schema=False, show_count=False, preview_count=20):
    """
    Print dataframe with adittional information

    :param dataframe: the dataframe to print
    :param show_schema: True to print the schema
    :param show_count: True to print the number of rows
    :param preview_count: number of rows to preview
    """
    if show_schema: dataframe.printSchema()
    if show_count: print(f'{dataframe.count()} rows\n')
    dataframe.show(n=preview_count)

if __name__ == '__main__':
    spark = SparkSession\
        .builder\
        .appName('mockData') \
        .config('spark.jars.packages', 'org.mongodb.spark:mongo-spark-connector:10.0.2') \
        .config('spark.mongodb.read.connection.uri', 'mongodb://localhost:27017') \
        .config('spark.mongodb.write.connection.uri', 'mongodb://localhost:27017') \
        .getOrCreate()

    t1 = time()
    df_highways = spark \
        .read \
        .format('mongodb') \
        .option('database', 'mock') \
        .option('collection', 'highways') \
        .load() \
        .select('highway', 'highway_extension', 'highway_max_speed', 'car_max_speed')

    print_df(df_highways, show_count = True)

    t2 = time()
    df_cars = spark \
        .read \
        .format('mongodb') \
        .option('database', 'mock') \
        .option('collection', 'cars') \
        .load() \
        .select('plate', 'pos', 'lane', 'highway', 'time')

    t3 = time()
    window = Window.partitionBy('plate', 'highway').orderBy('time')
    data = df_cars.join(df_highways, ['highway'], 'left')
    data = data.withColumn('last_pos', F.lag('pos', 1).over(window))
    data = data.withColumn('penultimate_pos', F.lag('pos', 2).over(window))
    data = data.withColumn('speed', data['pos'] - data['last_pos'])
    data = data.withColumn('acceleration', data['pos'] - 2 * data['last_pos'] + data['penultimate_pos'])
    data = data.withColumn('process_time', F.coalesce(F.lag('time', 2).over(window),
                                                      F.lag('time', 1).over(window),
                                                      data['time']))
    
    # colisão de primeira ordem
    window = Window.partitionBy('plate', 'highway').orderBy(F.col('time').desc())
    colision_df = data.withColumn('row_number', F.row_number() \
                                                    .over(window)) \
                                                    .filter((F.col('row_number') == 1) &
                                                            (F.col('pos') >= 0) & 
                                                            (F.col('pos') <= F.col('highway_extension')))
    
    colision_df = colision_df.select('plate', 'highway', 'lane', 'pos', 'speed', 'acceleration', 'car_max_speed')
    
    # data = data.join(data, ['highway', 'lane'], 'left')
    


    tf = time()
    print_df(colision_df, show_count = True)
    print(f'{tf - t2} segundos')
    print(f'{tf - t1} segundos')