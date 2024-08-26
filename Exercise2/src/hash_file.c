#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

// Global array of File_info pointers
File_Info** File_Manager;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HT_Init() {

  // Initialize an array of pointers to File_Info structs
  File_Manager = (File_Info **)malloc(MAX_OPEN_FILES * sizeof(File_Info*) );

  // If allocation failed return HT_Error
  if (File_Manager == NULL) {
    printf("Error in HT_Init : Failed to allocate file manager\n");
    return HT_ERROR;
  }

  // Set all values to NULL cause we don't have any open files when the HT level is initialized
  for(int i = 0; i < MAX_OPEN_FILES; i++ ) {
    File_Manager[i] = NULL;
  }
  
  return HT_OK;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HT_Close() {

  for(int i; i < MAX_OPEN_FILES; i++) {
    // Deallocate the memory for each file's info (even if it is NULL, there isn't a file) 
    free(File_Manager[i]);
  }
  
  // Free the array where the file_info pointers were stored.
  free(File_Manager);

  return HT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
 
  // Create the file
  BF_ErrorCode error = BF_CreateFile(filename);

  // If creating the file was unsuccessful return HT_ERROR
  if (error != BF_OK) { 
    printf("Error in HT_Create_Index: The file already exists\n");
    return HT_ERROR;
  }
  
  int file_desc;

  // Open the file and store it's descriptor in file_desc
  error = BF_OpenFile(filename, &file_desc);

  // If opening the file was unsuccessful return HT_ERROR
  if (error != BF_OK) { 
    printf("Error in HT_Create_Index: Opening the file failed\n");
    return HT_ERROR;
  }

  // Initialize a block
  BF_Block* block;
  BF_Block_Init(&block);

  // Allocate the first block of the HT file containing information about it
  error = BF_AllocateBlock(file_desc,block);
  void* data = BF_Block_GetData(block);

  // Initialize the file_header struct that is stored in the first block of the HT file
  HT_File_Header* header = data;

  // Initialize the information about the file stored in the header
  header->isHT = true;
  header->hashing_item = "id";
  header->global_depth = depth;
  header->max_num_of_rec_in_bucket = (BF_BLOCK_SIZE - sizeof(Records_Block_Info)) / sizeof(Record);
  header->index_block_capacity =  ( BF_BLOCK_SIZE - sizeof( Index_Block_Info )) / (sizeof(int));

  // TESTING
  // printf("Block_id: 0 , Is HT: %d, Hashing item: %s, Global depth: %d, Max num of recs in bucket: %d\n",header->isHT, header->hashing_item,header->global_depth,header->max_num_of_rec_in_bucket);
  
  // Store the index block capacity in a variable in order to use it after unpining and destroyng the header block
  int index_block_capacity = header->index_block_capacity;

  // Set the block dirty and unpin it
  BF_Block_SetDirty(block);
  error = BF_UnpinBlock(block);

  // Destroy the block
  BF_Block_Destroy(&block);

  // Initialize an Index block
  BF_Block* index_block;
  BF_Block_Init(&index_block);

  // Initialize the struct and variable needed for accessing the index block's info
  Index_Block_Info* index_block_info;
  void* index_data;

  // Initialize a Record block
  BF_Block* record_block;
  BF_Block_Init(&record_block);

  // Initialize the struct and variable needed for accessing the record block's info
  Records_Block_Info* record_block_info;
  void* record_data;

  // Size of Hash table
  int hash_table_size =  powerCustom(2, depth);

  int index_blocks_num = ceil( (float)hash_table_size / (float)index_block_capacity  );

  // TESTING
  // printf("Number of Index Blocks: %d, Capacty of index block %d\n", index_blocks_num,index_block_capacity);

  // Initialize the index block(s) needed for storing the hash table 
  for(int i=1 ; i <= index_blocks_num; i++) {

    // Allocate a new index block in the file 
    error = BF_AllocateBlock(file_desc,index_block);

    // Get a pointer to the blocks data
    index_data = BF_Block_GetData(index_block);

    // Get a pointer of type Index_block_info to point to the blocks data
    index_block_info = index_data;

    // If it is not the last index block get the id of the next one
    if (i < index_blocks_num) { 

      // Set the next_index_block_id as the block counter of the file since the id of the next index block that will be created will have that id.
      int next_index_block_id;

      BF_GetBlockCounter(file_desc, &next_index_block_id);
      index_block_info->next_block_id = next_index_block_id;
    }
    // Otherwise set the next block id to -1
    else {
      index_block_info->next_block_id = -1;
    }

    // Since we altered the block, set it Dirty and Unpin it from memory
    BF_Block_SetDirty(index_block);                             
    error = BF_UnpinBlock(index_block);

    // If unpinning the block was unsuccessful return HT_ERROR
    if (error != BF_OK) { 
      printf("Error in HT_Create_Index: Unpining the index block failed\n");
      return HT_ERROR;
    }

  }

  // The number of record block pointers(id's) stored in each index block
  int records_block_num_in_index_block;

  // In the beginning the remaining record block (ids) to be stored are equal to the size of the hash_table
  int remaining_record_blocks = hash_table_size; 

  int* hash_table_cells;

  // Start from i=1 cause the block with id = 0 in not an index block
  for(int i=1 ; i <= index_blocks_num; i++) {

    // Load each index block
    error = BF_GetBlock(file_desc,i,index_block);

    // Get a pointer to int to point in the position of the block where the hash table cells will begin ("after the Index_Block_Info")
    index_data = BF_Block_GetData(index_block);
    index_block_info = index_data;
    hash_table_cells = index_data  + sizeof(Index_Block_Info);

    // If the index block's capacity is less than or equal to the number of record block pointer's that need to be stored in the hash 
    // table, the records_block_num_in_index_block is the number of the remaining record block pointers.
    if (remaining_record_blocks >= index_block_capacity) {
      records_block_num_in_index_block = index_block_capacity;
    }
    // If the remaining record blocks are more than the maximum capacity of the index block, this specific blocks stores the maximum 
    // capacity and the number of remaining block pointers is reduced by the maximum capacity of the index block
    else {
      records_block_num_in_index_block = remaining_record_blocks;
    }
    
    index_block_info->num_of_record_blocks = records_block_num_in_index_block;

    // The remaining record blocks to be created is reduced by the number of record blocks created in the last index block checked
    remaining_record_blocks -= records_block_num_in_index_block;

    for(int j= 0; j < records_block_num_in_index_block; j++) {

      // Create a new record block for each cell of the hashing table
      error = BF_AllocateBlock(file_desc,record_block);
      
      // Get the new records block id by geting the block counter of the file after it's creation and substracting 1
      int record_block_id;

      error = BF_GetBlockCounter(file_desc,&record_block_id);
      record_block_id--;

      // Store that id in the cell of the hashing table so we can find that bucket when hashing to that value
      hash_table_cells[j] = record_block_id;

      // Initialize the record block's info 
      record_data = BF_Block_GetData(record_block);
      record_block_info = record_data;

      record_block_info-> record_num = 0;
      record_block_info->local_depth = depth;

      // Dirty and Unpin the record_block since we dont need it taking space in the buffer any more
      BF_Block_SetDirty(record_block);
      error = BF_UnpinBlock(record_block);

    }

    // Dirty and Unpin the index blocks after we are done assigning values to each one so they dont take space in the buffer
    BF_Block_SetDirty(index_block);
    error = BF_UnpinBlock(index_block);
  }

  // Close the file and Destroy the structs that were used
  error = BF_CloseFile(file_desc);
  BF_Block_Destroy(&index_block);
  BF_Block_Destroy(&record_block);
      
  return HT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){

  int file_desc;

  // Opens the file with name fileName and return it's file descriptor in file_desc
  BF_ErrorCode error = BF_OpenFile(fileName, &file_desc);

  if (error == BF_ERROR){
    printf("Error in HT_OpenIndex : Opening the file failed\n");
    return HT_ERROR;
  }

  // Create a new File_Info struct for the file we are opening and initialize it
  File_Info* file_info = (File_Info *)malloc(sizeof(File_Info)); 

  if (file_info == NULL) {
      printf("Error in HT_OpenFile: File Info Allocation Failed\n");
      return HT_ERROR;
  }

  // Store the file descriptor and name in file_info
  file_info->file_desc = file_desc;
  file_info->file_name = (char*)fileName;

  // Find the first empty position in array File_Manager and store the file info there
  for (int i = 0; i < MAX_OPEN_FILES; i++) {

    if (File_Manager[i] == NULL) {
      File_Manager[i] = file_info;
      *indexDesc = i;
      return HT_OK;
    }
  }
 
  // If we exited the loop there's no empty positions in File Manager, meaning we've reached the max number of open files
  printf("Unable to Open File: Reached max number of open files\n");
  return HT_ERROR;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HT_CloseFile(int indexDesc) {

  // Load the file's info from File_Manager using the indexDesc
  File_Info* file_info = File_Manager[indexDesc];

  if (file_info == NULL){
    printf("Error in HT_Close_File: File not open\n");
    return HT_ERROR;
  }

  // Get the file descriptor
  int file_descriptor = file_info->file_desc;

  // Initialize a block
  BF_Block* block;
  BF_Block_Init(&block);

  BF_ErrorCode error;

  int count = 0;
  int total_num_of_blocks;

  // Load the total number of blocks in the file in total_num_of_blocks
  error = BF_GetBlockCounter(file_descriptor,&total_num_of_blocks);

  // Unpin every pinned block of the file from the memory
  while ( count < total_num_of_blocks) {
    error = BF_GetBlock(file_descriptor,count,block);
    error = BF_UnpinBlock(block);
    count++;
  }

  // Delete the file's corresponding entry from the File Manager
  File_Manager[indexDesc] = NULL;

  // Destroy the block
  BF_Block_Destroy(&block);

  // Free the struct containing the file's info
  free(file_info);

  // Close the BF file
  error = BF_CloseFile(file_descriptor);

  // If closing the file was unsuccesful return HT_ERROR
  if (error != BF_OK) {
    printf("Error in HT_CloseFile: BF_CloseFile Failed\n");
    return HT_ERROR;
  }

  // If closing the file was succesful return HT_OK
  return HT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  
  // Load the file's info from File_Manager using the indexDesc
  File_Info* file_info = File_Manager[indexDesc];

  // If file isn't open return HT_ERROR
  if (file_info == NULL){
    printf("Error in HT_InsertEntry: File not open\n");
    return HT_ERROR;
  }

  // Get the file descriptor
  int file_descriptor = file_info->file_desc;

  BF_ErrorCode error;

  // Initialize a block
  BF_Block* block;
  BF_Block_Init(&block);

  // Load the header block (block 0)
  BF_GetBlock(file_descriptor,0,block);
  void* data = BF_Block_GetData(block);

  // Use the File Header to get the global depth 
  HT_File_Header* header = data;
  int global_depth = header->global_depth;
  int max_num_of_records_per_bucket = header->max_num_of_rec_in_bucket;
  int index_block_capacity = header->index_block_capacity; 

  // Unpin header block
  BF_UnpinBlock(block);

  // Destroy the block
  BF_Block_Destroy(&block);                                     
  
  // Calculate the hash value for the id given
  unsigned int id_hash_value = hash(record.id,global_depth);

  // Find the index block that contains a "pointer" to the record block where the record given should be stored
  int index_block_num = id_hash_value / index_block_capacity + 1;
  
  int pos_in_index_block = id_hash_value % index_block_capacity;

  BF_Block* index_block;
  BF_Block_Init(&index_block);
  Index_Block_Info* index_block_info;

  BF_Block* record_block;
  BF_Block_Init(&record_block);
  Records_Block_Info* record_block_info;

  // If the Index block we want is the first one, the loop will be skipped
  int index_id = 1;
  
  // Access all the Index blocks until we find the one we're searching for
  for (int i = 1; i < index_block_num; i ++) {

    // Load each index block
    error = BF_GetBlock(file_descriptor,index_id,index_block);

    data = BF_Block_GetData(index_block);
    index_block_info = data;

    // Since it's possible for the index blocks to not be stored serially we access them using the next_block id
    index_id = index_block_info->next_block_id;

    BF_UnpinBlock(index_block);
  }

  // Now that index_id stores the Index block id we want, we load said Index block
  BF_GetBlock(file_descriptor,index_id,index_block);
  data = BF_Block_GetData(index_block);

  // Get a pointer to the hash table cells
  int* index_cells = data + sizeof(Index_Block_Info);

  // Get the record block's/bucket id from the index block's cells
  int record_block_id = index_cells[pos_in_index_block];

  // Load the record block
  BF_GetBlock(file_descriptor,record_block_id,record_block);
  data = BF_Block_GetData(record_block);
  record_block_info = data;

  // Get a pointer to Record where the Records begin in the bucket
  Record* rec = data + sizeof(Records_Block_Info);

  // If the number of records currently in the bucket is less than the max number of records that can be stored in one, 
  // insert the record in the bucket
  if (record_block_info->record_num < max_num_of_records_per_bucket) {

    rec[record_block_info->record_num].id = record.id;
    memcpy(rec[record_block_info->record_num].name,record.name,strlen(record.name) + 1);
    memcpy(rec[record_block_info->record_num].surname,record.surname,strlen(record.surname) + 1);
    memcpy(rec[record_block_info->record_num].city,record.city,strlen(record.city)+1);
    
    record_block_info->record_num++;
    
    // Set Dirty and Unpin record block
    BF_Block_SetDirty(record_block);
    error = BF_UnpinBlock(record_block);

    // Unpin index block
    error = BF_UnpinBlock(index_block);

    // Destroy the blocks created to access the file's records                            
    BF_Block_Destroy(&index_block);
    BF_Block_Destroy(&record_block);
    
  }
  // If the record doesn't fit in the bucket it's supposed to be stored in, there're 2 possibillities
  else {
    
    // The local depth of the bucket the record's supposed to go in is less than the global depth
    if (record_block_info->local_depth < global_depth) {

      // TESTING
      //printf("\nSplit bucket case: local depth: %d , global depth: %d,bucket id: %d\n",record_block_info->local_depth,global_depth,record_block_id);

      // Initialize a new BF_block in the file that will be the new record block (bucket)
      BF_Block* new_record_block;
      BF_Block_Init(&new_record_block);

      // Allocate a new BF block in the File
      error = BF_AllocateBlock(file_descriptor, new_record_block);

      if (error != BF_OK) {
        printf("Error in HT_InsertEntry: Allocating the new block was unsuccesfull\n");
        return HT_ERROR;
      }

      data = BF_Block_GetData(new_record_block);

      // Get a pointer to the record block's info
      Records_Block_Info* new_rec_block_info = data;

      // Initialize the number of records in the new block to 0
      new_rec_block_info->record_num = 0;

      // Increase both the new and the old record's depth by one
      new_rec_block_info->local_depth = record_block_info->local_depth +1;
      record_block_info->local_depth = record_block_info->local_depth +1;

      // Find the id of the block that was just created with BF_GetBlockCounter
      int new_rec_block_id;
      error = BF_GetBlockCounter(file_descriptor,&new_rec_block_id);
      new_rec_block_id --;                                                  
      
      // Set dirty and unpin the new record block
      BF_Block_SetDirty(new_record_block);
      error = BF_UnpinBlock(new_record_block);

      // Destroy the new record block
      BF_Block_Destroy(&new_record_block);  

      // TESTING
      // printf("\nNew block's id:%d\n",new_rec_block_id);
      
      // Find the buddies that used to point to the bucket that the new record should have been inserted to (but was full)
      int first_buddy_index_block;
      int first_buddy_pos;
      int buddy_count;

      error = findBuddies(file_descriptor, record_block_id,&first_buddy_index_block,&first_buddy_pos, &buddy_count);

      if (error != HT_OK) {
        printf("Error in findBuddies\n");
        return HT_ERROR;
      }
      
      // Load the index block containing the first buddy and find the position of said buddy
      error = BF_GetBlock(file_descriptor,first_buddy_index_block, index_block);
      
      data = BF_Block_GetData(index_block);
      index_block_info = data;
      index_cells = data + sizeof(Index_Block_Info);

      int j = first_buddy_pos;

      // The number of hashing table cells that should point to the new bucket (record block) is half the number of buddies 
      for (int i = 0; i < (buddy_count/2) ; i++) {

        index_cells[j] = new_rec_block_id;

        // If the rest of the hash table's cells that need to be changed are stored in the next index block(s) we access said next block
        if (j == index_block_capacity-1) {                                  

          // Set dirty and Unpin the previous index block that had changes done 
          BF_Block_SetDirty(index_block);
          error = BF_UnpinBlock(index_block);

          // Load the next index block
          error = BF_GetBlock(file_descriptor,index_block_info->next_block_id,index_block);
          data = BF_Block_GetData(index_block);
          index_block_info = data;

          index_cells = data + sizeof(Index_Block_Info);

          // j resets to 0 since it is a new block
          j = 0;
        }
        else
          j++;
        
      }
      // If the last block wasn't filled it has yet to be set dirty and unpined so do that now
      if (j!=0){
        // Set dirty and Unpin the last index block that had changes done 
        BF_Block_SetDirty(index_block);
        error = BF_UnpinBlock(index_block);
      }

      // Allocate a temporary array of records to insert the records that the full bucket (record block) stored
      // and then re-hash them into the right bucket

      // The size of the temporary array
      int temp_array_size = record_block_info->record_num;

      // Allocate memory for the array
      Record* temp_rec_array = malloc(temp_array_size * sizeof(Record));

      // Get a pointer to the data of the record block
      data = BF_Block_GetData(record_block);
      Record* rec = data + sizeof(Records_Block_Info);

      // Copy all the records from the full bucket to the temporary array
      for (int i=0; i<record_block_info->record_num; i++) {
        temp_rec_array[i] = rec[i];
      }

      // Set the full buckets record_num to 0 (so the new data replaces the old)
      record_block_info->record_num = 0;

      // Set dirty and unpin the new and old record blocks so the insertions can happen
      BF_Block_SetDirty(record_block);
      error = BF_UnpinBlock(record_block);

      BF_Block_Destroy(&record_block);                            
      BF_Block_Destroy(&index_block);

      HT_ErrorCode HT_error;

      // For every record reapeat the proccess of inserting a record 
      for(int i = 0; i < temp_array_size; i++) {
        
        // Call HT_insert recursivly 
        HT_error = HT_InsertEntry(indexDesc, temp_rec_array[i]);                                  

        // If insertion failed return HT_ERROR
        if (HT_error != HT_OK)
          return HT_ERROR;

      }

      free (temp_rec_array);

      // Finally insert the record that was originally supposed to be inserted 
      HT_error = HT_InsertEntry(indexDesc, record);                                       
      
      // If insertion failed return HT_ERROR
      if (HT_error != HT_OK)
          return HT_ERROR;

    }
    // The local depth of the bucket the record's supposed to go in is equal to the global depth
    // In this case the hash table extends and it's contents are rearranged properly
    else {
      
      // TESTING
      // printf("\nExpand Table occasion GLOBAL DEPTH: %d, LOCAL DEPTH: %d\n",global_depth,record_block_info->local_depth);

      // We create a dynamic array that will temporarily store the hash table (currently stored in one or more index blocks)
      // so it can be properly doubled in size and have it's contents properly updated.

      // The temp array's initial size is the number of hash table cells which is 2^global_depth
      int size = powerCustom(2,global_depth);

      // Allocate memory for the array
      int* temp_array = malloc( size * sizeof(int) );
      
      int num_of_rec_blocks;
      int record_number;

      int k = 0;

      // Access each index block using the next block id
      for (int i = 1; i != -1; i = index_block_info->next_block_id) {
      
        // Load the index block
        error = BF_GetBlock(file_descriptor,i,index_block);

        // If loading the index block was unsuccesful return HT_ERROR
        if (error!= BF_OK) {
          printf( "Loading the block was unsuccesful\n");
          return HT_ERROR;
        }

        // Get the index block's data
        data = BF_Block_GetData(index_block);
        index_block_info = data;

        // Get a pointer to the position of the block where the hash table cells begin
        index_cells = data + sizeof(Index_Block_Info);

        // Get the number of record "pointers" (ids) stored in the index block
        num_of_rec_blocks = index_block_info->num_of_record_blocks;

        // For every one of the record blocks stored in the index block get their id
        for (int j = 0; j < num_of_rec_blocks; j++) {

          // Get the record block's id that is stored in the index block's cells in position j and store it in temp array
          temp_array[k] = index_cells[j];

          k++;

        }
        // "Empty" the cells
        index_block_info->num_of_record_blocks = 0;                                       
        
        // Unpin the index block
        error = BF_UnpinBlock(index_block);
      }

      // Resize the array
      int* new_array = resize_array(size, temp_array);

      // Load the first index block
      BF_GetBlock(file_descriptor,1,index_block);
      data = BF_Block_GetData(index_block);
      index_block_info = data;

      index_cells = data + sizeof(Index_Block_Info);

      // j is used for accessing the index block's cells
      int j = 0;
      int current_index_block_id = 1;

      // Now that new array stores the updated hash table, go back to update the existing hash table cells
      for(int i = 0; i < size; i++) {

        index_cells[j] = new_array[i];

        index_block_info->num_of_record_blocks ++;
        
        // Once the block gets filled move on to the next one
        if (j == index_block_capacity-1) {
            
          // If the block we just filled was not the (old) last one, access the next one
          if (index_block_info->next_block_id != -1) {

            // Set previous index block Dirty and Unpin
            BF_Block_SetDirty(index_block);
            error = BF_UnpinBlock(index_block);

            // Get the id of the next index_block
            current_index_block_id = index_block_info->next_block_id;

            // Load the next index block
            error = BF_GetBlock(file_descriptor,index_block_info->next_block_id,index_block);

            data = BF_Block_GetData(index_block);
            index_block_info = data;

            // Get a pointer to the new block's cells
            index_cells = data + sizeof(Index_Block_Info);
          }
          // j resets to 0 since it is a new block
          j=0;
        }
        else
          j++;

      }

      // If j is 0 it means that the last block was filled so we allocate a new block
      if (j == 0) {

        int blocks_num;
        error = BF_GetBlockCounter(file_descriptor,&blocks_num);

        // The new block's id is the number of blocks in the file 

        index_block_info->next_block_id = blocks_num ;                            

        BF_Block_SetDirty(index_block);
        error = BF_UnpinBlock(index_block);

        // Allocate a new block in the file
        BF_AllocateBlock(file_descriptor,index_block);

        error = BF_GetBlock(file_descriptor,index_block_info->next_block_id,index_block);
        data = BF_Block_GetData(index_block);
        index_block_info = data;

        index_block_info->next_block_id = -1;
        index_block_info->num_of_record_blocks = 0;

        index_cells = data + sizeof(Index_Block_Info);
      }

      // Keep index_block because it indicates which index block was last accessed

      // Keep j as "counter" beacause it's current value indicates the next position in an index block
      // that should be assigned a record block/bucket id
      
      // For the other half of the hash table (the newly formed one) allocate new index blocks to store the rest of the values
      for (int i = size; i < 2*size; i++) {

        index_cells[j] = new_array[i];

        index_block_info->num_of_record_blocks ++;

        // When the index block gets filled allocate a new block
        if (j == index_block_capacity -1) {

          // Only create a new index block if the last index block is full and there's more elements to be stored
          // This ensures that if the last block is filled, and there's no more elements in the array, we don't allocate an extra block
          if (i < 2*size-1) {
            
            int blocks_num;
            error = BF_GetBlockCounter(file_descriptor,&blocks_num);

            // The new block's id is the number of blocks in the file before allocating said block
            index_block_info->next_block_id = blocks_num;

            // Get the id of the next index_block
            current_index_block_id = index_block_info->next_block_id;

            // Set previous block Dirty and Unpin
            BF_Block_SetDirty(index_block);
            error = BF_UnpinBlock(index_block);

            // Allocate a new block in the file
            BF_AllocateBlock(file_descriptor,index_block);
          
            // Load the new block
            error = BF_GetBlock(file_descriptor,index_block_info->next_block_id,index_block);
            data = BF_Block_GetData(index_block);
            index_block_info = data;

            // Initialize it's info
            index_block_info->next_block_id = -1;
            index_block_info->num_of_record_blocks = 0;

            // Get a pointer to access it's cells
            index_cells = data + sizeof(Index_Block_Info);
          }
          j=0;
        }
        else
          j++;

      }

      // Set dirty and Unpin the last index block that had changes done 
      BF_Block_SetDirty(index_block);
      error = BF_UnpinBlock(index_block);

      // Initialize a block
      BF_Block* block;
      BF_Block_Init(&block);

      // Load the header block (block 0)
      BF_GetBlock(file_descriptor,0,block);
      void* data = BF_Block_GetData(block);

      HT_File_Header* header = data;

      // Increase global depth by one
      header->global_depth = header->global_depth + 1;

      // Set header block Dirty and Unpin
      BF_Block_SetDirty(block);
      error = BF_UnpinBlock(block);

      // Destroy the blocks used 
      BF_Block_Destroy(&block);
      BF_Block_Destroy(&index_block);
      BF_Block_Destroy(&record_block);

      // Free the array
      free (new_array);

      HT_ErrorCode HT_error;

      // Finally insert the record that was originally supposed to be inserted 
      HT_error = HT_InsertEntry(indexDesc, record);                                  

      // If insertion was unsuccessful return HT_ERROR
      if (HT_error != HT_OK)
        return HT_ERROR;
          
    }

  }

  // TESTING
  // Check(file_descriptor);
  // printf("\nRecord with id %d inserted\n",record.id);

  return HT_OK;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {

  // Load the file's info from File_Manager using the indexDesc
  File_Info* file_info = File_Manager[indexDesc];

  if (file_info == NULL){
    printf("Error in HT_PrintAllEntries: File not open\n");
    return HT_ERROR;
  }

  // Get the file descriptor
  int file_descriptor = file_info->file_desc;

  int total_num_of_blocks;

  // Load the number of blocks in the file in total_num_of_blocks
  BF_ErrorCode error = BF_GetBlockCounter(file_descriptor,&total_num_of_blocks);

  // Initialize the structs and variables needed to access the index blocks
  BF_Block* index_block;
  BF_Block_Init(&index_block);
  Index_Block_Info* index_block_info;
  void* index_data;
  int* hash_cell;

  // Initialize the structs and variable needed to access the record blocks
  BF_Block* record_block;
  BF_Block_Init(&record_block);
  Records_Block_Info* record_block_info;
  void* record_data;
  
  Record* record;

  int num_of_rec_blocks;
  int record_block_id;

  // If id is NULL print all entries
  if (id==NULL) {

    for (int i = 1; i != -1; i = index_block_info->next_block_id) {
      
      // Load the index block in index block
      error = BF_GetBlock(file_descriptor,i,index_block);

      // If loading the index block was unsuccesful return HT_ERROR
      if (error!= BF_OK){
        printf( "Loading the block was unsuccesful\n");
        return HT_ERROR;
      }

      // Get the index block's data
      index_data = BF_Block_GetData(index_block);
      index_block_info = index_data;

      // Get a pointer to the position of the block where the hash table cells begin
      hash_cell = index_data + sizeof(Index_Block_Info);

      // Get the number of pointers (ids) to record blocks/buckets stored in the index block
      num_of_rec_blocks = index_block_info->num_of_record_blocks;

      // For every one of the record blocks stored in the index block access it's records
      for (int j = 0; j < num_of_rec_blocks; j++) {
        
        // Get the record block's id that is stored in the index block's cells in position j
        record_block_id = hash_cell[j];

        // Use the id to load the record block
        error = BF_GetBlock(file_descriptor,record_block_id,record_block);  

        // Get a pointer to the record block's data
        record_data = BF_Block_GetData(record_block);
        record_block_info = record_data;

        record = record_data + sizeof(Records_Block_Info);
        
        // Print all records stored in bucket
        for (int k = 0; k < record_block_info->record_num; k ++) {
          printRecord(record[k]);
        }

        // Unpin the record block
        error = BF_UnpinBlock(record_block);
      }

      // Unpin the index block
      error = BF_UnpinBlock(index_block);
    }
  }
  // If id isn't NULL print only the records with the specific id given
  else {

    // Initialize a block
    BF_Block* block;
    BF_Block_Init(&block);

    // Load the Header block
    BF_GetBlock(file_descriptor,0,block);
    void* data = BF_Block_GetData(block);

    // Use the File Header to get the global depth 
    HT_File_Header* header = data;

    // Calculate the hash value for the id
    unsigned int id_hash_value = hash(*id,header->global_depth);

    // Find index block that stores the pointer to the record block with the id given
    int index_block_capacity = header->index_block_capacity; 
    int index_block_num =  id_hash_value / index_block_capacity + 1; 
    int pos_in_index_block = id_hash_value % index_block_capacity;

    // If the Index block we want is the first one, the loop will be skipped
    int index_id = 1;
  
    // Access all the Index blocks until we find the one we're searching for
    for (int i = 1; i < index_block_num; i ++) {

      // Load each index block
      error = BF_GetBlock(file_descriptor,index_id,index_block);

      // Get a pointer to the Index block data
      data = BF_Block_GetData(index_block);
      index_block_info = data;

      // Get the next block's id
      index_id = index_block_info->next_block_id;

      // Unpin the block
      BF_UnpinBlock(index_block);
    }
  
    // Now that index_id stores the id of the index block we want, load said index block 
    BF_GetBlock(file_descriptor,index_id,index_block);

    data = BF_Block_GetData(index_block);

    // Get a pointer to the hash table cells
    int* index_cells = data + sizeof(Index_Block_Info);

    // Get the record block's id from the index block's cells
    record_block_id = index_cells[pos_in_index_block];

    // Load the record block
    BF_GetBlock(file_descriptor,record_block_id,record_block);

    data = BF_Block_GetData(record_block);

    // Get a pointer to the record block info
    record_block_info = data;
    
    // Find the number of records in the record block
    int record_number = record_block_info->record_num;

    // Get the pointer to the address where the records start
    record = data + sizeof(Records_Block_Info);
  
    for (int i= 0; i < record_number; i++) {

      if(record[i].id == *id) {
          printRecord(record[i]);
      }
    }

    // Unpin the Record block 
    error = BF_UnpinBlock(record_block);

    // Unpin the Index block 
    error = BF_UnpinBlock(index_block);

    // Unpin the Header Block
    error = BF_UnpinBlock(block);
    BF_Block_Destroy(&block);

  }
  // Destroy the blocks created to access the files records
  BF_Block_Destroy(&index_block);
  BF_Block_Destroy(&record_block);

  return HT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HT_ErrorCode HashStatistics(char* filename){
  
  int file_descriptor;
  BF_ErrorCode error;

  // A bool indicating if the file is open
  bool is_file_open = false;

  for (int i= 0; i < MAX_OPEN_FILES; i++) {

    if ( (File_Manager[i] != NULL) && (File_Manager[i]->file_name == filename) ) {

        // We get the file's descriptor from File_Mnager
        file_descriptor = File_Manager[i]->file_desc;
        is_file_open = true;

        // If we found the file we exit the loop
        break;
      }
  }

  int indexDesc;
  HT_ErrorCode HT_error;

  // If the file isn't open we open it
  if (!(is_file_open)) {
    
    // Open the file
    HT_error = HT_OpenIndex(filename, &indexDesc);

    // If opening the file was unsuccessful return HT_ERROR
    if (HT_error != HT_OK) {
      printf("Error in HashStatistics: Failed to open File\n");
      return HT_ERROR;
    }

    // Get the file descriptor
    file_descriptor = File_Manager[indexDesc]->file_desc;                     
  }

  int total_num_of_blocks;

  // Get the total number of blocks in the file
  error = BF_GetBlockCounter(file_descriptor,&total_num_of_blocks);

  printf("Total number of blocks in file: %s is %d\n",filename, total_num_of_blocks);

  // Initialize the structs and variables needed to access the index blocks
  BF_Block* index_block;
  BF_Block_Init(&index_block);
  Index_Block_Info* index_block_info;
  void* index_data;
  int* hash_cell ;

  // Initialize the structs and variables needed to access the record blocks
  BF_Block* record_block;
  BF_Block_Init(&record_block);
  Records_Block_Info* record_block_info;
  void* record_data;
  int record_block_id;

  // Initiliaze the counters
  int total_number_of_record_blocks = 0;
  int total_number_of_records = 0;

  int num_of_rec_blocks;

  // Initialize min and max with extreme values 
  int max_num_of_records = -1;
  int min_num_of_records = 11;

  // For every index block we access it's record blocks.
  // Start from the first block which is definitely an index block
  for (int i = 1; i != -1; i = index_block_info->next_block_id) {
    
    // Load the index block in block
    error = BF_GetBlock(file_descriptor,i,index_block);

    // If loading the index block was unsuccesful return HT_ERROR
    if (error!= BF_OK){
      printf( "Loading the block was unsuccesful\n");
      return HT_ERROR;
    }

    // Get the index block's data
    index_data = BF_Block_GetData(index_block);
    index_block_info = index_data;

    // Get a pointer to the position of the block where the hash table cells begin
    hash_cell = index_data + sizeof(Index_Block_Info);

    num_of_rec_blocks = index_block_info->num_of_record_blocks;

    // For every one of the record blocks stored in the index block access it's records
    for (int j = 0; j < num_of_rec_blocks; j++) {
      
      // Get the record block's id that is stored in the index block's cells in position j
      record_block_id = hash_cell[j];
  
      // If the cell of the index block points to a buddy that is not the first in order skip it 
      if ( (j>0) && (record_block_id == hash_cell[j-1]) )
        continue;
      
      // Increase the total number of record blocks by 1 
      total_number_of_record_blocks ++;
      
      // Use the id to load the record block
      error = BF_GetBlock(file_descriptor,record_block_id,record_block);  

      // Get a pointer to the block's data
      record_data = BF_Block_GetData(record_block);
      record_block_info = record_data;

      // For each record block add it's number of records to the total_number_of_records
      total_number_of_records += record_block_info->record_num;

      // Find the max and min number of records in any bucket
      if (record_block_info->record_num > max_num_of_records)
        max_num_of_records = record_block_info->record_num;
      
      if (record_block_info->record_num < min_num_of_records)
        min_num_of_records = record_block_info->record_num;

      // Unpin the record block
      error = BF_UnpinBlock(record_block);
    }
    // Unpin the index block
    error = BF_UnpinBlock(index_block);
  }
  // TESTING
  // printf("Number of record blocks %d\n",total_number_of_record_blocks);

  printf("Maximum number of records in block: %d\n", max_num_of_records);
  printf("Minimum number of records in block: %d\n",min_num_of_records);
  
  if(total_number_of_record_blocks > 0 )
    printf("Avarage number of records per block %0.2f\n" , (float) total_number_of_records/(float) total_number_of_record_blocks );
  else
    printf("Error : There aren't record blocks in the file\n");

  // Destroy the blocks created to access the file's records
  BF_Block_Destroy(&index_block);
  BF_Block_Destroy(&record_block);

  // If we opened the file, we close it when we're done 
  if (!is_file_open) {
    HT_CloseFile(indexDesc);
  }

  return HT_OK;
}