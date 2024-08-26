// gcc -o test test.c


// #include <stdio.h>

// // void printInDecimalForm(unsigned int num) {
// //     int i;

// //     // Print exactly 32 bits
// //     for (i = 31; i >= 0; i--) {
// //         // Check if the i-th bit is set
// //         if ((num >> i) & 1) {
// //             printf("1");
// //         } else {
// //             printf("0");
// //         }

// //         // Print a space between groups of 4 bits for better readability
// //         if (i % 4 == 0) {
// //             printf(" ");
// //         }
// //     }

// //     printf("\n");
// // }

// // int main() {
// //     unsigned int number;

// //     // Get input from the user
// //     printf("Enter an unsigned integer: ");
// //     scanf("%u", &number);

// //     // Print the number in decimal form with leading zeros
// //     printf("Decimal form with leading zeros: ");
// //     printInDecimalForm(number);

// //     return 0;
// // }

// // #include <stdio.h>

// // void printBinaryFormatted(unsigned long long number) {
// //     for (int i = 63; i >= 0; i--) {
// //         printf("%c", (number & (1ULL << i)) ? '1' : '0');
// //         if (i % 4 == 0)
// //             printf(" ");
// //     }
// //     printf("\n");
// // }

// // int main() {
    
// //     unsigned long long number = 2; // 0000 0000 0000 0000 0000 0000 0000 0010
// //     printf("Initial number: ");
// //     printBinaryFormatted(number);

// //     while ((number & (1ULL << 63)) == 0) { // Check if the first bit is not 1
// //         number <<= 1; // Shift left by 1
// //     }

// //     printf("Shifted number in binary form: ");
// //     printBinaryFormatted(number);

// //     return 0;
// // }

// // struct File_Info **File_Manager; // Διπλός δείκτης προς δομή File_Info

// // HT_ErrorCode HT_Init() {
// //     File_Manager = (struct File_Info **)malloc(MAX_OPEN_FILES * sizeof(struct File_Info *));
    
// //     if (File_Manager == NULL) {
// //         printf("Allocation Failed\n");
// //         return HT_ERROR;
// //     }

// //     // Αρχικοποίηση των δεικτών σε NULL
// //     for (int i = 0; i < MAX_OPEN_FILES; ++i) {
// //         File_Manager[i] = NULL;
// //         // printf("Check\n");
// //     }

// //     return HT_OK;
// // }
// // HT_ErrorCode HT_Close() {

// //   for (int i = 0; i < MAX_OPEN_FILES; ++i) {
// //         free(File_Manager[i]);
// //         // printf("Check\n");
// //     }
// //   free(File_Manager);
// //   return HT_OK; 
// // }

//  // int* test_array = malloc(4*sizeof(int));
//   // test_array[0] = 1;
//   // test_array[1]= 2;
//   // test_array[2] = 3;
//   // test_array[3] = 4;
//   // int* new_array = resize_array(4,test_array);
//   // for (int i = 0; i < 8; i++)
//   //   printf("%d \t %d\n", i ,new_array[i]);

//   // free(new_array);
  
// // hash sttistcs





// //   do {

// //     // Load the index block in block
// //     error = BF_GetBlock(file_descriptor,id,index_block);
// //     // If loading the index block was unsuccesful return HT_ERROR
// //     if (error!= BF_OK){
// //       printf( "Loading the block was unsuccesful\n");
// //       return HT_ERROR;
// //     }
// //     // Get the blocks data
// //     data = BF_Block_GetData(index_block);
// //     index_block_info = data;
// //     printf("id = %d, next block id = %d\n",id,index_block_info->next_block_id);

// //     // Get a pointer to the position of the block where the hash table cells begin
// //     hash_cell = data + sizeof(Index_Block_Info);

// //     printf("number of records blocks in block %d is %d\n",id,index_block_info->num_of_record_blocks);
// //     // For every one of the record blocks stored in the index block access it's records
// //     for (int j = 0; j < index_block_info->num_of_record_blocks; j++) {

// //       // Increase the total number of recorf blocks by 1
// //       total_number_of_record_blocks ++;

