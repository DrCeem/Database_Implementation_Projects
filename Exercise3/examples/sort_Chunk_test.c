#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chunk.h"
#include "sort.h"
#define FILENAME "sort_Chunk_test.db"

//Compile with: make sort_Chunk_test
//Run with: ./build/sort_Chunk_test

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
        // printRecord(record);
    }
    // Create a sample CHUNK
    CHUNK* chunk = malloc(sizeof(CHUNK));
    chunk->file_desc = fileDesc;
    chunk->from_BlockId = 1;
    chunk->blocksInChunk = 5;
    chunk->to_BlockId = 5;
    chunk->recordsInChunk = 45;

    // Print the contents of the unsorted chunk
    printf("\nBefore Sorting:\n\n");
    CHUNK_Print(*chunk);

    // Sort using Quicksort
    sort_Chunk(chunk);
    printf("\nAfter Quicksort:\n\n");
    CHUNK_Print(*chunk);

    result = HP_CloseFile(fileDesc);
    BF_Close();
    free(chunk);

    return result;
}
