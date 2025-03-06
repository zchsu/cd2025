//Bonus:

//Using a hash table,  reduce the search time complexity to O(1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 256 

typedef struct {
    int count[HASH_SIZE];  
} HashTable;

void initHashTable(HashTable *table) {
    memset(table->count, 0, sizeof(table->count));
}

void insertOrUpdate(HashTable *table, char ch) {
    table->count[(unsigned char)ch]++;  
}

void printHashTable(HashTable *table) {
    for (int i = 0; i < HASH_SIZE; i++) {
        if (table->count[i] > 0) {
            printf("%c: %d\n", i, table->count[i]);
        }
    }
}

int main() {
    FILE *file = fopen(__FILE__, "r");  
    if (!file) {
        perror("can not open file");
        return 1;
    }

    HashTable table;
    initHashTable(&table);

    char ch;
    while ((ch = fgetc(file)) != EOF) {
        insertOrUpdate(&table, ch);
    }

    fclose(file);

    printHashTable(&table);

    return 0;
}
