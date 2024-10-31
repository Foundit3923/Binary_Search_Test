#include "search_index.h"

typedef struct wildcards {
    unsigned char* st;
    uint8_t* wildcard_index[100];
}WILDCARDS;

typed

int** extractFuncRanges(char* filepath, int line_count){
  char line[58];
  int r_count = 0;
  int func_ranges[line_count][2];
  FILE* Functions = malloc(sizeof(FILE*));
  Functions = fopen(filepath, "r");    
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
  free(func_ranges);
  return func_ranges;
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

WILDCARDS* decode_hex(unsigned char* st, uint8_t* wildcard_index[100]){
  uint8_t* wildcard_index[100];
  const char *src = st; 
  int text_len = strlen(st);
  unsigned char* text = (unsigned char*) malloc(sizeof(unsigned char) * text_len);
  WILDCARDS* tmp = {text,wildcard_index};
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
      tmp->wildcard_index[wildcard_sequence_count] = &dst[0];
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
  return tmp;
}

void freeptr(void* ptr){
    free(ptr);
}