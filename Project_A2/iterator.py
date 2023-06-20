import os
import sys
from pyspark.sql import SparkSession
from pyspark.sql.window import Window
from pyspark.sql import functions as F

from pprint import pprint
from time import time

sys.path.append('mockData/')
from db_connection import *

T = 50
NUM_MAX_TICKETS = 5

HIGH_ACC = 6
HIGH_SPEED_COEF = .9

def transformer(doc):
    pprint(doc)

def NextDoc(collection):
    doc = collection.find_one()
    ID = doc['_id']
    _ = collection.delete_one({'_id': ID})
    transformer(doc)

def print_df(dataframe, show_schema=False, show_count=False, preview_count=10):
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

spark = SparkSession\
    .builder\
    .appName('mockData') \
    .config('spark.jars.packages', 'org.mongodb.spark:mongo-spark-connector:10.0.2') \
    .config('spark.mongodb.read.connection.uri', 'mongodb://localhost:27017') \
    .config('spark.mongodb.write.connection.uri', 'mongodb://localhost:27017') \
    .getOrCreate()

df_highways = spark \
    .read \
    .format('mongodb') \
    .option('database', 'mock') \
    .option('collection', 'highways') \
    .load() \
    .select('highway', 'highway_extension', 'highway_max_speed', 'car_max_speed',
            'interval_start', 'interval_end', 'max_risk_events')

