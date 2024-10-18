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


#define LAST_BITS_ON 0x101010101010101UL
#define LAST_BITS_NOT_ON 0xFEFEFEFEFEFEFEFEUL
#define SIGNIFICANT_BITS_ON 0x8080808080808080UL
#define SIGNIFICANT_BITS_NOT_ON 0x7F7F7F7F7F7F7F7FUL

//Compares a character (c) to every character in (t). Results in a fully set byte when there is a match. Fully set byte is at match location.
#define xnor(t,q,c) t ^ q * (c)// & (t ^ LAST_BITS_ON * (c))

//Reduces fully set bytes to a byte with a 1 in the LSB position, non fully set bytes are reudced to 0
#define reduce(v) (((~v - LAST_BITS_ON) ^ ~v) & SIGNIFICANT_BITS_ON) >> 7 //Original Method
//#define reduce(t,q,c) (~(xnor(t,q,c) + LAST_BITS_ON) / 128) & LAST_BITS_ON

//Counts the number of set bits in reduced integer
#define bitcount(q) (q + q/255) & 255

//Keeps track of found matches
#define count(v) result += bitcount(v)

//Compares and reduces the comparison of a character (c) to every character in (t). Results in a fully set byte reduced to a byte with the LSB position on. Non-fully set bytes are 0
//(((LAST_BITS_NOT_ON - x) ^ x) >> 7) & LAST_BITS_ON
//#define reduce(x) (((LAST_BITS_NOT_ON - x) ^ x) >> 7) & (((x - LAST_BITS_ON) ^ x) & SIGNIFICANT_BITS_ON) >> 7 
//#define reduce_xnor(t,q,c) (((LAST_BITS_NOT_ON - ~((t ^ q * (c)) << 1)) ) >> 7) & LAST_BITS_ON
//#define reduce_xnor(x) (((LAST_BITS_NOT_ON - ((LAST_BITS_NOT_ON - x) - x)) ) >> 7) & LAST_BITS_ON
//#define reduce_xnor(t,q,c) (~((q * (c)) - t) >> 7) & LAST_BITS_ON
#define boolean_reduce(x,n) (x>>n)&x 

//Unused
#define distance(x) ffsll(x >> 1) / 8

union Window {
    uint64_t* i;
    uint8_t* c;
}Window;

union Query {
    uint64_t* i;
    uint8_t* c;
    uint8_t * b;
};

union Test {
    char* c;
    //char l[8];
    uint64_t i;
};

int getBitOffset(uint64_t matches){
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
}

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

unsigned char* decode_hex(unsigned char* st){
  const char *src = st; 
  int st_len = strlen(st);
  int text_len = st_len;
  unsigned char* text = (unsigned char*) malloc(sizeof(unsigned char) * text_len);
  unsigned char* dst = text;
  bool first_wild_in_sequence = true;

  while (*src != '\0')
  {
    if(*src == '?'){
      *src++;
      *src++;
      if(first_wild_in_sequence){
        *dst++ = 0x2A;
      }      
      first_wild_in_sequence = false;
    }
    else{
      const unsigned char high = hexdigit2int(*src++);
      const unsigned char low  = hexdigit2int(*src++);
      *dst++ = (high << 4) | low;
      first_wild_in_sequence = true;
    }
  }
  *dst = '\0';
  return text;
}

int search_s_p (unsigned char* query_array,
                int query_len,
                unsigned char* text,
                int text_len,
                uint8_t* R[12]) {

  //Setup
  int result = 0;
  int byte_offset = 0;
  int count = 0;
  query_len = strlen(query_array)/2;
  query_array = decode_hex(query_array);
  int capacity = text_len/query_len;
  int first_offset = 0;

  uint8_t* byte_ptr = &query_array[0];
  uint8_t* last_byte = &query_array[query_len-1];
  uint8_t* first_byte = &query_array[0];
  uint8_t* match_start;
  uint8_t* segment_start;

  uint8_t* results[12];

  union Window file_window;
  file_window.c = &text[0];

  uint64_t query_matches = LAST_BITS_ON;
  uint64_t value = 0;
  uint64_t value2 = 0;
  uint64_t value3 = 0;
  uint64_t value4 = 0;
  uint64_t reduced_value = 0;

  bool wild = false;
  bool first_segment = true;

  search:

  //While the address of the first char of file_window.c is not the address of the last char of the text. 
  while(!(&*file_window.c > &text[text_len-1])) {
    if(*byte_ptr == '*') {
      //reset query_matches, flag not in the first segment, flag currently in wild search, store the beginning of this segment
      if(*(++byte_ptr) == '*'){
        goto search;
      }
      goto wild;
    }
    //Check if *byte_ptr matches any chars in *file_window  
    //reduce any found matches to a single bit occupying the lsb position of a byte
    //compare query_matches with reduce to only keep desired matches
    value = ~xnor(*file_window.i, query_matches, *byte_ptr);
    value2 = boolean_reduce(value,4);
    value3 = boolean_reduce(value2,2);
    reduced_value = boolean_reduce(value3,1);

    if((bool)(query_matches = reduced_value & query_matches)) {
      if (&byte_ptr[0] == last_byte) { //Pattern matched, no longer wild, count matches, store location, increase array position, now in first segment
        count(query_matches);
        *(results + count) = match_start + first_offset;     
        count++; 
        goto jump;      
      } else {
        //Character match found: move to next char in text and query
        if(match_start == NULL){
          match_start = file_window.c; //34
          first_offset = getBitOffset(query_matches);
        }                
        byte_ptr++;
        file_window.c++;
      }
    } else {
      //No match found in window: move window and reset
      goto jump;
    }
  }
  goto end;
  jump:
  match_start = NULL;
  byte_ptr = first_byte;
  byte_offset += 8;
  file_window.c = &text[byte_offset];
  query_matches = LAST_BITS_ON;  
  goto search;  

  wild:
  query_matches = LAST_BITS_ON;
  first_segment = false;
  wild = true;
  segment_start = &byte_ptr[0];
  //While the address of the first char of file_window.c is not the address of the last char of the text. 
  while(!(&*file_window.c > &text[text_len-1])) {
    if(*byte_ptr == '*') {
      //reset query_matches, flag not in the first segment, flag currently in wild search, store the beginning of this segment
      if(*(++byte_ptr) == '*'){
        goto search;
      }
      
      goto wild;
    }
    //Check if *byte_ptr matches any chars in *file_window  
    //reduce any found matches to a single bit occupying the lsb position of a byte
    //compare query_matches with reduce to only keep desired matches
    value = ~xnor(*file_window.i, query_matches, *byte_ptr);
    value2 = boolean_reduce(value,4);
    value3 = boolean_reduce(value2,2);
    reduced_value = boolean_reduce(value3,1);

    if((bool)(query_matches = reduced_value & query_matches)) {
      if (&byte_ptr[0] == last_byte) { //Pattern matched, no longer wild, count matches, store location, increase array position, now in first segment
        wild = false;
        first_segment = true;
        count(query_matches);
        *(results + count) = match_start + first_offset;     
        count++; 
        goto jump;        
      } else {
        //Character match found: move to next char in text and query             
        byte_ptr++;
        file_window.c++;
      }
    } else {
      //No match found in window: move window and reset
      goto wild_jump;
    }
  }
  goto end;
  wild_jump:
  byte_ptr = segment_start;
  byte_offset += 8;
  file_window.c = &text[byte_offset];
  query_matches = LAST_BITS_ON;  
  goto wild;  
  end:
  R = results;
  //Return False if the text is searched and nothing is found.
  return count;
}


