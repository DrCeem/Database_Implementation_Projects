#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "sort.h"

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc ){
    
    // Get the number of chunks 
    int ChunkNum = ceil((float)HP_GetIdOfLastBlock(input_FileDesc)/ (float)chunkSize);

    // Get the number of steps needed for merging (basically the number of bWay-groups of chunks)
    int merge_steps = ceil((float)ChunkNum/(float)bWay);

    // Get the maximum number of records in a block
    int max_recs_in_block = HP_GetMaxRecordsInBlock(output_FileDesc);

    CHUNK* chunks = (CHUNK*) malloc(sizeof(CHUNK)*bWay);
    CHUNK_RecordIterator* record_iterators = (CHUNK_RecordIterator*) malloc(sizeof(CHUNK_RecordIterator) * bWay);
    Record* records = (Record*) malloc(sizeof(Record)*bWay);

    CHUNK_Iterator iterator =  CHUNK_CreateIterator(input_FileDesc,chunkSize);
    // printf("Iterator Info: \n %d\n %d \n %d \n",iterator.current,iterator.blocksInChunk,iterator.lastBlocksID);
    int k;

    bool is_Empty[bWay];
  
    int insertion_num = 0;

    

    for (int step = 0; step < merge_steps; step++) {


        if (step == 0) { 
            
            chunks[0].file_desc = input_FileDesc;
            chunks[0].from_BlockId = iterator.lastBlocksID-iterator.blocksInChunk+1;
            chunks[0].blocksInChunk = iterator.blocksInChunk;
            chunks[0].to_BlockId = iterator.lastBlocksID;
            int record_num = 0;
            // printf("chunks[0] Info: \n %d\n %d \n %d \n %d \n",chunks[0].file_desc,chunks[0].from_BlockId,chunks[0].blocksInChunk,chunks[0].to_BlockId);

        
            for(int i = 1; i <=iterator.blocksInChunk; i++) {
                record_num = record_num + HP_GetRecordCounter(input_FileDesc,i);
            }
        

            chunks[0].recordsInChunk = record_num;
            record_iterators[0] = CHUNK_CreateRecordIterator(&chunks[0]);

            k = 1;
            while ((k < bWay) && (CHUNK_GetNext(&iterator, &chunks[k]) != - 1 )) {
                record_iterators[k] = CHUNK_CreateRecordIterator(&chunks[k]);
                k++;
            }
        }


        else {

            k = 0;
            while ((k < bWay) && (CHUNK_GetNext(&iterator, &chunks[k]) != - 1 )) {
                record_iterators[k] = CHUNK_CreateRecordIterator(&chunks[k]);
                k++;
            }
        }


        
        int result;
        
        for (int i = 0; i < k ; i++) {
            result = CHUNK_GetIthRecordInChunk(&chunks[i],1,&records[i]);
        }
                                // printf("The problem is in merge\n");

        for (int i = 0; i < bWay; i++) {
            is_Empty[i] = false;
        }

        while(1) {

            int minimum_index = -1;

            for (int i = 0; i < k; i++) { 
                // If the chunk is not empty
                if (!is_Empty[i]) {
                    if ((minimum_index == -1) || (shouldSwap(&records[minimum_index],&records[i]) )) {
                        minimum_index = i;
                    }
                }
            }
            // If after the for loop the minimum_index still has a value of -1, it means that all chunks are empty,
            // therefore we can move onto the next group of bWay chunks
            if (minimum_index == -1) {
                break;
            }

            // Insert the smallest Record of the bway chunks in the new outut file
            HP_InsertEntry(output_FileDesc,records[minimum_index]);

            // Check if threre's more Records in the chunk we've just 'extracted' the minimum Record from
            if ( CHUNK_GetNextRecord(&record_iterators[minimum_index],&records[minimum_index]) == -1 ) {
                // Store a bool variable symbolizing the chunk minimun_index is now empty
                is_Empty[minimum_index] = true;
            }
        }
    }


    free(chunks);
    free(record_iterators);
    free(records);   
}