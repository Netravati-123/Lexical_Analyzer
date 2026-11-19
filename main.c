#include <stdio.h>
#include "lexical.h"

int main(int argc, char *argv[])
{
    FILE *fp;

    if(argc == 2)
        fp = fopen(argv[1], "r");
    else
        fp = fopen("sample.c", "r");

    if(fp == NULL)
    {
        printf("Unable to open file\n");
        return 1;
    }

    printf("LEXICAL ANALYZER OUTPUT\n\n");

    lexical_analyzer(fp);

    fclose(fp);

    return 0;
}