#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    char value;
    struct node *next;
} node_t;

int main(int argc, char *argv[]) 
{ 
//    long keys[][2] = {{688, 5472316177786122208}, {634, 10753594719449760}, {499, 2975631023800051}, {608, 9556776748275712}, {575, 6035219295902900}, {613, 10915442737423189}};
    long arg;
    char c;

    node_t *start = NULL;
    start = malloc(sizeof(node_t));
    node_t *t = start;

    for (size_t i = 0; i < 5; i++) {
        t->next = malloc(sizeof(node_t));
        t = t->next;
    }
    t->next = start;
    t = start;
    int base;
    long n;
 
    for (size_t i = 0; i < 6; i++) {
        scanf("%d", &base);
        scanf("%ld", &n);
        while (n != 0) {
            c = n % base;
            n /= base;
            t->value = c;
            t = t->next;
        }
    }
    return 0;
}
