//https://github.com/zchsu/cd2025
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char character;
    int count;
    struct Node *next;
} Node;

// Search for the character in the linked list, increment count if found, otherwise add a new node
void insertOrUpdate(Node **head, char ch) {
    Node *current = *head;
    Node *prev = NULL;

    while (current) {
        if (current->character == ch) {
            current->count++;
            return;
        }
        prev = current;
        current = current->next;
    }

    // If the character does not exist in the linked list, create a new node
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->character = ch;
    newNode->count = 1;
    newNode->next = NULL;

    if (prev)
        prev->next = newNode;
    else
        *head = newNode;
}

// Free the memory of the linked list
void freeList(Node *head) {
    Node *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// Print the contents of the linked list
void printList(Node *head) {
    while (head) {
        printf("%c: %d\n", head->character, head->count);
        head = head->next;
    }
}

int main() {
    FILE *file = fopen(__FILE__, "r"); // Read its own source code
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    Node *head = NULL;
    char ch;

    while ((ch = fgetc(file)) != EOF) {
        insertOrUpdate(&head, ch);
    }

    fclose(file);

    // Print the result
    printList(head);

    // Free allocated memory
    freeList(head);

    return 0;
}
