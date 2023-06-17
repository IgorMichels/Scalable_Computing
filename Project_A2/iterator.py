import sys

from pprint import pprint

sys.path.append('mockData/')
from db_connection import *

def transformer(doc):
    pprint(doc)

def NextDoc(collection):
    doc = collection.find_one()
    ID = doc['_id']
    _ = collection.delete_one({"_id": ID})
    transformer(doc)

if __name__ == '__main__':
    NextDoc(db_highways)