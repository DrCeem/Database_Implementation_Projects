#include <merge.h>
#include <stdio.h>
#include <math.h>
#include "chunk.h"

CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk){
    
    // Initialize a chunk iterator struct
    CHUNK_Iterator chunk_iterator;

    // Set the file Desciptor and (maximum) number of blocks per chunk
    chunk_iterator.file_desc = fileDesc;
    chunk_iterator.blocksInChunk = blocksInChunk;
    
    // Set the current block id to be one since we are going to be starting the iteration from the first block of the first chunk which will be 
    // the block with id = 1    
    chunk_iterator.current = 1; 

    int number_of_blocks_in_file = HP_GetIdOfLastBlock(fileDesc);

    // The last block of the first chunk is: The value of blocksInChunk if the file has at least blocksInChunk blocks in total 
    // (excluding the metadata block) or the number of blocks in the file (excluding the metadata block) 
    // if the file has less blocks than blocksInChunk
    if(number_of_blocks_in_file > blocksInChunk)
        chunk_iterator.lastBlocksID = blocksInChunk;
    else 
        chunk_iterator.lastBlocksID = number_of_blocks_in_file;

    // Return the chunk iterator that was created 
    return chunk_iterator;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator,CHUNK* chunk) {

    // Get the id of the first block of the next chunk
    int id_first_block_next_chunk = iterator->lastBlocksID + 1;

    // Get the id of the last block in the file
    int last_block_in_file_id = HP_GetIdOfLastBlock(iterator->file_desc);

    // If the id of the first block in the next chunk is greater than the id of the last block in the file it means that there's no next
    // blocks to traverse, and therefore no more chunks
    if ( id_first_block_next_chunk > last_block_in_file_id) {
        return -1;
    }

    // If there aren't enough blocks left in the file for the chunk to be filled, the number of blocks in the chunk is the number of 
    // remaining blocks in the file
    if ( (id_first_block_next_chunk + iterator->blocksInChunk - 1 ) > last_block_in_file_id ) {
        chunk->blocksInChunk = last_block_in_file_id - id_first_block_next_chunk + 1;
    }
    // Otherwise the number of blocks in the chunk is the max number of blocks in a chunk
    else {
        chunk->blocksInChunk = iterator->blocksInChunk;
    }

    // Store the information needed in chunk
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = id_first_block_next_chunk;
    chunk->to_BlockId = chunk->from_BlockId + chunk->blocksInChunk - 1; 

    // Update the iterator
    iterator->current = chunk->from_BlockId;
    iterator->lastBlocksID = chunk->to_BlockId;
    iterator->blocksInChunk = chunk->blocksInChunk;

    int num_of_recs_in_chunk = 0;

    // For every block in the chunk add it's number of records in num_of_recs_in_chunk
    for (int i = iterator->current; i <= iterator->lastBlocksID; i ++ ) {

        num_of_recs_in_chunk += HP_GetRecordCounter(iterator->file_desc,i);
    }

    // Store the number of records in the chunk in chunk
    chunk->recordsInChunk = num_of_recs_in_chunk;

    // If everything worked return zero
    return 0;

}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk, int i, Record* record) {

    int block_capacity = HP_GetMaxRecordsInBlock(chunk->file_desc);

    // Calculate the block where the ith record is stored in the chunk
    int block_of_i = ceil( (float)i / (float)block_capacity );

    int pos_in_block;

    // If the position i of the record is divided perfectly with blok capacity, it means that said record is stored in the last position
    // of the block
    if ( i % block_capacity == 0 )
        pos_in_block = block_capacity;
    else
        pos_in_block = i % block_capacity; 

    // Stores the record wanted in record 
    int result = HP_GetRecord(chunk->file_desc, chunk->from_BlockId + block_of_i - 1 ,pos_in_block-1,record);
    
    result = HP_Unpin(chunk->file_desc,chunk->from_BlockId + block_of_i - 1 );
    
    return result;

}

int CHUNK_UpdateIthRecord(CHUNK* chunk, int i, Record record) {

    int block_capacity = HP_GetMaxRecordsInBlock(chunk->file_desc);

    // Calculate the block where the ith record is stored in the chunk
    int block_of_i = ceil( (float)i / (float)block_capacity );

    int pos_in_block;

    // If the position i of the record is divided perfectly with blok capacity, it means that said record is stored in the last position
    // of the block
    if ( i % block_capacity == 0 )
        pos_in_block = block_capacity;
    else
        pos_in_block = i % block_capacity; 

    // Updates the record wanted 
    int result = HP_UpdateRecord(chunk->file_desc,chunk->from_BlockId + block_of_i - 1,pos_in_block-1,record);
    
    result = HP_Unpin(chunk->file_desc,chunk->from_BlockId + block_of_i - 1 );
    
    return result;
}

void CHUNK_Print(CHUNK chunk) {

    int records_in_block;
    int result;
    Record rec;

    // Traverses the blocks of the chunk
    for (int block_iterator = chunk.from_BlockId; block_iterator <= chunk.to_BlockId; block_iterator ++) {

        printf("Block id: %d\n",block_iterator);

        records_in_block = HP_GetRecordCounter(chunk.file_desc, block_iterator);
        
        for (int record_iterator = 0; record_iterator < records_in_block; record_iterator ++) {

            // TESTING
            // printf("file desc:%d, block iterator: %d, record itrator: %d\n",chunk.file_desc,block_iterator,record_iterator);
            
            result = HP_GetRecord(chunk.file_desc,block_iterator,record_iterator,&rec);
            
            HP_Unpin(chunk.file_desc,block_iterator);

            // If HP_GetRecord was successful print the record
            if (result == 0) {
                printRecord(rec);
            }

        }

        printf("\n");
    }

}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {

    CHUNK_RecordIterator record_iterator;

    // Initialize the record iterator
    record_iterator.chunk = *chunk;
    record_iterator.currentBlockId = chunk->from_BlockId;
    record_iterator.cursor = 0;

    return record_iterator;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator,Record* record) {

    int block_capacity = HP_GetMaxRecordsInBlock(iterator->chunk.file_desc);
    int result;

    // If the current record (the one where cursor is "pointing") is not the last in the block, access the next one in said block 
    if (iterator->cursor < block_capacity-1) {

        iterator->cursor = iterator->cursor + 1;
        result = HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId,iterator->cursor,record);
        HP_Unpin(iterator->chunk.file_desc,iterator->currentBlockId);
    }
    // Otherwise, if the current block is not the last block in chunk, the next record is the first record in the next block 
    else if ( iterator->currentBlockId < iterator->chunk.to_BlockId) {
        
        iterator->currentBlockId = iterator->currentBlockId + 1;
        iterator->cursor = 0;

        result = HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor, record);
        HP_Unpin(iterator->chunk.file_desc, iterator->currentBlockId);
    }
    else {
        return -1;
    }

    return result;
    
}