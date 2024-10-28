import numpy as np
import struct
import codecs
import os
import os.path as op
from ctypes import *

def main():
  dir_path = os.path.dirname(os.path.realpath(__file__))
  so_file = op.join(dir_path, "smart_s_p.so")
  packedfilter = CDLL(so_file)
  P = POINTER(c_ubyte)

  #last iteration?
  packedfilter.search_s_p.argtypes = [POINTER(c_ubyte), c_int, POINTER(c_ubyte), c_int]
  string_to_find = "4C 8B 51 18 4D 8B D8 48 8B 05 ?? ?? ?? ?? 4D 8D 42 18 49 39 40 08 75 1C 48 8B 05 ?? ?? ?? ?? 49 39 00 75 10 0F 10 02 4D 89 5A 28 45 88 4A 30 41 0F 11 00 C3 48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??"
  hex_string = string_to_find.replace(" ", "")
  patt_len = len(hex_string)
  b_string = hex_string.encode('utf-8')
  P = (c_ubyte * patt_len).from_buffer_copy(bytearray(b_string))
  filename = 'NMS.exe'
  file_path = op.join(dir_path,filename)
  file_stream = open(file_path, 'rb')
  file = file_stream.read()
  file_len = os.path.getsize(file_path)
  T = POINTER(c_ubyte * file_len)
  T = (c_ubyte * file_len).from_buffer(bytearray(file))
  
  print(file_len, patt_len)
  result = packedfilter.search_s_p(P, patt_len, T, file_len)
  print(result)

main()