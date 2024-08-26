#include <stdio.h>
#include <stdlib.h>
#include "bf.h"
#include "hash_file.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float powerCustom(float base, float exponent) {

	float result=1;

	if (exponent > 0) {
        for (int i = 0; i < exponent; i++) {
            result *= base;
        }
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int hash(unsigned int  id, int depth) {

  unsigned int hash_value = id * 999999937;

  // Shift the depth-most significant bits 
  hash_value = hash_value >> ( 32- depth );

  return hash_value;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int* resize_array(int size, int* array) {

  // Allocate memory for the new array
  int* newArray= malloc( (2* size) * sizeof(int) );

  for(unsigned int  i = 0; i <2 * size; i++) {

    newArray[i] = array[i/2];

  }
  free(array);

  return newArray;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printRecord(Record record) {

    printf("Id: %d, Name: %s, Surname: %s, City: %s\n", record.id,record.name,record.surname,record.city);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode findBuddies(int file_desc,int record_block_id,int* first_buddy_index_block, int* first_buddy_pos,int* count) {

  // Initialize an index block as well as the struct and variables needed to access it's contents
  BF_Block* index_block;
  BF_Block_Init(&index_block);
  Index_Block_Info* index_block_info;
  void* index_data;
  int* hash_cell;

  BF_ErrorCode error;

  // The number of record block pointers(ids) stored in an index block
  int num_of_rec_blocks;

  int rec_block_id;

  // Initialize buddy count to 0
  *count = 0;

  // For every index block we check the record blocks that it contains
  for (int i = 1; i != -1; i = index_block_info->next_block_id) {
    
    // Load the index block in index block
    error = BF_GetBlock(file_desc,i,index_block);

    // If loading the index block was unsuccesful return HT_ERROR
    if (error != BF_OK) {
      printf( "Loading the block was unsuccesful\n");
      return HT_ERROR;
    }

    // Get the index block's data
    index_data = BF_Block_GetData(index_block);
    index_block_info = index_data;

    // Get a pointer to the position of the block where the hash table cells begin
    hash_cell = index_data + sizeof(Index_Block_Info);

    num_of_rec_blocks = index_block_info->num_of_record_blocks;

    // Access every one of the record block ids stored in the index block
    for (int j = 0; j < num_of_rec_blocks; j++) {
      
      // Get the record block's id that is stored in the index block's cells in position j, which acts as a "pointer" to that block
      rec_block_id = hash_cell[j];

      // If the record block id that is stored in position j is the same as the record block id we're looking for we consider 
      // them buddies, so we update the buddy count
      if (rec_block_id == record_block_id) {

        *count = *count + 1;                      
        
        // If this is the first buddy, save the index block it's id is stored in, as well as the buddy's position in said index block
        if (*count == 1) {
          *first_buddy_index_block = i;
          *first_buddy_pos = j;
        }
        
      }

    }

    // Unpin the index block
    error = BF_UnpinBlock(index_block);
  }

  // Destroy the block used in the function
  BF_Block_Destroy(&index_block);

  return HT_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* Check(int file_desc) {

  // Initialize an index block as well as the struct and variables needed to access it's contents
  BF_Block* index_block;
  BF_Block_Init(&index_block);
  Index_Block_Info* index_block_info;
  void* index_data;
  int* hash_cell;

  // Initialize a record block as well as the struct and variable needed to access it's contents
  BF_Block* record_block;
  BF_Block_Init(&record_block);
  Records_Block_Info* record_block_info;
  void* record_data;

  BF_ErrorCode error;

  Record* record;

  // The number of record block pointers(ids) stored in an index block
  int num_of_rec_blocks;

  int rec_block_id;
  void* data;

  // For every index block we check the record blocks that it contains
  for (int i = 1; i != -1; i = index_block_info->next_block_id) {
    
    // Load the index block in index block
    error = BF_GetBlock(file_desc,i,index_block);

    // printf("\nIndex block id is %d\n",i);

    printf("\n");
    // Get the index block's data
    index_data = BF_Block_GetData(index_block);
    index_block_info = index_data;

    // Get a pointer to the position of the block where the hash table cells begin
    hash_cell = index_data + sizeof(Index_Block_Info);

    num_of_rec_blocks = index_block_info->num_of_record_blocks;

    // Access every one of the record block ids stored in the index block
    for (int j = 0; j < num_of_rec_blocks; j++) {
      
      // Get the record block's id that is stored in the index block's cells in position j, which acts as a "pointer" to that block
      rec_block_id = hash_cell[j];

      // printf("Index id is %d, index_block[%d] = %d\n", i, j, rec_block_id);

      // Load the record block
      BF_GetBlock(file_desc,rec_block_id,record_block);
      data = BF_Block_GetData(record_block);
      record_block_info = data;

      record = data + sizeof(Records_Block_Info);
      printf("In index block %d, index_block[%d] -> record block %d, with %d/8 records\n", i, j, rec_block_id,record_block_info->record_num);

      // printf("In index block %d,[%d] -> %d\n", i, j, rec_block_id);

      // printf("num of records in record block %d is %d/8\n",rec_block_id,record_block_info->record_num);
      // for (int k = 0; k < record_block_info->record_num; k ++){
      //   printRecord(record[k]);
      // }
      // printf("\n");

      // Unpin the record block
      error = BF_UnpinBlock(record_block);
    }

    // Unpin the index block
    error = BF_UnpinBlock(index_block);
  }

  // Destroy the blocks used in the function
  BF_Block_Destroy(&record_block);
  BF_Block_Destroy(&index_block);

}