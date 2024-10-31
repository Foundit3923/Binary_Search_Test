#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fileapi.h>
#include <io.h>
#include <unistd.h>
#include <windows.h>
#include <shlwapi.h>
#include <ctype.h>
#include <errno.h>
#include "search_index.h"
#include "json-parser/json.h"



int remove_spaces(char* s) {
    char* d = s;
    int space_count = 0;
    do {
        while (*d == ' ') {
            ++d;
            space_count++;
        }
    *s++ = *d++;
    printf("%zu %c = %zu %c \n", s, *s, d, *d);
    } while (*s != '\0');
    return space_count;
}

/* static unsigned char hexdigit2int(unsigned char xd)
{
  if (xd <= '9')
    return xd - '0';
  xd = tolower(xd);
  if (xd == 'a')
    return 10;
  if (xd == 'b')
    return 11;
  if (xd == 'c')
    return 12;
  if (xd == 'd')
    return 13;
  if (xd == 'e')
    return 14;
  if (xd == 'f')
    return 15;
  return 0;
} */


void getFile(FILE* f, char* path){
  f = fopen(path, "rb");
}

int search(unsigned char *x, int m, unsigned char *y, int n) {
   int i, count, j;

   /* Searching */
   count = 0;
   for (j = 0; j <= n - m; ++j) {
      for (i = 0; i < m && x[i] == y[i + j]; ++i);
      if (i >= m)
         count++;
   }
   return count;
}

unsigned char* decode_hex(unsigned char* st, uint8_t* wildcard_index[100]){
  const char *src = st; 
  int st_len = strlen(st);
  int text_len = st_len;
  unsigned char* text = (unsigned char*) malloc(sizeof(unsigned char) * text_len);
  unsigned char* dst = text;
  bool wildcard_sequence_start = false;
  int wildcard_sequence_count = 0;
  int wildcard_count = 0;
  int count = 0;
  int char_count = 0;

  while (*src != '\0')
  {
    if(*src == '?'){      
      wildcard_count++;
      *src++;
      *src++;
      *dst = wildcard_count;
      wildcard_index[wildcard_sequence_count] = &dst[0];
      wildcard_sequence_start = true;
    }
    else{
      if(wildcard_sequence_start){
        wildcard_sequence_start = false;
        wildcard_sequence_count++;
        dst++;
      }
      const unsigned char high = hexdigit2int(*src++);
      const unsigned char low  = hexdigit2int(*src++);
      *dst = (high << 4) | low;
      dst++;
    }
    char_count++;
  }
  *dst = '\0';
  return text;
}

unsigned char* generate_all_hex(){
  char hex_num[10] = "0123456789";
  char hex_char[6] = "ABCDEF";
  unsigned char* all_hex = (unsigned char*)malloc(sizeof(unsigned char)*512);
  unsigned char* dst = all_hex;
  unsigned char* init = 0;

  for(unsigned int i=0; i<256; i++){
    *dst++ = i;
  }
  return all_hex;
}

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ","); //
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

int* readnfields(char* line, int size)
{
    const char* tok;
    int* indexes = malloc(sizeof(int)*size);
    int count = 0;
    for (tok = strtok(line, ","); //
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
      if(count > 1){
        if(count < size){
          indexes[count] = atoi(tok);
        }
        else{
          break;
        }
      }
      count++;
    }
    return indexes;
}

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
}

static void process_value(json_value* value, int depth)
{
        if (value == NULL) {
                return;
        }
        if (value->type != json_object) {
                print_depth_shift(depth);
        }
        switch (value->type) {
                case json_none:
                        printf("none\n");
                        break; 
                case json_null:
                        printf("null\n");
                        break;
                case json_object:
                        process_object(value, depth+1);
                        break;
                case json_array:
                        process_array(value, depth+1);
                        break;
                case json_integer:
                        printf("int: %10ld\n", (long)value->u.integer);
                        break;
                case json_double:
                        printf("double: %f\n", value->u.dbl);
                        break;
                case json_string:
                        printf("string: %s\n", value->u.string.ptr);
                        break;
                case json_boolean:
                        printf("bool: %d\n", value->u.boolean);
                        break;
        }
}

