#check, load, and write all files with python
#only use C to process data, except filtering the index
    
""" 
decode hex and generate wildcard index <- no need to reinvent the wheel
generate all hex sequences <- can be done in python
check for function ranges file <- can be done in python
generate function ranges <- could be done in python
build/load index
index filtered in python
search with index or filtered index """

from typing import Optional
import numpy as np
import struct
import codecs
import os
import os.path as op
from ctypes import *
from collections import Counter
from filter import filter_index_by_ranges

class WILDCARDS(Structure):
    _fields_ = [('decoded_hex', POINTER(c_ubyte)),
                ('wildcard_index', POINTER(c_uint8)*100)]
    
def get_pdata(path, function_filename, reject_filename):
  pdata = []
  reject = []
  try:
      with open(path, 'rb') as f:
          data = f.read()
          # First, find the start and ends of the .rdata section.
          pdata_hdr = data.find(b'.pdata')
          if(pdata_hdr != -1):
            pdata_size, _, _, pdata_offset = struct.unpack_from(
                '<IIII',
                data,
                offset=pdata_hdr + 8
            )
            entries =  pdata_size // 0xC
            for i in range(entries):
                func_start, func_end = struct.unpack_from('<II', data, offset=pdata_offset + 0xC * i)
                pdata.append((func_start, func_end, func_end - func_start))

            with open(function_filename, "w") as f:
                f.write("Start,End,Size;\n")
                for func in pdata:
                    if func[0] % 8 == 0:
                        f.write(f"0x{func[0]:X},0x{func[1]:X},0x{func[2]:X},\n")
                    else:
                        reject.append(func)

            with open(reject_filename, "w") as f:
                f.write("Start,End,Size;\n")
                for func in reject:
                    f.write(f"0x{func[0]:X},0x{func[1]:X},0x{func[2]:X},\n")
            
            return 1
          else:
              return 0
  except IOError as e:
      return 0

def c_array_to_list():
   pass

def main():
  #load in C functions
  filtered_index_filename = "filtered_index.json"
  index_filename = "index.txt"
  function_filename = "functions.txt"
  reject_filename = "rejects.txt"
  to_search_filename = 'NMS.exe'
  patterns_filename = 'patterns.txt'
  output_dir = "output"
  input_dir = "input"
  c_filename = "utils.so"
  dir_path = os.path.dirname(os.path.realpath(__file__))
  input_dir = op.join(dir_path, input_dir)
  output_dir = op.join(dir_path, output_dir)
  so_file = op.join(dir_path, c_filename)
  c_utils = CDLL(so_file)  
  result = None

  #specify argtypes for some reason
  """
  unsigned char* query_array,
  int query_len,
  unsigned char* text,
  int text_len,
  uint8_t* wildcard_index[100],
  int* file_index[255]"""
  c_utils.index_search.argtypes = [POINTER(c_ubyte), c_int, POINTER(c_ubyte), c_int, POINTER(POINTER(c_uint8))*100, POINTER(POINTER(c_int))*255]
  c_utils.index_search.restype = c_int
  """ 
  unsigned char* st, 
  uint8_t* wildcard_index[100] """
  c_utils.decode_hex.argtypes = [POINTER(c_ubyte), POINTER(POINTER(c_uint8))*100]
  c_utils.decode_hex.restype = c_void_p

  c_utils.freeptr.argtypes = c_void_p
  c_utils.freeptr.restype = None

  """
  unsigned char* query_array,
  int query_len,
  unsigned char* text,
  int text_len"""
  c_utils.build_index.argtypes = [POINTER(c_ubyte), c_int, POINTER(c_ubyte), c_int]
  c_utils.build_index.restype = POINTER(POINTER(c_int))

  #generate all hex sequences
  py_list = list(range(0,255))
  all_hex = (c_uint8  * len(py_list))(*py_list)

  #Load file to search and convert to ctype unsigned char*
  file_path = op.join(input_dir,to_search_filename)
  file_len = os.path.getsize(file_path)
  T = POINTER(c_ubyte * file_len)
  T = (c_ubyte * file_len).from_buffer(bytearray(open(file_path, 'rb').read()))

  #generate index
  index = POINTER(POINTER(c_int))*255
  try:
    with open(op.join(output_dir, index_filename), 'r') as f:
      size = 0
      hex = 0
      value = 0
      count = 0
      for line in f:
         if line.startswith(']'):
            info = f.next()
            hex = info[0]
            size = info[1]
            index[hex] = POINTER(c_int) * size
            index[hex][count] = size
            count += 1
         else:
            index[hex][count] = int(line[0])      
  except:
    with open(op.join(output_dir, index_filename), 'a') as f:
       """build_index(
        unsigned char* query_array,
        int query_len,
        unsigned char* text,
        int text_len)"""
       index = c_utils.build_index(all_hex, len(py_list), T, file_len)
       array_end = ']'
       delim = ', '
       nl = '\n'
       f.write(f'{array_end}{delim}{nl}')
       for i in range(256):
          f.write(f'{i}{delim}')
          for l in range(index[i][0]):
             f.write(f'{index[i][l]}{delim}{nl}')
          f.write(f'{array_end}{delim}{nl}')

  #check if pdata
  filtered_index_dict = {}
  filtered_index = []
  if(get_pdata(op.join(input_dir, to_search_filename), op.join(output_dir,function_filename), op.join(output_dir,reject_filename))):
    #filter index by function ranges
    filtered_index_dict = filter_index_by_ranges(op.join(output_dir, index_filename), op.join(output_dir, function_filename), op.join(output_dir, filtered_index_filename))
    for key in filtered_index_dict:
       filtered_index[key] = list(filtered_index_dict[key]["size"]) + list(filtered_index_dict[key]["compliant"])
    index = cast(filtered_index, POINTER(POINTER(c_int))*255)
  
  #Decode string to find and generate wildcard index  
  #string_to_find = "4C 8B 51 18 4D 8B D8 48 8B 05 ?? ?? ?? ?? 4D 8D 42 18 49 39 40 08 75 1C 48 8B 05 ?? ?? ?? ?? 49 39 00 75 10 0F 10 02 4D 89 5A 28 45 88 4A 30 41 0F 11 00 C3 48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??"
  try:
    with open(op.join(input_dir, patterns_filename), 'r') as pat_file:
      pat_list = pat_file.split(",")
      pat_count = 0
      result = POINTER(POINTER(int)*255) * len(pat_list)
      for pat in pat_list:
        hex_string = pat.replace(" ", "")
        patt_len = len(hex_string)
        P = (c_ubyte * patt_len).from_buffer_copy(bytearray(hex_string.encode('utf-8')))
        wildcard_index = POINTER(c_uint8)*100
        decoded_wildcard = WILDCARDS.from_address(c_utils.decode_hex(P, wildcard_index))
        P = decoded_wildcard.decoded_hex
        wildcard_index = decoded_wildcard.wildcard_index
        result[pat_count] = c_utils.index_search(P, patt_len, T, file_len, wildcard_index, index)
        pat_count += 1
  except Exception as e:
     print(f'Error: {e}')
     print(f'Unable to open {op.join(input_dir,patterns_filename)}')
     


  #free pointers allocated in C
  c_utils.freeptr(byref(result))
  c_utils.freeptr(byref(P))
  c_utils.freeptr(byref(index))
  c_utils.freeptr(byref(filtered_index_dict))
  c_utils.freeptr(byref(decoded_wildcard))
  #print(result)
  return result

main()