if __name__ == '__main__':
    os.system('clear')

    t_load_cars = time()
    window_1 = Window.partitionBy('plate', 'highway').orderBy('time')
    window_2 = Window.partitionBy('plate', 'highway').orderBy(F.col('time').desc())
    last_iter_data = spark \
        .read \
        .format('mongodb') \
        .option('database', 'mock') \
        .option('collection', 'cars') \
        .load() \
        .select('plate', 'pos', 'lane', 'highway', 'time') \
        .join(df_highways, ['highway'], 'left') \
        .withColumn('last_pos', F.lag('pos', 1).over(window_1)) \
        .withColumn('penultimate_pos', F.lag('pos', 2).over(window_1)) \
        .withColumn('speed', F.col('pos') - F.col('last_pos')) \
        .withColumn('acceleration', F.col('pos') - 2 * F.col('last_pos') + F.col('penultimate_pos')) \
        .withColumn('process_time', F.lag('time', 2).over(window_1)) \
        .withColumn('row_number', F.row_number().over(window_2)) \
        .filter((F.col('row_number') == 1) &
                (F.col('pos') >= 0) & 
                (F.col('pos') <= F.col('highway_extension')))
    
    window = Window.partitionBy('highway', 'lane').orderBy('pos')
    colision_df = last_iter_data \
        .select('plate', 'highway', 'lane', 'pos', 'last_pos', 'speed', 'acceleration', 'car_max_speed') \
        .filter(F.col('speed').isNotNull()) \
        .fillna(value = 0,subset = ['acceleration']) \
        .withColumn('acceleration', F.when(F.col('speed') != 0,
                                           F.col('acceleration')) \
                                     .otherwise(0)) \
        .withColumn('next_speed', F.when(F.col('pos') - F.col('last_pos') < 0,
                                         F.least(F.lit(0),
                                                 F.greatest(F.col('speed') + F.col('acceleration'),
                                                            - F.col('car_max_speed')))) \
                                   .otherwise(F.least(F.col('car_max_speed'),
                                                      F.greatest(F.col('speed') + F.col('acceleration'),
                                                                 F.lit(0))))) \
        .withColumn('next_pos', F.col('pos') + F.col('next_speed')) \
        .withColumn('plate_other_car', F.lag('plate', 1).over(window)) \
        .withColumn('pos_other_car', F.lag('pos', 1).over(window)) \
        .withColumn('next_pos_other_car', F.lag('next_pos', 1).over(window)) \
        .withColumn('speed_other_car', F.lag('speed', 1).over(window)) \
        .filter((F.col('speed') != 0) & F.col('pos_other_car').isNotNull()) \
        .withColumn('can_crash', F.when(((F.col('pos') > F.col('pos_other_car')) &
                                         (F.col('next_pos') <= F.col('next_pos_other_car'))) |
                                        ((F.col('pos') < F.col('pos_other_car')) &
                                         (F.col('next_pos') >= F.col('next_pos_other_car'))),
                                        1) \
                                   .otherwise(0)) \
        .filter(F.col('can_crash') == 1) \
        .withColumn('speed', F.abs(F.col('speed'))) \
        .withColumn('speed_other_car', F.abs(F.col('speed_other_car'))) \
        .select('highway', 'plate', 'speed', 'plate_other_car', 'speed_other_car')
    
    t_colision = time()

    last_iter_data = last_iter_data \
        .join(colision_df \
        .select('highway', 'plate', 'plate_other_car') \
        .union(colision_df.select('highway', 'plate_other_car', 'plate')),
               ['highway', 'plate'],
               'left')
    
    # veículos acima da velocidade
    overspeed_cars = last_iter_data \
        .filter(F.col('speed') > F.col('highway_max_speed')) \
        .withColumn('can_crash', F.when(F.col('plate_other_car').isNotNull(), 1).otherwise(0)) \
        .select('highway', 'plate', 'speed', 'highway_max_speed', 'can_crash')
    
    t_overspeed_cars = time()

    # estatísticas gerais
    stats = last_iter_data \
        .agg(F.count('plate').alias('cars_count'),
             F.countDistinct('highway').alias('highway_count'),
             F.sum(F.when(F.col('speed') > F.col('highway_max_speed'), 1).otherwise(0)).alias('overspeed_cars'),
             F.count(F.col('plate_other_car')).alias('possible_crashes'))
    
    t_stats = time()

    t_historic_analysis = time()
    window = Window.partitionBy('plate', 'highway').orderBy('time')
    historic = spark \
        .read \
        .format('mongodb') \
        .option('database', 'mock') \
        .option('collection', 'cars') \
        .load() \
        .select('plate', 'pos', 'lane', 'highway', 'time') \
        .join(df_highways, ['highway'], 'left') \
        .withColumn('exiting', ((F.col('pos') < 0) | (F.col('pos') > F.col('highway_extension'))).cast('integer')) \
        .withColumn('times', 1 - F.col('exiting') + F.sum('exiting').over(window)) \
        .withColumn('last_pos', F.lag('pos', 1).over(window)) \
        .withColumn('penultimate_pos', F.lag('pos', 2).over(window)) \
        .withColumn('speed', F.coalesce(F.abs(F.col('pos') - F.col('last_pos')), F.lit(0))) \
        .withColumn('acceleration', F.abs(F.col('pos') - 2 * F.col('last_pos') + F.col('penultimate_pos'))) \
        .withColumn('ticket', ((F.col('speed') > F.col('highway_max_speed')) &
                               (F.lag('speed', 1).over(window) <= F.col('highway_max_speed'))).cast('integer')) \
        .withColumn('tickets_last_T_periods', F.sum('ticket').over(window.rowsBetween(- T, 0))) \
        .withColumn('in_critical_interval', ((F.col('pos') >= F.col('interval_start')) &
                                             (F.col('pos') <= F.col('interval_end'))).cast('integer')) \
        .withColumn('last_lane', F.coalesce(F.lag('lane', 1).over(window), F.col('lane'))) \
        .withColumn('switched_lane', (F.col('last_lane') != F.col('lane')).cast('integer')) \
        .withColumn('times_switching', F.sum('switched_lane').over(window.rowsBetween(- T, 0))) \
        .withColumn('high_speed', (F.col('speed') >= HIGH_SPEED_COEF * F.col('highway_max_speed')).cast('integer')) \
        .withColumn('high_speed_events', F.sum('high_speed').over(window.rowsBetween(- T, 0))) \
        .withColumn('high_acceleration', (F.col('acceleration') >= HIGH_ACC).cast('integer')) \
        .withColumn('high_acceleration_events', F.sum('high_acceleration').over(window.rowsBetween(- T, 0)))

    dangerous_driving = historic \
        .filter(F.col('in_critical_interval') * (F.col('times_switching') + F.col('high_speed_events') + F.col('high_acceleration_events')) >= F.col('max_risk_events')) \
        .select('highway','plate') \
        .distinct()
    
    t_dangerous_driving = time()

    cars_forbidden = historic \
        .filter(F.col('tickets_last_T_periods') >= NUM_MAX_TICKETS) \
        .select('highway', 'plate') \
        .distinct()
    
    t_cars_forbidden = time()
    
    window = Window.partitionBy('highway', 'plate').orderBy('time')
    accidents = historic \
        .filter(F.col('speed') == 0) \
        .withColumn('last_time', F.lag('time', 1).over(window)) \
        .filter(F.col('last_time').isNotNull()) \
        .withColumn('check_time', (F.col('last_time') <= F.lag('time').over(Window.partitionBy('highway').orderBy('time')))) \
        .withColumn('check_plate', (F.col('plate') != F.lag('plate').over(Window.partitionBy('highway').orderBy('time')))) \
        .filter(F.col('check_time') & F.col('check_plate') == 1) \
        .select('highway', 'lane', 'pos', 'plate') \
        .distinct() \
        .groupBy('highway', 'lane', 'pos') \
        .agg((F.count('plate') - F.lit(1)).alias('accidents')) \
        .groupBy('highway') \
        .agg(F.sum('accidents').alias('accidents'))

    window = Window.partitionBy('plate', 'highway', 'times').orderBy('time')
    cross_time = historic \
        .withColumn('cross_time', F.row_number().over(window)) \
        .filter(F.col('exiting') == 1) \
        .groupBy('highway') \
        .agg(F.mean(F.col('cross_time')).alias('mean_crossing_time')) \
        .select('highway', 'mean_crossing_time') \

    window = Window.partitionBy('plate', 'highway', 'times').orderBy('time')
    historic_info = historic \
        .groupBy('highway') \
        .agg(F.mean(F.col('speed')).alias('mean_speed')) \
        .select('highway', 'mean_speed') \
        .join(accidents, ['highway'], 'full') \
        .join(cross_time, ['highway'], 'full') \
        .orderBy(F.col('mean_crossing_time').desc())
    
    t_historic_info = time()

    # top 100 carros com mais rodovias
    top100 = historic \
        .select('plate', 'highway') \
        .dropDuplicates(['plate', 'highway']) \
        .groupBy('plate') \
        .agg(F.countDistinct('highway').alias('highways_passed')) \
        .orderBy(F.col('highways_passed').desc()) \
        .limit(100)
    
    t_top100 = time()
    print_df(colision_df, show_count = True)
    print(f'{t_colision - t_load_cars} segundos')
    print_df(overspeed_cars, show_count = True)
    print(f'{t_overspeed_cars - t_load_cars} segundos')
    print_df(stats, show_count = True)
    print(f'{t_stats - t_load_cars} segundos')
    print_df(dangerous_driving, show_count = True)
    print(f'{t_dangerous_driving - t_historic_analysis} segundos')
    print_df(cars_forbidden, show_count = True)
    print(f'{t_cars_forbidden - t_historic_analysis} segundos')
    print_df(historic_info, show_count = True)
    print(f'{t_historic_info - t_historic_analysis} segundos')
    print_df(top100, show_count = True)
    print(f'{t_top100 - t_historic_analysis} segundos')
    print(f'Total: {time() - t_load_cars} segundos')