int main(){
  int result_occ = 0;
  int function_file_exists = 0;
  int test_result = 0;
  PLARGE_INTEGER T_size = (PLARGE_INTEGER) malloc(sizeof(PLARGE_INTEGER));
  char* index_filename = "\\index.txt";
  char* function_filename = "\\functions.txt";
  char *lpStr3;
  lpStr3 = index_filename;
  char* bin_filename = "\\NMS.exe";
  char* text_path = "E:\\Documents\\Code\\Python\\byte_search\\NMS.exe";
  char cwd[PATH_MAX];
  char *lpStr2;
  lpStr2 = cwd;
  GetCurrentDirectory(PATH_MAX, cwd);
  char fullpath[MAX_PATH] = "";
  char *lpStr1;
  lpStr1 = fullpath;
  HANDLE text_handle = (HANDLE) malloc(sizeof(HANDLE));
  HANDLE index_file_handle = (HANDLE) malloc(sizeof(HANDLE));
  FILE* File = malloc(sizeof(FILE*));
  FILE* Index = malloc(sizeof(FILE*));
  FILE* Functions = malloc(sizeof(FILE*));
  FILE* In_Range = malloc(sizeof(FILE*));
  FILE* Not_In_Range = malloc(sizeof(FILE*));
  unsigned char * T;
  unsigned char * P;
  uint8_t ** R = (uint8_t **) malloc(sizeof(uint8_t*));
  uint8_t* results[20];
  char patt_buffer[] = "4C 8B 51 18 4D 8B D8 48 8B 05 ?? ?? ?? ?? 4D 8D 42 18 49 39 40 08 75 1C 48 8B 05 ?? ?? ?? ?? 49 39 00 75 10 0F 10 02 4D 89 5A 28 45 88 4A 30 41 0F 11 00 C3 48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??";//"05 ?? ?? ?? ?? 48 63 98";//7E 3B 49 8B D6 48 2B D1 0F 1F 84 00 00 00 00 00";//28 C7 43 2C 00 00 00 80 E8 E3 F7 01 00 EB 03 48";//21 B8 01 4C CD 21 54 68"; //48 83 EC 28 48 8B 89 90";//48 83 EC 20 8B 49 38 48
  int spaces = remove_spaces(patt_buffer);
  //P = (unsigned char *) malloc(sizeof(unsigned char) * strlen(trimmed_pat));
  P = (unsigned char*) patt_buffer;
  File = fopen(text_path, "rb");
  text_handle = (HANDLE) _get_osfhandle(_fileno(File));
  GetFileSizeEx(text_handle, T_size);
  size_t st_size = (size_t) T_size->QuadPart;
  T = (unsigned char*) malloc(sizeof(unsigned char) * st_size);
  fread(T, sizeof(unsigned char), st_size, File);
  test_result = search(P, strlen(P), T, (int)st_size);

  clock_t start;
  double final_time;

  //generate wildcard index. Needed for final search
  uint8_t* wildcard_index[100];
  P = decode_hex(P, wildcard_index);

  //generate all hex vals. Needed to generate index
  unsigned char* all_hex = generate_all_hex();

  int line_count = 0;
  char line[58];
  int r_count = 0;
  char* functions_filepath = strdup(cwd);
  strcat(functions_filepath, function_filename);
  if(( Functions = fopen(functions_filepath, "r")) == NULL){
    function_file_exists = -1;
  }
  else{
    while (fgets(line, 58, Functions))
    {
      line_count++;
    }
  }
  
  int func_ranges[line_count][2];
  fseek(Functions, 0, SEEK_SET);
  fgets(line, 58, Functions);
  while (fgets(line, 58, Functions))
  {
      char* tmp = strdup(line);
      func_ranges[r_count][0] = (int)strtol(getfield(tmp, 1), NULL, 0);
      tmp = strdup(line);
      func_ranges[r_count][1] = (int)strtol(getfield(tmp, 2), NULL, 0);
      r_count++;
      free(tmp);
  }
  
  
  //int** hex_index = malloc(sizeof(int*)*256);
  printf("building index...\n");
  start = startTimer();
  int** hex_index;
  char* index_filepath = strdup(cwd);
  strcat(index_filepath, index_filename);
  if(( Index = fopen(index_filepath, "r")) == NULL){
    if (errno = ENOENT){
      hex_index = build_index(all_hex, strlen(all_hex), T, (int)st_size);
      final_time = endTimer(start);
      printf("index built in %f seconds\n",final_time);
      //transform to json
      index_to_json(Index, hex_index, index_filepath);
    }
  }
  else{
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
    }
    free(tmp);
    final_time = endTimer(start);
    printf("index built in %f seconds\n",final_time);
  }
  
  //filter index
