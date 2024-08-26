#ifndef SORT_H
#define SORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "chunk.h"
#include "record.h"
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "merge.h"
#include "chunk.h"


/* Determines if two records should be swapped during sorting, returning true if the order needs adjustment.*/
bool shouldSwap(Record* rec1,Record* rec2);

/* Sorts the contents of a file identified by 'file_desc' in chunks, where each chunk contains 'numBlocksInChunk' blocks. The sorting is performed in-place within each chunk, using an appropriate sorting algorithm.*/
void sort_FileInChunks(int file_desc, int numBlocksInChunk);

/* Sorts records within a CHUNK in ascending order based on the name and surname of each person. */
void sort_Chunk(CHUNK* chunk);


// New Functions used for sorting
int partition(Record arr[], int low, int high);

void quicksort(Record arr[], int low, int high);

#endif 