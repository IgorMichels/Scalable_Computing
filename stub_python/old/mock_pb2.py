# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: mock.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\nmock.proto\"\xde\x01\n\x0bloadRequest\x12\x0f\n\x07HighWay\x18\x01 \x01(\x05\x12\x17\n\x0fMaxSpeedHighway\x18\x02 \x01(\x05\x12\x13\n\x0bMaxSpeedCar\x18\x03 \x01(\x05\x12\r\n\x05plate\x18\x04 \x01(\t\x12\x12\n\nactualLane\x18\x05 \x01(\x05\x12\x0b\n\x03pos\x18\x06 \x01(\x05\x12\x0f\n\x07lastPos\x18\x07 \x01(\x05\x12\x16\n\x0epenultimatePos\x18\x08 \x01(\x05\x12\r\n\x05model\x18\t \x01(\t\x12\x0c\n\x04name\x18\n \x01(\t\x12\x0c\n\x04year\x18\x0b \x01(\x05\x12\x0c\n\x04time\x18\x0c \x01(\t\"\x1f\n\x0cloadResponse\x12\x0f\n\x07message\x18\x01 \x01(\t20\n\x07Sending\x12%\n\x04Send\x12\x0c.loadRequest\x1a\r.loadResponse\"\x00\x62\x06proto3')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'mock_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _LOADREQUEST._serialized_start=15
  _LOADREQUEST._serialized_end=237
  _LOADRESPONSE._serialized_start=239
  _LOADRESPONSE._serialized_end=270
  _SENDING._serialized_start=272
  _SENDING._serialized_end=320
# @@protoc_insertion_point(module_scope)
