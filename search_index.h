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
#include "packedfilter_index.h"



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

int index_search (unsigned char* query_array,
                int query_len,
                unsigned char* text,
                int text_len,
                uint8_t* wildcard_index[100],
                int* file_index[256]) {

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

  int byte_index_size = file_index[(int)*first_byte][0];
  int* byte_index = file_index[(int)*first_byte];
  int results[255];

  union B_window file_window;
  
  //for each location in the associated byte_index, search until a match or mismatch is found. Repeat until all locations are checked
  for(int i=1; i<=byte_index_size; i++){

    byte_offset = byte_index[i];

    file_window.c = &text[byte_offset];

    uint64_t query_matches = LAST_BITS_ON;
    uint64_t value = 0;
    uint64_t value2 = 0;
    uint64_t value3 = 0;
    uint64_t reduced_value = 0;

    byte_ptr = first_byte;
    wildcard_count = 0;

    while(!(&*file_window.c > &text[text_len-1])) {
        
        //If the current byte_ptr is a wildcard segment skip that segment. Check if wildcard is the last char in pattern.
        if(&byte_ptr[0] == jump){
            if(!(&*file_window.c > &text[text_len-1])){
                byte_ptr+=(int)*jump;
                file_window.c+=(int)*jump;
                jump = wildcard_index[++wildcard_count];
                if(&byte_ptr[0] != last_byte){
                    //count(query_matches);
                    if(count < 256){
                        *(results + count) = match_start + first_offset;
                        count++;
                    }
                    break;
                }
            }
            else{
                break;
            }
        }
        else{
            //search for a match
            value = ~xnor(*file_window.i, query_matches, *byte_ptr);
            value2 = boolean_reduce(value,4);
            value3 = boolean_reduce(value2,2);
            reduced_value = boolean_reduce(value3,1);
            query_matches = reduced_value & query_matches;

            if((bool)(query_matches)) {
                if (&byte_ptr[0] == last_byte) {
                    //count(query_matches);
                    if(count < 256){
                        *(results + count) = match_start + getBitOffset(first_offset);
                        count++;
                    }
                    break;
                } else {
                    //Character match found: move to next char in text and query
                    if(&byte_ptr[0] == first_byte){
                        match_start = file_window.c;
                        first_offset = query_matches;
                    }
                    byte_ptr++;
                    file_window.c++;
                }
            }
            else{
                break;
            }
        }
    }
  }
  return results;
}


