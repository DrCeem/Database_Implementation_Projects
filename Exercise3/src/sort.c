#include "sort.h"


bool shouldSwap(Record* rec1,Record* rec2) {
    
    // Compare names
    int nameCompare = strcmp(rec1->name, rec2->name);

    if (nameCompare < 0) {
        // rec1's name comes before rec2's name, no need to swap
        return false;
    } else if (nameCompare > 0) {
        // rec1's name comes after rec2's name, swap needed
        return true;
    } else {
        // Names are the same, compare surnames
        int surnameCompare = strcmp(rec1->surname, rec2->surname);

        if (surnameCompare < 0) {
            // rec1's surname comes before rec2's surname, no need to swap
            return false;
        } else if (surnameCompare > 0) {
            // rec1's surname comes after rec2's surname, swap needed
            return true;
        } else {
            // Both name and surname are the same, no swap needed
            return false;
        }
    }
}

// Function to sort each one of the file's chunks
void sort_FileInChunks(int file_desc, int numBlocksInChunk) {

    // Create a CHUNK iterator for efficient traversal of chunks within the file
    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);

    // Initialize a pointer to a chunk struct
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
    
    sort_Chunk(chunk);

    // Iterate through chunks in the file
    while (CHUNK_GetNext(&iterator, chunk) != -1) {
        // Sort each chunk using the provided sort_Chunk function
        sort_Chunk(chunk);
    }
    free (chunk);

}

// Helper function for partitioning in quicksort
int partition(Record arr[], int low, int high) {
    
    Record pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (!(shouldSwap(&arr[j], &pivot)) ){
            i++;
            // Swap arr[i] and arr[j]
            Record temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    // Swap arr[i+1] and arr[high] (pivot)
    Record temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;

    return i + 1;
}

// Quicksort function
void quicksort(Record arr[], int low, int high) {

    if (low < high) {
        int pi = partition(arr, low, high);

        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

// Sort function for CHUNK using quicksort
void sort_Chunk(CHUNK* chunk) {

    int totalRecords = chunk->recordsInChunk;

    Record* records = malloc(sizeof(Record) * totalRecords);

    // Retrieve all records in the chunk
    for (int i = 0; i < totalRecords; i++) {
        CHUNK_GetIthRecordInChunk(chunk, i + 1, &records[i]);
        // TESTING
        // printRecord(records[i]);
    }

    // Apply quicksort
    quicksort(records, 0, totalRecords - 1);

    // Update records in the chunk
    for (int i = 0; i < totalRecords; i++) {
        CHUNK_UpdateIthRecord(chunk, i + 1, records[i]);
        // TESTING
        // printRecord(records[i]);
    }

    free(records);
}