/*   printf("Filtering index...\n");
  start = startTimer();
  int val = 0;
  int** index_in_func_range = malloc(sizeof(int*)*256);
  int** index_not_in_func_range = malloc(sizeof(int*)*256);
  int index_in_func_range_count = 0;
  int index_not_in_func_range_count = 0;
  char* in_range_filepath = strdup(cwd);
  strcat(in_range_filepath, "index_in_func_range.txt");
  char* not_in_range_filepath = strdup(cwd);
  strcat(not_in_range_filepath, "index_not_in_func_range.txt");
  if(( In_Range = fopen(in_range_filepath, "r")) == NULL || ( Not_In_Range = fopen(not_in_range_filepath, "r")) == NULL ){
    for(int i=0; i<256; i++){
      index_in_func_range[i] = malloc(sizeof(int)* hex_index[i][0]);
      index_not_in_func_range[i] = malloc(sizeof(int)* hex_index[i][0]);
      for(int j=0; j< hex_index[i][0];j++){
        val = hex_index[i][j];
        int pass = 0;
        for(int k=0; k<line_count; k++){
          if(val >= func_ranges[k][0] && val <= func_ranges[k][1]){
            index_in_func_range[i][index_in_func_range_count] = val;
            index_in_func_range_count++;
            pass = 1;
            break;
          }
        }
        if(!pass){
          index_not_in_func_range[i][index_not_in_func_range_count] = val;
          index_not_in_func_range_count++;
        }
        pass = 0;
      }
      index_in_func_range_count = 0;
      index_not_in_func_range_count = 0;
    }
    final_time = endTimer(start);
    printf("index filtered in %f seconds\n",final_time);
    index_to_json(In_Range, index_in_func_range, index_filepath);
    index_to_json(Not_In_Range, index_not_in_func_range, index_filepath);
  }
  else{ 
    //extract from files
    
  }*/
/*   int test = 0;
  for(int i=0; i<10; i++){
    printf("Index count for Hex: %d | %d\n", i, *hex_index[i]);
    for(int j=1; j<50; j++){
      printf("Offset: %d | %d\n", i, hex_index[i][j]);
    }
  } */

  char* filename = "\\filtered_index.json";
  FILE *fp;
  struct stat filestatus;
  int file_size;
  char* file_contents;
  json_char* json;
  json_value* value;

  if ( stat(filename, &filestatus) != 0) {
          fprintf(stderr, "File %s not found\n", filename);
          return 1;
  }
  file_size = filestatus.st_size;
  file_contents = (char*)malloc(filestatus.st_size);
  if ( file_contents == NULL) {
          fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
          return 1;
  }

  fp = fopen(filename, "rt");
  if (fp == NULL) {
          fprintf(stderr, "Unable to open %s\n", filename);
          fclose(fp);
          free(file_contents);
          return 1;
  }
  if ( fread(file_contents, file_size, 1, fp) != 1 ) {
          fprintf(stderr, "Unable to read content of %s\n", filename);
          fclose(fp);
          free(file_contents);
          return 1;
  }
  fclose(fp);

  json = (json_char*)file_contents;

  value = json_parse(json,file_size);

  if (value == NULL) {
    fprintf(stderr, "Unable to parse data\n");
    free(file_contents);
    exit(1);
  }

  start = startTimer();
  for(int n=0; n<1;n++){
    result_occ = search_index(P, strlen(all_hex), T, (int)st_size, wildcard_index, index_in_func_range);
  }
  final_time = endTimer(start);

  printf("Result: %d | %f\n", result_occ, final_time);
  free(T);
  free(T_size);




/*   struct my_struct *s, *i;
  unsigned char* uhex_str = "3F";
  char* hex_str = "3F";
  const unsigned char high = hexdigit2int(*uhex_str++);
  const unsigned char low  = hexdigit2int(*uhex_str);
  unsigned int* hex_id = (unsigned int*) malloc(sizeof(unsigned int) * 1);
  *hex_id = (high << 4) | low;
  add_hex(hex_str, *hex_id);
  s = find_hex_int(*hex_id);
  i = find_hex_str(hex_str);
  int test = (int) s->id;
  unsigned char* code = s->code;
  int test2 = (int) i->id;
  s->id = 24;
  test = (int) s->id;
  test2 = (int) i->id; */

  return 0;
}