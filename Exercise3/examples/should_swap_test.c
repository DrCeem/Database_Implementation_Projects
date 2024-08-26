#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"

//Test for shouldSwap function 
//Compile with: make should_swap_test
//Run with: ./build/should_swap_test

int main() {

    // Create and display two random records
    Record rec1 = randomRecord();
    Record rec2 = randomRecord();

    printf("Record 1:\n");
    printf("Name: %s\nSurname: %s\nCity: %s\nID: %d\nDelimiter: %s\n\n", rec1.name, rec1.surname, rec1.city, rec1.id, rec1.delimiter);

    printf("Record 2:\n");
    printf("Name: %s\nSurname: %s\nCity: %s\nID: %d\nDelimiter: %s\n\n", rec2.name, rec2.surname, rec2.city, rec2.id, rec2.delimiter);

    // Test the shouldSwap function
    if (shouldSwap(&rec1, &rec2)) {
        printf("Swap needed!\n");
    } else {
        printf("No swap needed.\n");
    }

    // Create two records with the same name
    Record rec3;
    Record rec4;
    strncpy(rec3.name, "SameName", sizeof(rec3.name));
    strncpy(rec3.surname, "Surname1", sizeof(rec3.surname));
    strncpy(rec3.city, "City1", sizeof(rec3.city));
    rec3.id = 123;
    strncpy(rec3.delimiter, "|", sizeof(rec3.delimiter));

    strncpy(rec4.name, "SameName", sizeof(rec4.name));
    strncpy(rec4.surname, "Surname2", sizeof(rec4.surname));
    strncpy(rec4.city, "City2", sizeof(rec4.city));
    rec4.id = 456;
    strncpy(rec4.delimiter, "|", sizeof(rec4.delimiter));

    // Display the records with the same name
    printf("\nRecord 3 (Same Name as Record 4):\n");
    printf("Name: %s\nSurname: %s\nCity: %s\nID: %d\nDelimiter: %s\n\n", rec3.name, rec3.surname, rec3.city, rec3.id, rec3.delimiter);

    printf("Record 4 (Same Name as Record 3):\n");
    printf("Name: %s\nSurname: %s\nCity: %s\nID: %d\nDelimiter: %s\n\n", rec4.name, rec4.surname, rec4.city, rec4.id, rec4.delimiter);

    // Test the shouldSwap function for records with the same name
    if (shouldSwap(&rec3, &rec4)) {
        printf("Swap needed!\n");
    } else {
        printf("No swap needed.\n");
    }

    return 0;

}
