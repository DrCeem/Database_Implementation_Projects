#include<stdbool.h>
#include <math.h>
#ifndef HASH_FILE_H
#define HASH_FILE_H

typedef enum HT_ErrorCode {
  HT_OK,
  HT_ERROR
} HT_ErrorCode;

typedef struct Record {
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;

// A structure containing information about the relation of the HT File with it's corresponding BF File ++++
typedef struct File_Info {
    int file_desc;					// The file descriptor
	char* file_name;				// A pointer to the file name
} File_Info;

// A structure containing information about the Extended Hashing File 
typedef struct HT_File_Header {
    bool isHT;						// A bool declaring if the file is an Extended Hash Table file
    char* hashing_item;				// A char that declares the Record item that is used for hashing
	int global_depth;				// The global depth
	int max_num_of_rec_in_bucket;	// The maximum number of records that can be stored in a bucket (Record Block)
	int index_block_capacity;		// The maximum number of of "pointers" (BF blocks ids) to records blocks that can be stored in an Index Block
} HT_File_Header;

// A structure containing information about an Index block
typedef struct Index_Block_Info {
    int next_block_id ;				// Stores the id of the next Index Block, or -1 if it's the last one
	int num_of_record_blocks; 		// The number of "pointers" (BF blocks ids) to records blocks currently stored in the Index block
} Index_Block_Info;

// A structure containing information about a Record block
typedef struct Records_Block_Info {
    int record_num; 			    // Number of records currently in the Record block (bucket)
    int local_depth;    			// The local depth of the Record block (bucket)
} Records_Block_Info;

// A custom power function
float powerCustom(float base, float exponent);

// The hash function that returns the value multiplied by a prime number 
unsigned int hash(unsigned int  id, int depth);

// Doubles the size of the array and updates the pointers to the buckets/blocks
int* resize_array(int size, int* array);

// Prints the record given
void printRecord(Record record);

// Takes a record block id and returns the index block where it's first buddy is stored at in first_buddy_index_block,
// as well as the first buddy's position in said index block in first_buddy_pos. Additionally returns the number of buddies in count
HT_ErrorCode findBuddies(int file_desc,int record_block_id,int* first_buddy_index_block, int* first_buddy_pos,int* count);

// A function used from printing all the blocks for TESTING purposes 
void* Check(int file_desc);

/*
 * Η συνάρτηση HT_Init χρησιμοποιείται για την αρχικοποίηση κάποιον δομών που μπορεί να χρειαστείτε. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_Init();

/*
 * Η συνάρτηση HT_Close χρησιμοποιείται για την αποδέσμευση των δομών που αρχικοποιήθηκα στην HT_Init
 * Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_Close();

/*
 * Η συνάρτηση HT_CreateIndex χρησιμοποιείται για τη δημιουργία και κατάλληλη αρχικοποίηση ενός άδειου αρχείου κατακερματισμού με όνομα fileName. 
 * Στην περίπτωση που το αρχείο υπάρχει ήδη, τότε επιστρέφεται ένας κωδικός λάθους. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HΤ_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_CreateIndex(
	const char *fileName,		/* όνομααρχείου */
	int depth
	);


/*
 * Η ρουτίνα αυτή ανοίγει το αρχείο με όνομα fileName. 
 * Εάν το αρχείο ανοιχτεί κανονικά, η ρουτίνα επιστρέφει HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_OpenIndex(
	const char *fileName, 		/* όνομα αρχείου */
  int *indexDesc            /* θέση στον πίνακα με τα ανοιχτά αρχεία  που επιστρέφεται */
	);

/*
 * Η ρουτίνα αυτή κλείνει το αρχείο του οποίου οι πληροφορίες βρίσκονται στην θέση indexDesc του πίνακα ανοιχτών αρχείων.
 * Επίσης σβήνει την καταχώρηση που αντιστοιχεί στο αρχείο αυτό στον πίνακα ανοιχτών αρχείων. 
 * Η συνάρτηση επιστρέφει ΗΤ_OK εάν το αρχείο κλείσει επιτυχώς, ενώ σε διαφορετική σε περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_CloseFile(
	int indexDesc 		/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	);

/*
 * Η συνάρτηση HT_InsertEntry χρησιμοποιείται για την εισαγωγή μίας εγγραφής στο αρχείο κατακερματισμού. 
 * Οι πληροφορίες που αφορούν το αρχείο βρίσκονται στον πίνακα ανοιχτών αρχείων, ενώ η εγγραφή προς εισαγωγή προσδιορίζεται από τη δομή record. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
HT_ErrorCode HT_InsertEntry(
	int indexDesc,	/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	Record record		/* δομή που προσδιορίζει την εγγραφή */
	);

/*
 * Η συνάρτηση HΤ_PrintAllEntries χρησιμοποιείται για την εκτύπωση όλων των εγγραφών που το record.id έχει τιμή id. 
 * Αν το id είναι NULL τότε θα εκτυπώνει όλες τις εγγραφές του αρχείου κατακερματισμού. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HP_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
HT_ErrorCode HT_PrintAllEntries(
	int indexDesc,	/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	int *id 				/* τιμή του πεδίου κλειδιού προς αναζήτηση */
	);

/* 
Η συνάρτηση διαβάζει το αρχείο με όνομα filename και τυπώνει στατιστικά στοιχεία: α) 
Το πόσα blocks έχει ένα αρχείο, β) Το ελάχιστο, το μέσο και το μέγιστο πλήθος εγγραφών 
που έχει κάθε bucket ενός αρχείου, Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται 
HT_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
*/
HT_ErrorCode HashStatistics(char* filename);

#endif // HASH_FILE_H