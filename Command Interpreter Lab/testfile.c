#include <stdio.h>
#include "ci.h"

int main(void) {
    char *str = malloc(5);
    str = "hell";
    int result = strrev(str);
    printf("Result: " + result);
    return 0;
}