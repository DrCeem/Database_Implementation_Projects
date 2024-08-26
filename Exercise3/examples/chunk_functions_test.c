#include <stdio.h>
#include <string.h>
#include "chunk.h"
#include "hp_file.h" 

#define FILENAME "chunk_test_file.db"

// Compile with: make chunk_functions_test
// Run with: ./build/chunk_functions_test

int test_CHUNK_CreateIterator(int fileDesc, CHUNK_Iterator* iterator) {

    int result;
    int blocksInChunk = 5; // Set the number of blocks per chunk for testing

    // Call the function to create the iterator
    *iterator = CHUNK_CreateIterator(fileDesc, blocksInChunk);

    // Perform tests
    printf("\nTESTING: CHUNK_CreateIterator\n\n");

    // Test 1: Check if the file descriptor is set correctly
    if (iterator->file_desc == fileDesc) {
        printf("File descriptor set correctly.\n");
    } else {
        printf("File descriptor NOT set correctly.\n");
    }

    // Test 2: Check if the current block is set correctly to 1
    if (iterator->current == 1) {
        printf("Current block set correctly to 1.\n");
    } else {
        printf("Current block NOT set correctly to 1.\n");
    }

    // Test 3: Check if last block ID is set according to the file's blocks
    int expectedLastBlock = (HP_GetIdOfLastBlock(fileDesc) > blocksInChunk) ? blocksInChunk : HP_GetIdOfLastBlock(fileDesc);
    if (iterator->lastBlocksID == expectedLastBlock) {
        printf("Last block ID set correctly.\n");
    } else {
        printf("Last block ID NOT set correctly.\n");
    }

    printf("\nTESTING COMPLETE.\n");


    return result;

}

