//
// Created by michael on 2/13/2024.
//

//#include "include/main.h"
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
#define xnor(t,q,c) t ^ q * (c) // & (t ^ LAST_BITS_ON * (c))

//Reduces fully set bytes to a byte with a 1 in the LSB position, non fully set bytes are reudced to 0
//#define reduce(v) (((~v - LAST_BITS_ON) ^ ~v) & SIGNIFICANT_BITS_ON) >> 7 //Original Method
//#define reduce(t,q,c) (~(xnor(t,q,c) + LAST_BITS_ON) / 128) & LAST_BITS_ON

//Counts the number of set bits in reduced integer
#define bitcount(q) (q + q/255) & 255

//Keeps track of found matches
#define count(v) result += bitcount(v)

//Compares and reduces the comparison of a character (c) to every character in (t). Results in a fully set byte reduced to a byte with the LSB position on. Non-fully set bytes are 0
//(((LAST_BITS_NOT_ON - x) ^ x) >> 7) & LAST_BITS_ON
//#define reduce(x) (((LAST_BITS_NOT_ON - x) ^ x) >> 7) & LAST_BITS_ON//(((x - LAST_BITS_ON) ^ x) & SIGNIFICANT_BITS_ON) >> 7 
//#define reduce_xnor(t,q,c) ((LAST_BITS_NOT_ON - ~(t ^ q * (c))) >> 7) & LAST_BITS_ON
#define boolean_reduce(x,n) (x>>n)&x 

union Window {
    uint64_t* i;
    unsigned char* c;
}Window;

union Query {
    uint64_t* i;
    unsigned char* c;
};

union Test {
    char* c;
    //char l[8];
    uint64_t i;
};

int* quick_pass (unsigned char* query_array,
            int query_len,
            unsigned char* text,
            int text_len) {

    //Setup
    int buf_size = 0;
    int buf_used = 0;
    int* buf = (int*)malloc(sizeof(int));
    int* tmp = (int*)malloc(sizeof(int));
    int text_offset = 0;
    

    unsigned char* char_ptr = &query_array[0];
    unsigned char* last_char = &query_array[query_len-1];
    unsigned char* first_char = &query_array[0];
    
    union Window text_window;
    text_window.c = &text[0];

    uint64_t query_matches = LAST_BITS_ON;
    uint64_t value;
    uint64_t value2;
    uint64_t value3;
    uint64_t reduced_value;
    
    //While the address of the first char of text_window.c is not the address of the last char of the text. 
    while(!(&*text_window.c > &text[text_len-1])) {
        value = ~xnor(*text_window.i, query_matches, *char_ptr);
        value2 = boolean_reduce(value,4);
        value3 = boolean_reduce(value2,2);
        reduced_value = boolean_reduce(value3,1);
        query_matches = reduced_value & query_matches;

        if((bool)(query_matches)) {
            if (&char_ptr[0] == last_char) {
                //count(query_matches);
                if(buf_used == buf_size){
                    buf_size += 20;
                    tmp = realloc(buf, buf_size*sizeof(int));
                    if (!tmp){
                        free(tmp);
                        printf("oops");
                    }
                    else{
                        buf = tmp;
                    }
                }
                buf[buf_used] = text_offset;
                ++buf_used;
                char_ptr = first_char;
                text_offset += query_len;
                text_window.c = &text[text_offset];
                query_matches = LAST_BITS_ON;
            } else {
                //Character match found: move to next char in text and query
                char_ptr++;
                text_window.c++;
            }
        } else {
            //No match found in window: move window and reset
            char_ptr = first_char;
            text_offset += 8;
            text_window.c = &text[text_offset];
            query_matches = LAST_BITS_ON;
        }
    }
    //Return False if the text is searched and nothing is found.
    return buf;

}


