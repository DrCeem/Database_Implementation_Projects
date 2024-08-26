#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merge.h"

#define RECORDS_NUM 10000 // you can change it if you want
#define FILE_NAME "data.db"
#define OUT_NAME "out"


int createAndPopulateHeapFile(char* filename);

void sortPhase(int file_desc,int chunkSize);

void mergePhases(int inputFileDesc,int chunkSize,int bWay, int* fileCounter);

int nextOutputFile(int* fileCounter);

void printFileWithChunks(int file_desc,int numBlocksInChunk);

int main() {
  int chunkSize=5;
  int bWay = 2;
  int fileIterator = 0;
  //
  BF_Init(LRU);
  int file_desc = createAndPopulateHeapFile(FILE_NAME);
  //printFileWithChunks(file_desc,chunkSize);
  sortPhase(file_desc,chunkSize);
  //printFileWithChunks(file_desc,chunkSize);
  mergePhases(file_desc,chunkSize,bWay,&fileIterator);

}

//////////////////////////
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
        HP_Unpin(file_desc,i);
    }
    printf ("Records num: %d \n",record_num);
    chunk->recordsInChunk = record_num;
    CHUNK_Print(*chunk);
    
    while (CHUNK_GetNext(&iterator, chunk) != -1) {
        CHUNK_Print(*chunk);
    }
    
    free(chunk);
}
////////////////
int createAndPopulateHeapFile(char* filename){
  HP_CreateFile(filename);
  
  int file_desc;
  HP_OpenFile(filename, &file_desc);

  // int max_recs_in_block = HP_GetMaxRecordsInBlock(file_desc);
  // int block_id = 1;

  Record record;
  srand(12569874);
  for (int id = 0; id < RECORDS_NUM; ++id)
  {
    record = randomRecord();
    HP_InsertEntry(file_desc, record);
        // HP_Unpin(file_desc,HP_GetIdOfLastBlock(file_desc));

    
  }
  return file_desc;
}

/*Performs the sorting phase of external merge sort algorithm on a file specified by 'file_desc', using chunks of size 'chunkSize'*/
void sortPhase(int file_desc,int chunkSize){ 
  sort_FileInChunks( file_desc, chunkSize);
}

/* Performs the merge phase of the external merge sort algorithm  using chunks of size 'chunkSize' and 'bWay' merging. The merge phase may be performed in more than one cycles.*/
void mergePhases(int inputFileDesc,int chunkSize,int bWay, int* fileCounter){
  int oututFileDesc;
  while(chunkSize<=HP_GetIdOfLastBlock(inputFileDesc)){
    oututFileDesc = nextOutputFile(fileCounter);
    merge(inputFileDesc, chunkSize, bWay, oututFileDesc );
    HP_CloseFile(inputFileDesc);
    chunkSize*=bWay;
    inputFileDesc = oututFileDesc;
  }
  printFileWithChunks(oututFileDesc,HP_GetIdOfLastBlock(oututFileDesc));
  HP_CloseFile(oututFileDesc);
}

/*Creates a sequence of heap files: out0.db, out1.db, ... and returns for each heap file its corresponding file descriptor. */
int nextOutputFile(int* fileCounter){
    char mergedFile[50];
    char tmp[] = "out";
    sprintf(mergedFile, "%s%d.db", tmp, (*fileCounter)++);
    int file_desc;
    HP_CreateFile(mergedFile);
    HP_OpenFile(mergedFile, &file_desc);
    return file_desc;
}