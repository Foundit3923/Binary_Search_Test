//
// Created by michael on 2/13/2024.
//


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <ctype.h>
#include "stable_packedfilter.h"


#define LAST_BITS_ON 0x101010101010101UL
#define LAST_BITS_NOT_ON 0xFEFEFEFEFEFEFEFEUL
#define SIGNIFICANT_BITS_ON 0x8080808080808080UL
#define SIGNIFICANT_BITS_NOT_ON 0x7F7F7F7F7F7F7F7FUL

//Compares a character (c) to every character in (t). Results in a fully set byte when there is a match. Fully set byte is at match location.
#define xnor(t,q,c) t ^ q * (c)// & (t ^ LAST_BITS_ON * (c))

//Compares and reduces the comparison of a character (c) to every character in (t). Results in a fully set byte reduced to a byte with the LSB position on. Non-fully set bytes are 0
#define boolean_reduce(x,n) (x>>n)&x 

union B_window {
    uint64_t* i;
    uint8_t* c;
}B_window;

/* int getBitOffset(uint64_t matches){
  int err = -1;
  if(matches & 0xFFFFFFFF00000000UL){
    switch(matches>>32) {
      case 1: return 4;
      case 256: return 5;
      case 65536: return 6;
      case 16777216: return 7;
    }
  }
  else{
    switch(matches) {
      case 1: return 0;
      case 256: return 1;
      case 65536: return 2;
      case 16777216: return 3;
    }
  }
  return err;
} */

static unsigned char hexdigit2int(unsigned char xd)
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
}

/* unsigned char* decode_hex(unsigned char* st, uint8_t* wildcard_index[100]){
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
} */

/* uint64_t boolean_reduce(uint64_t x, int n){
  return(x>>n)&x;
}  */

int search_s_p (unsigned char* query_array,
                int query_len,
                unsigned char* text,
                int text_len,
                uint8_t* wildcard_index[100]) {

  //Setup
  int byte_offset = 0;
  int count = 0;
  int wildcard_count = 0;
  int first_offset = 0;
  

  uint8_t* byte_ptr = &query_array[0];
  uint8_t* last_byte = &query_array[query_len-1];
  uint8_t* first_byte = &query_array[0];
  uint8_t* match_start;
  uint8_t* jump = wildcard_index[wildcard_count];
  //uint8_t* results[20];

  int* locations = quick_pass(byte_ptr, 1, text, text_len);
  
  union B_window file_window;
  file_window.c = &text[0];

  uint64_t query_matches = LAST_BITS_ON;
  uint64_t value = 0;
  uint64_t value2 = 0;
  uint64_t value3 = 0;
  uint64_t reduced_value = 0;

  
  while(!(&*file_window.c > &text[text_len-1])) {

    if(&byte_ptr[0] == jump){
      byte_ptr+=(int)*jump;
      file_window.c+=(int)*jump;
      jump = wildcard_index[++wildcard_count];
      if(&byte_ptr[0] != last_byte){
        //count(query_matches);
        //*(R + count) = match_start + first_offset;
        count++;
        byte_ptr = first_byte;
        byte_offset += 8;
        file_window.c = &text[byte_offset];
        query_matches = LAST_BITS_ON;
        wildcard_count = 0;
      }
    }
    else{
      value = ~xnor(*file_window.i, query_matches, *byte_ptr);
      value2 = boolean_reduce(value,4);
      value3 = boolean_reduce(value2,2);
      reduced_value = boolean_reduce(value3,1);
      query_matches = reduced_value & query_matches;

      if((bool)(query_matches)) {
        if (&byte_ptr[0] == last_byte) {
          //count(query_matches);
          //*(R + count) = match_start + getBitOffset(first_offset);
          count++;
          byte_ptr = first_byte;
          byte_offset += query_len;
          file_window.c = &text[byte_offset];
          query_matches = LAST_BITS_ON;
          wildcard_count = 0;
        } else {
          //Character match found: move to next char in text and query
          if(&byte_ptr[0] == first_byte){
            match_start = file_window.c;
            //first_offset = query_matches;
          }
          byte_ptr++;
          file_window.c++;
        }
      } else {
        //No match found in window: move window and reset
        byte_ptr = first_byte;
        byte_offset += 8;
        file_window.c = &text[byte_offset];
        query_matches = LAST_BITS_ON;  
        wildcard_count = 0;
      }
    }
  }
  //R = results;
  return count;
}


