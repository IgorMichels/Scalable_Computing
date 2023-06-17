import sys
from pyspark.sql import SparkSession

from pprint import pprint

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

    df_cars = spark \
        .read \
        .format('mongodb') \
        .option('database', 'mock') \
        .option('collection', 'cars') \
        .load() \
        .select('plate', 'pos', 'lane', 'highway', 'time')

    df_highways = spark \
        .read \
        .format('mongodb') \
        .option('database', 'mock') \
        .option('collection', 'highways') \
        .load() \
        .select('highway', 'highway_extension', 'highway_max_speed')

    data = orders_with_info_df = df_cars.join(df_highways, ['highway'], 'left')
    data.coalesce(1).write.csv('data_df')
    print_df(data, show_count = True)
    