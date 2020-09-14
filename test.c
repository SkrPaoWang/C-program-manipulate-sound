#include <stdio.h>
#include <stdlib.h>

void decimal_to_hex(short value, char* hex){
    sprintf(hex, "%04x",  value);
}

int main()
{
    int x = 3;
    char *hex = malloc(sizeof(char)*6);
    decimal_to_hex(x,hex);
    printf("%s\n",hex);    
}
