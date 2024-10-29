#include <stdio.h>
#include <fileapi.h>
#include <io.h>
//#include "packedfilter_index.h"
#include "search_index.h"


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

int main(){
  int result_occ = 0;

  int test_result = 0;
  PLARGE_INTEGER T_size = (PLARGE_INTEGER) malloc(sizeof(PLARGE_INTEGER));
  char* text_path = "E:\\Documents\\Code\\Python\\byte_search\\NMS.bin";
  HANDLE text_handle = (HANDLE) malloc(sizeof(HANDLE));
  FILE* Text;
  unsigned char * T;
  unsigned char * P;
  uint8_t ** R = (uint8_t **) malloc(sizeof(uint8_t*));
  uint8_t* results[20];
  char patt_buffer[] = "4C 8B 51 18 4D 8B D8 48 8B 05 ?? ?? ?? ?? 4D 8D 42 18 49 39 40 08 75 1C 48 8B 05 ?? ?? ?? ?? 49 39 00 75 10 0F 10 02 4D 89 5A 28 45 88 4A 30 41 0F 11 00 C3 48 8D 0D ?? ?? ?? ?? E9 ?? ?? ?? ??";//"05 ?? ?? ?? ?? 48 63 98";//7E 3B 49 8B D6 48 2B D1 0F 1F 84 00 00 00 00 00";//28 C7 43 2C 00 00 00 80 E8 E3 F7 01 00 EB 03 48";//21 B8 01 4C CD 21 54 68"; //48 83 EC 28 48 8B 89 90";//48 83 EC 20 8B 49 38 48
  int spaces = remove_spaces(patt_buffer);
  //P = (unsigned char *) malloc(sizeof(unsigned char) * strlen(trimmed_pat));
  P = (unsigned char*) patt_buffer;
  Text = fopen(text_path, "rb");
  text_handle = (HANDLE) _get_osfhandle(_fileno(Text));
  GetFileSizeEx(text_handle, T_size);
  size_t st_size = (size_t) T_size->QuadPart;
  T = (unsigned char*) malloc(sizeof(unsigned char) * st_size);

  fread(T, sizeof(unsigned char), st_size, Text);
  test_result = search(P, strlen(P), T, (int)st_size);
  clock_t start;
  double final_time;
  uint8_t* wildcard_index[100];
  P = decode_hex(P, wildcard_index);
  unsigned char* all_hex = generate_all_hex();
  
  //int** hex_index = malloc(sizeof(int*)*256);
  start = startTimer();
  int** hex_index = quick_pass(all_hex, strlen(all_hex), T, (int)st_size);
  final_time = endTimer(start);
  printf("Time to index: %f\n", final_time);
  for(int i=0; i<256; i++){
    printf("Index count for Hex: %d | %d\n", i, *hex_index[i]);
  } 

/*   int test = 0;
  for(int i=0; i<10; i++){
    printf("Index count for Hex: %d | %d\n", i, *hex_index[i]);
    for(int j=1; j<50; j++){
      printf("Offset: %d | %d\n", i, hex_index[i][j]);
    }
  } */

  start = startTimer();
  for(int n=0; n<1;n++){
    result_occ = search_index(P, strlen(all_hex), T, (int)st_size, wildcard_index, hex_index);
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