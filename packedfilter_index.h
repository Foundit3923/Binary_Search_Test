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
#include <time.h>
#include "uthash.h"


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

struct my_struct {
    unsigned int id;
    char* code;
    int offset_size;
    int offset_used;
    int* tmp;
    int* offsets;
    UT_hash_handle int_handle;
    UT_hash_handle str_handle;
};

clock_t startTimer(){
  double start, end, cpu_time_used;
  return clock();
}

double endTimer(clock_t startTime){
  clock_t endTime = clock();
  return ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
}

struct my_struct *hex_int = NULL, *hex_code = NULL;

struct my_struct* find_hex_int(unsigned int hex_id){
    struct my_struct *i;

    HASH_FIND(int_handle, hex_int, &hex_id, sizeof(int), i);
    if (i) printf("found hex id %d: %s\n", hex_id, i->code);
    return i;
}

struct my_struct* find_hex_str(char* hex_str){
    struct my_struct *i;

    HASH_FIND(str_handle, hex_code, &hex_str, strlen(hex_str), i);
    if (i) printf("found hex string %s: %d\n", hex_str, i->id);
    return i;
}

void add_hex(char* code, unsigned int code_id){
    struct my_struct *i;

    i = find_hex_int(code_id);
    if (i == NULL){
        i = find_hex_str(code);
        if(i == NULL){
            i = (struct my_struct*)malloc(sizeof*i);
            i->id = code_id;
            i->code = (char*)malloc(sizeof(char)*strlen(code));
            strcpy(i->code, code);
            i->offset_size = 10;
            i->offset_used = 0;
            i->tmp = (int*)malloc(sizeof*(i->offsets) *i->offset_size);
            i->offsets = (int*)malloc(sizeof*(i->offsets) *i->offset_size);
            HASH_ADD(int_handle, hex_int, id, sizeof(int), i);
            HASH_ADD_KEYPTR(str_handle, hex_code, &code, strlen(code), i);
        }
    }
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

int** build_index (unsigned char* query_array,
            int query_len,
            unsigned char* text,
            int text_len) {

    //Setup
    int hex_idx_size = 0;
    int hex_idx_used = 0;
    int** index_buf = malloc(sizeof(int*)*256);

    union Window text_window;
    
    for(int i=0; i<256; i++){
        int* buf = NULL;
        int* tmp = NULL;
        int buf_size = 0;
        int buf_used = 0;
        int text_offset = 0;
        unsigned char* char_ptr = &query_array[i];

        if(buf_used == buf_size){
            buf_size += 1;             
            tmp = realloc(buf, buf_size*sizeof(int));
            if (!tmp){
                free(tmp);
                printf("oops");
            }
            else{
                buf = tmp;
            }
            buf[buf_used] = 0;
            buf_used++;
        }

        
        text_window.c = &text[0];

        uint64_t query_matches = LAST_BITS_ON;
        uint64_t value;
        uint64_t value2;
        uint64_t value3;
        uint64_t reduced_value;

        while(!(&*text_window.c > &text[text_len-1])) {
            value = ~xnor(*text_window.i, query_matches, *char_ptr);
            value2 = boolean_reduce(value,4);
            value3 = boolean_reduce(value2,2);
            reduced_value = boolean_reduce(value3,1);
            query_matches = reduced_value & query_matches;

            if((bool)(query_matches)) {
                if(buf_used == buf_size){
                    buf_size = buf_size * 2;                
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
                //printf("Text offset for hex %d | %d\n", i, buf[buf_used]);
                buf_used++;
            }
            //No match found in window: move window and reset
            text_offset += 8;
            text_window.c = &text[text_offset];
            query_matches = LAST_BITS_ON;
        }
        //store the number of indexes in the first bucket
        buf[0] = buf_used;
        //resize the index to trim excess allocated memory
        tmp = realloc(buf, buf_used*sizeof(int));
        if (!tmp){
            free(tmp);
        }
        else{
            buf = tmp;
        }
/*         printf("Buf_used: %d for hex: %d\n", buf_used, i);
        for(int l=1; l< buf_used;l++){
            printf("Text offset for hex %d | %d\n", i, buf[l]);
        } */
        //store address of first bucket in buf in hex_index at `hex_idx_completed`
        

        /* if(hex_idx_used == hex_idx_size){
            hex_idx_size += 10;
            hex_tmp = realloc(index_buf, hex_idx_size*sizeof(int*));
            if (!hex_tmp){
                free(hex_tmp);
            }
            else{
                index_buf = hex_tmp;
            }
        } */
        //index_buf[hex_idx_used] = (int*)malloc(buf[0] * sizeof(int));
        int size = buf[0];
        index_buf[i] = (int*)malloc(sizeof(int)* size);
        //hex_index[i] = (int*)malloc(sizeof(int)* size);
        memcpy(index_buf[i],buf, sizeof(int)*size);
        //index_buf[hex_idx_used] = &buf[0];
/*         int** visual = hex_index;
        int* visual2 = visual[hex_idx_used];
        int visual3 = index_buf[hex_idx_used];
        int visual4 = hex_index[hex_idx_used][0]; */
        //printf("Hex val: %d indexed | %d\n", *char_ptr, *index_buf[i]);
/*         if(hex_idx_used>0){
            int* test = index_buf[hex_idx_used-1];
            printf("Last Hex val: %c indexed | %d\n", query_array[i-1] , *test);
        } */
        char_ptr++;
        hex_idx_used++;
        free(buf);
    }

/*     hex_tmp = realloc(hex_index, hex_idx_used*sizeof(int*));
    if (!hex_tmp){
        free(hex_tmp);
    }
    else{
        hex_index = hex_tmp;
    }    */ 
    //*hex_index = (int**)malloc(sizeof(index_buf));
    //memcpy(&hex_index, &index_buf, sizeof(index_buf));
    //Return False if the text is searched and nothing is found.
    //memcpy(hex_index,index_buf,sizeof(index_buf));
/*     int test = 0;
    for(int i=0; i<10; i++){
        printf("Index count for Hex: %d | %d\n", i, *index_buf[i]);
        for(int j=1; j<index_buf[i][0]; j++){
        printf("Offset: %d | %d\n", i, index_buf[i][j]);
        }
    } */
    //memcpy(hex_index, index_buf, sizeof(index_buf));
    //hex_index = index_buf;
    //free(index_buf);
    return index_buf;
    

}