// //       // Get the record block's id that is stored in the index block's cells in position j
// //       record_block_id = hash_cell[j];

// //       printf("Record block's id = %d\n", record_block_id);
// //       // Use the id to load the record block
// //       error = BF_GetBlock(file_descriptor,record_block_id,record_block);  

// //       if (error != BF_OK){
// //         printf("Error in loading block %d\n",record_block_id);
// //         return HT_ERROR;
// //       }
// //       // Get a pointer to the blocks data
// //       void* data = BF_Block_GetData(record_block);
// //       record_block_info = data;

// //       // For each record block add it's number of records to the total_number_of_records
// //       total_num_of_blocks += record_block_info->record_num;

// //       if (record_block_info->record_num > max_num_of_records)
// //         max_num_of_records = record_block_info->record_num;
      
// //       if (record_block_info->record_num < min_num_of_records)
// //         min_num_of_records = record_block_info->record_num;

// //       // Unpin the record block
// //       error = BF_UnpinBlock(record_block);
// //       if (error != BF_OK){
// //         printf("Error in unpining block %d\n",record_block_id);
// //         return HT_ERROR;
// //       }
// //     }
// //     id = index_block_info->next_block_id;
// //     // Unpin the index block
// //     error = BF_UnpinBlock(index_block);
// //     if (error != BF_OK){
// //         printf("Error in unpining block \n");
// //         return HT_ERROR;
// //       }
// // }while(id != -1);
    

// #include <stdio.h>
// #include <math.h>

// void printBinaryFormatted(unsigned long long number) {
//     for (int i = 63; i >= 0; i--) {
//         printf("%c", (number & (1ULL << i)) ? '1' : '0');
//         if (i % 4 == 0)
//             printf(" ");
//     }
//     printf("\n");
// }

// unsigned long long shiftLeftAndKeep(unsigned long long number, int k) {
//     return number >> (64 - k);
// }

// // Takes a number and swifts it to the left until the first bit is 1
// unsigned long long Shift_Left(unsigned long long number) {

//     while ((number & (1ULL << 63)) == 0) {
//         number <<= 1;
//     }
//     return number;
// }

// int main() {
//     unsigned long long number;
    
//     printf("Enter a number: ");
//     scanf("%llu", &number);

//     printf("Initial number in binary form: ");
//     printBinaryFormatted(number);

//     number = Shift_Left(number);

//     printf("Shifted number in binary form: ");
//     printBinaryFormatted(number);

//     // Convert shifted number to decimal
//     printf("Shifted number in decimal form: %llu\n", number);

//     int k;
//     printf("Enter the number of most significant digits to keep: ");
//     scanf("%d", &k);

//     unsigned long long result = shiftLeftAndKeep(number, k);
//     printf("Result keeping %d most significant digits: %llu\n", k, result);

//     // Convert result to decimal
//     printf("Result in decimal form: %llu\n", result);

//     return 0;
// }


// /*GIA README

// αφορμη για αυτο με το shift το σαιτ https://copyprogramming.com/howto/what-is-the-meaning-of-1ull-in-c-programming-language-duplicate#what-is-1ul



// */

// // // Keeps the k-first bits of the value given
// // unsigned int k_first_bits(unsigned int number, int k) {

// //     // Create a mask with the first k bits set to 1
// //     unsigned int mask = (1 << k) - 1;

// //     // Keep only the first k bits
// //     unsigned int result = number & mask;  

// //     return result;
// // }

// // // Get the k-least-significant bits of each number. Always returns a positive integer in [0 , 2^k - 1]
// // unsigned int k_first_bits(unsigned int value,unsigned int k ) {
// //   value =  value << (sizeof(unsigned int) * 8 - k);
// //   return value >> (sizeof(unsigned int) * 8 - k );
// // }

// //

#include <stdio.h>

typedef struct Record {
    int id;
    char name[15];
    char surname[20];
    char city[20];
} Record;

void printRecord(Record record) {

    printf("Id: %d, Name: %s, Surname: %s, City: %s\n", record.id,record.name,record.surname,record.city);

}

int main() {
    Record rec = {1, "John", "Doe", "New York"};

    // Printing the record
    printRecord(rec);

    return 0;
}
