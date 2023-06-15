from pprint import pprint
from pymongo import MongoClient


def transformer(doc):
    pprint(doc)

def NextDoc(collection):
    doc = collection.find_one()
    ID = doc['_id']
    _ = collection.delete_one({"_id": ID})
    transformer(doc)