void test_CHUNK_GetNext( CHUNK_Iterator* iterator) {

    // Test CHUNK_GetNext function with the created iterator
    printf("\nTESTING: CHUNK_GetNext\n\n");

    int result;
    int testCount = 1;

    Record fetchedRecord;
    Record new_record;
    new_record.id = 100;
    memcpy(new_record.name, (char*)"Orfeas",strlen("Orfeas") + 1);
    memcpy(new_record.surname, (char*)"Iliadis",strlen("Iliadis") + 1);
    memcpy(new_record.city, (char*)"Athens",strlen("Athens") + 1);
    memcpy(new_record.delimiter, "\n", strlen("\n") + 1);

    CHUNK chunk;
    chunk.file_desc = iterator->file_desc;
    chunk.from_BlockId = 1;
    chunk.to_BlockId = iterator->lastBlocksID;
    chunk.blocksInChunk = chunk.to_BlockId - chunk.from_BlockId + 1;

    CHUNK_RecordIterator record_iterator = CHUNK_CreateRecordIterator(&chunk);
    Record record;
    printf("Testing Record Iterator For 1st Chunk:\n");

    result = HP_GetRecord(record_iterator.chunk.file_desc,record_iterator.currentBlockId,record_iterator.cursor,&record);
    printf("\t");
    printRecord(record);

    while(CHUNK_GetNextRecord(&record_iterator,&record) == 0 ) {
        printf("\t");
        printRecord(record);
    }
    

    
    int num_of_recs_in_chunk = 0;

    // For every block in the chunk add it's number of records in num_of_recs_in_chunk
    for (int i = iterator->current; i <= iterator->lastBlocksID; i ++ ) {

        num_of_recs_in_chunk += HP_GetRecordCounter(iterator->file_desc,i);

    }

    // Store the number of records in the chunk in chunk
    chunk.recordsInChunk = num_of_recs_in_chunk;

    printf("\nPrint Iterator information:\n");
    printf("\tIterator File Descriptor: %d\n", iterator->file_desc);
    printf("\tIterator Current: %d\n", iterator->current);
    printf("\tIterator Last Block Id: %d\n", iterator->lastBlocksID);
    printf("\tIterator Blocks In Chunk : %d\n", iterator->blocksInChunk);

    // Print first chunk of file

    printf("\nPrint First Block information:\n");

    printf("\tFile Descriptor: %d\n", chunk.file_desc);
    printf("\tFrom Block ID: %d\n", chunk.from_BlockId);
    printf("\tTo Block ID: %d\n", chunk.to_BlockId);
    printf("\tBlocks in Chunk: %d\n", chunk.blocksInChunk);
    printf("\tRecords in Chunk: %d\n", chunk.recordsInChunk);
    
    printf("\nPrint chunk in blocks:\n\n");
    CHUNK_Print(chunk);

    printf("\nPrint all records in chunk:\n");
    for (int i = iterator->current; i <= iterator->lastBlocksID; i ++ ) {

        int rec_counter = HP_GetRecordCounter(iterator->file_desc,i);

        for (int j = 0; j < rec_counter; j++) {
            HP_GetRecord(iterator->file_desc,i,j,&fetchedRecord);
            printRecord(fetchedRecord);
        }
    }

    while (1) {

        result = CHUNK_GetNext(iterator, &chunk);

        printf("Test %d: ", testCount);

        if (result == -1) {
            printf("End of file reached. No more chunks available.\n");
            break;
        } else if (result == 0) {

            printf("Chunk obtained successfully.\n");

            // Perform tests for CHUNK_GetNext function

            // Test if iterator is updated properly
            printf("\tIterator File Descriptor: %d\n", iterator->file_desc);
            printf("\tIterator Current: %d\n", iterator->current);
            printf("\tIterator Last Block Id: %d\n", iterator->lastBlocksID);
            printf("\tIterator Blocks In Chunk : %d\n", iterator->blocksInChunk);

            printf("\n");

            // Test 1: Verify the values populated in the CHUNK struct
            printf("\tFile Descriptor: %d\n", chunk.file_desc);
            printf("\tFrom Block ID: %d\n", chunk.from_BlockId);
            printf("\tTo Block ID: %d\n", chunk.to_BlockId);
            printf("\tBlocks in Chunk: %d\n", chunk.blocksInChunk);
            printf("\tRecords in Chunk: %d\n", chunk.recordsInChunk);

            printf("\n");
            
            CHUNK_Print(chunk);

            printf("\nPrint all records in chunk:\n");

            for (int i = iterator->current; i <= iterator->lastBlocksID; i ++ ) {

                int rec_counter = HP_GetRecordCounter(iterator->file_desc,i);

                for (int j = 0; j < rec_counter; j++) {
                    HP_GetRecord(iterator->file_desc,i,j,&fetchedRecord);
                    printRecord(fetchedRecord);
                }
            }

            // Test CHUNK_GetIthRecordInChunk function
            // Test 2: Fetch specific records within the obtained chunk using CHUNK_GetIthRecordInChunk
            int i = rand() % 9;
            int fetchResult = CHUNK_GetIthRecordInChunk(&chunk, i , &fetchedRecord);
            
            printf("\n\tFetching Record %d: ", i);

            if (fetchResult == 0) {
                printf("Record fetched successfully:");
                printRecord(fetchedRecord);
                printf("\n");

            } else {
                printf("Failed to fetch record.\n");
            }

            printf("Update %dth record \n",i);
            result = CHUNK_UpdateIthRecord(&chunk,i,new_record);

            fetchResult = CHUNK_GetIthRecordInChunk(&chunk, i , &fetchedRecord);
            
            printf("\tFetching Updated Record %d: ", i);

            if (fetchResult == 0) {
                printf("Record fetched successfully:");
                printRecord(fetchedRecord);
                printf("\n");

            } else {
                printf("Failed to fetch record.\n");
            }


            
        } else {
            printf("Error encountered while obtaining chunk.\n");
        }

        testCount++;
    }

    printf("\nTESTING COMPLETE.\n");

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
    // Insert random records in the file
    for(int i = 0; i < 100; i++) {
        record = randomRecord();
        HP_InsertEntry(fileDesc,record);
    }

    CHUNK_Iterator iterator;

    // Call the test_CHUNK_CreateIterator function
    result = test_CHUNK_CreateIterator(fileDesc,&iterator);

    test_CHUNK_GetNext(&iterator);

    result = HP_CloseFile(fileDesc);
    BF_Close();

    return result;
}
