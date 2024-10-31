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

  #specify argtypes for some reason
  """
  unsigned char* query_array,
  int query_len,
  unsigned char* text,
  int text_len,
  uint8_t* wildcard_index[100],
  int* file_index[255]"""
  c_utils.index_search.argtypes = [POINTER(c_ubyte), c_int, POINTER(c_ubyte), c_int, POINTER(c_uint8)*100, POINTER(c_int)*255]
  c_utils.index_search.restype = c_int
  """ 
  unsigned char* st, 
  uint8_t* wildcard_index[100] """
  c_utils.decode_hex.argtypes = [POINTER(c_ubyte), POINTER(c_uint8)*100]
  c_utils.decode_hex.restype = c_void_p

  c_utils.freeptr.argtypes = c_void_p
  c_utils.freeptr.restype = None

  c_utils.build_index.argtypes = [POINTER(c_ubyte), c_int, POINTER(c_ubyte), c_int]
  c_utils.build_index.restype = POINTER(POINTER(c_int))


  #Decode string to find and generate wildcard index
  #TODO: load patterns from text file and run all
  string_to_find = "4C 8B 51 18 4D 8B D8 48 8B 05 ?? ?? ?? ?? 4D 8D 42 18 49 39 40 08 75 1C 48 8B 05 ?? ?? ?? ?? 49 39 00 75 10 0F 10 02 4D 89 5A 28 45 88 4A 30 41 0F 11 00 C3 48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??"
  hex_string = string_to_find.replace(" ", "")
  patt_len = len(hex_string)

  P = (c_ubyte * patt_len).from_buffer_copy(bytearray(hex_string.encode('utf-8')))
  wildcard_index = POINTER(c_uint8)*100
  decoded_wildcard = WILDCARDS.from_address(c_utils.decode_hex(P, wildcard_index))
  P = decoded_wildcard.decoded_hex
  wildcard_index = decoded_wildcard.wildcard_index

  #generate all hex sequences
  py_list = list(range(0,255))
  all_hex = (c_uint8  * len(py_list))(*py_list)

  #Load file to search and convert to ctype unsigned char*
  file_path = op.join(input_dir,to_search_filename)
  file_len = os.path.getsize(file_path)
  T = POINTER(c_ubyte * file_len)
  T = (c_ubyte * file_len).from_buffer(bytearray(open(file_path, 'rb').read()))

  #generate index
  index = POINTER(c_int)*255
  try:
    with open(op.join(output_dir, index_filename), 'r') as f:
      """
      hex_index = malloc(sizeof(int*)*256);
      int** buffer = hex_index;
      int* view;
      char* tmp;
      int hex;
      int size;
      int value;
      int count = 0;
      while (fgets(line, 15, Index))
      {
          tmp = strdup(line);
          if(strcmp(getfield(tmp,1), "]") == 0){
            count = 0;
            fgets(line, 15, Index);
            tmp = strdup(line);
            hex = atoi(getfield(tmp, 1));
            tmp = strdup(line);
            size = atoi(getfield(tmp, 2));
            buffer[hex] = malloc(sizeof(int)*size);
            view = buffer[hex];
            buffer[hex][count] = size;
            count++;
            fgets(line, 15, Index);
  
          }
          else{
            buffer[hex][count] = atoi(getfield(tmp, 1));
            count++;
          }
      }"""
      #TODO: Two options. This is adapted to python. Or a new format is adopted which would require filter.py to be rewritten
      #rewriting would be cleaner, but also I don't want to
       
  except:
    with open(op.join(output_dir, index_filename), 'a') as f:
       """build_index(
        unsigned char* query_array,
        int query_len,
        unsigned char* text,
        int text_len)"""
       index = c_utils.build_index(all_hex, len(py_list), T, file_len)

       """
       void index_to_json(FILE* f, int** hex_index, char* filepath){

         char buf[200];
         char* array_end = "]";
         char* delim = ", ";
         char* nl = "\n";
       
         f = fopen(filepath, "a");
         if(f == NULL){
           printf("Error opening index");
         }
         for(int i=0; i<256; i++){
           sprintf(buf, "%d", i);
           fprintf(f, "%s", buf);
           fprintf(f, "%s", delim);
           for(int j=0; j<hex_index[i][0]; j++){
             sprintf(buf, "%d", hex_index[i][j]);
             fprintf(f, "%s", buf);
             fprintf(f, "%s", delim); 
             fprintf(f, "%s", nl);
           }
           fprintf(f, "%s", array_end);
           fprintf(f, "%s", delim);
           fprintf(f, "%s", nl);
         }
       }"""
       #TODO: adapt to python. If we have to make it and it can be reused, it needs to be stored.


  #check if pdata
  filtered_index_dict = {}
  filtered_index = []
  if(get_pdata(op.join(input_dir, to_search_filename), op.join(output_dir,function_filename), op.join(output_dir,reject_filename))):
    #filter index by function ranges
    filtered_index_dict = filter_index_by_ranges(op.join(output_dir, index_filename), op.join(output_dir, function_filename), op.join(output_dir, filtered_index_filename))
    for key in filtered_index_dict:
       filtered_index[key] = list(filtered_index_dict[key]["size"]) + list(filtered_index_dict[key]["compliant"])
    index = cast(filtered_index, POINTER(c_int)*255)
  
  #TODO: have index_search return something other than the count
  result = c_utils.index_search(P, patt_len, T, file_len, wildcard_index, index)


  #free pointers allocated in C
  c_utils.freeptr(byref(result))
  c_utils.freeptr(byref(P))
  c_utils.freeptr(byref(index))
  c_utils.freeptr(byref(filtered_index_dict))
  c_utils.freeptr(byref(decoded_wildcard))
  print(result)

main()