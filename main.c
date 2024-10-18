#include <stdio.h>
#include <fileapi.h>
#include <io.h>
#include "smart_s_p.h"



void remove_spaces (char* str_trimmed, const char* str_untrimmed)
{
  while (*str_untrimmed != '\0')
  {
    if(!isspace(*str_untrimmed))
    {
      *str_trimmed = *str_untrimmed;
      str_trimmed++;
    }
    str_untrimmed++;
  }
  *str_trimmed = '\0';
}

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

int main(){
  int result = 0;
  int test_result = 0;
  PLARGE_INTEGER T_size = (PLARGE_INTEGER) malloc(sizeof(PLARGE_INTEGER));
  char* text_path = "E:\\Documents\\Code\\Python\\byte_search\\NMS.bin";
  HANDLE text_handle = (HANDLE) malloc(sizeof(HANDLE));
  FILE* Text;
  unsigned char * T;
  unsigned char * P;
  uint8_t ** R = (uint8_t **) malloc(sizeof(uint8_t*));
  uint8_t* results[12];
  char* untrimmed_pat = "05 ?? ?? ?? ?? 48 63 98";//7E 3B 49 8B D6 48 2B D1 0F 1F 84 00 00 00 00 00";//28 C7 43 2C 00 00 00 80 E8 E3 F7 01 00 EB 03 48";//21 B8 01 4C CD 21 54 68"; //48 83 EC 28 48 8B 89 90";//48 83 EC 20 8B 49 38 48
  char * trimmed_pat = (char *) malloc(sizeof(char));
  remove_spaces(trimmed_pat, untrimmed_pat);
  P = (unsigned char *) malloc(sizeof(unsigned char) * strlen(trimmed_pat));
  P = (unsigned char*) trimmed_pat;
  Text = fopen(text_path, "rb");
  text_handle = (HANDLE) _get_osfhandle(_fileno(Text));
  GetFileSizeEx(text_handle, T_size);
  size_t st_size = (size_t) T_size->QuadPart;
  T = (unsigned char*) malloc(sizeof(unsigned char) * st_size);
  
  fread(T, sizeof(unsigned char), st_size, Text);
  test_result = search(P, strlen(P), T, (int)st_size);
  result = search_s_p(P, strlen(P), T, (int)st_size, results);

  return result;
}