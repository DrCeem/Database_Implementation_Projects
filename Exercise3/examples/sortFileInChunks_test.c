#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chunk.h"
#include "sort.h"
#define FILENAME "sortFileInChunks_test.db"

//Compile with: make sortFileInChunks_test
//Run with: ./build/sortFileInChunks_test

void printFileWithChunks(int file_desc,int numBlocksInChunk) {
    // Reset the iterator to the beginning of the file
    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    CHUNK* chunk = malloc(sizeof(CHUNK));
    chunk->blocksInChunk = iterator.blocksInChunk;
    chunk->file_desc = iterator.file_desc;
    chunk->from_BlockId = iterator.current;
    chunk->to_BlockId = iterator.blocksInChunk;

    int record_num = 0;

    for(int i = 1; i <= numBlocksInChunk; i++) {
        record_num = record_num + HP_GetRecordCounter(file_desc,i);
    }
    chunk->recordsInChunk = record_num;
    CHUNK_Print(*chunk);
    
    while (CHUNK_GetNext(&iterator, chunk) != -1) {
        CHUNK_Print(*chunk);
    }
}

int main() {
    BF_Init(LRU);

    // Initialize variables for testing
    int result = HP_CreateFile(FILENAME);

    if (result == -1){
        return result;
    }

    int fileDesc; 
    result = HP_OpenFile(FILENAME,&fileDesc);

    if (result == -1){
        return result;
    }
    
    Record record;
    for(int i = 0; i < 100; i++) {
        record = randomRecord();
        HP_InsertEntry(fileDesc,record);
    }
    int num_of_blocks_in_chunk = 5;

    printf("\nFile before Chunk Sorting:\n\n");
    printFileWithChunks(fileDesc,num_of_blocks_in_chunk);

    sort_FileInChunks(fileDesc,num_of_blocks_in_chunk);

    printf("\nFile after Chunk Sorting:\n\n");
    printFileWithChunks(fileDesc,num_of_blocks_in_chunk);

    result = HP_CloseFile(fileDesc);
    BF_Close();

    return result;
}