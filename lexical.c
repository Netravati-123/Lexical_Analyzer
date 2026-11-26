#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexical.h"

/* Check keywords */
int is_keyword(char word[])
{
    char *keywords[] =
    {
        "auto", "break", "case", "char", "const",
        "continue", "default", "do", "double",
        "else", "enum", "extern", "float", "for",
        "goto", "if", "int", "long", "register",
        "return", "short", "signed", "sizeof",
        "static", "struct", "switch", "typedef",
        "union", "unsigned", "void", "volatile",
        "while", "_Bool"
    };

    int size = sizeof(keywords) / sizeof(keywords[0]);

    for(int i = 0; i < size; i++)
    {
        if(strcmp(word, keywords[i]) == 0)
            return 1;
    }

    return 0;
}

/* Check operators */
int is_operator(int ch)
{
    if(ch == '+' || ch == '-' || ch == '*' ||
       ch == '/' || ch == '%' || ch == '=' ||
       ch == '!' || ch == '<' || ch == '>' ||
       ch == '&' || ch == '|' || ch == '^' ||
       ch == '~' || ch == '?')
    {
        return 1;
    }

    return 0;
}

/* Identify single, double and triple operators */
void print_operator(FILE *fp, int ch)
{
    char op[4] = {0};
    int next, third;

    op[0] = ch;

    next = fgetc(fp);

    if(next == EOF)
    {
        printf("%s -> OPERATOR\n", op);
        return;
    }

    /* Three-character operators */
    if((ch == '<' || ch == '>') && next == ch)
    {
        third = fgetc(fp);

        if(third == '=')
        {
            op[1] = next;
            op[2] = third;
        }
        else
        {
            op[1] = next;

            if(third != EOF)
                ungetc(third, fp);
        }
    }

    /* Two-character operators */
    else if((ch == '+' && (next == '+' || next == '=')) ||
            (ch == '-' && (next == '-' || next == '=' || next == '>')) ||
            ((ch == '*' || ch == '/' || ch == '%') && next == '=') ||
            (ch == '=' && next == '=') ||
            (ch == '!' && next == '=') ||
            ((ch == '<' || ch == '>') && next == '=') ||
            (ch == '&' && (next == '&' || next == '=')) ||
            (ch == '|' && (next == '|' || next == '=')) ||
            (ch == '^' && next == '='))
    {
        op[1] = next;
    }
    else
    {
        ungetc(next, fp);
    }

    printf("%s -> OPERATOR\n", op);
}

/* Main lexical analyzer */
void lexical_analyzer(FILE *fp)
{
    int ch, next;
    int i, dot_count, invalid;

    char word[200];

    while((ch = fgetc(fp)) != EOF)
    {
        /* Skip white spaces */
        if(isspace((unsigned char)ch))
        {
            continue;
        }

        /* Preprocessor directives */
        else if(ch == '#')
        {
            i = 0;
            word[i++] = ch;

            while((ch = fgetc(fp)) != EOF && ch != '\n')
            {
                if(i < 199)
                    word[i++] = ch;
            }

            word[i] = '\0';

            printf("%s -> PREPROCESSOR DIRECTIVE\n", word);
        }

        /* Comments */
        else if(ch == '/')
        {
            next = fgetc(fp);

            /* Single-line comment */
            if(next == '/')
            {
                while((ch = fgetc(fp)) != EOF && ch != '\n')
                {
                }
            }

            /* Multi-line comment */
            else if(next == '*')
            {
                int previous = 0;
                int found = 0;

                while((ch = fgetc(fp)) != EOF)
                {
                    if(previous == '*' && ch == '/')
                    {
                        found = 1;
                        break;
                    }

                    previous = ch;
                }

                if(found == 0)
                    printf("INVALID COMMENT\n");
            }
            else
            {
                if(next != EOF)
                    ungetc(next, fp);

                print_operator(fp, '/');
            }
        }

        /* Identifiers and keywords */
        else if(isalpha((unsigned char)ch) || ch == '_')
        {
            i = 0;

            while(isalnum((unsigned char)ch) || ch == '_')
            {
                if(i < 199)
                    word[i++] = ch;

                ch = fgetc(fp);

                if(ch == EOF)
                    break;
            }

            word[i] = '\0';

            if(ch != EOF)
                ungetc(ch, fp);

            if(is_keyword(word))
                printf("%s -> KEYWORD\n", word);
            else
                printf("%s -> IDENTIFIER\n", word);
        }

        /* Integer and floating-point constants */
        else if(isdigit((unsigned char)ch))
        {
            i = 0;
            dot_count = 0;
            invalid = 0;

            while(isalnum((unsigned char)ch) ||
                  ch == '_' || ch == '.')
            {
                if(ch == '.')
                    dot_count++;

                if(isalpha((unsigned char)ch) || ch == '_')
                    invalid = 1;

                if(i < 199)
                    word[i++] = ch;
                else
                    invalid = 1;

                ch = fgetc(fp);

                if(ch == EOF)
                    break;
            }

            word[i] = '\0';

            if(ch != EOF)
                ungetc(ch, fp);

            if(invalid)
                printf("%s -> INVALID IDENTIFIER\n", word);
            else if(dot_count > 1)
                printf("%s -> INVALID CONSTANT\n", word);
            else
                printf("%s -> CONSTANT\n", word);
        }

        /* Character constants */
        else if(ch == '\'')
        {
            char literal[200];
            int length = 0;
            int content_count = 0;
            int valid = 1;
            int closed = 0;

            literal[length++] = ch;

            while((next = fgetc(fp)) != EOF)
            {
                if(next == '\n')
                    break;

                if(next == '\'')
                {
                    literal[length++] = next;
                    closed = 1;
                    break;
                }

                if(next == '\\')
                {
                    literal[length++] = next;

                    next = fgetc(fp);

                    if(next == EOF || next == '\n')
                        break;

                    if(length < 198)
                        literal[length++] = next;

                    content_count++;
                }
                else
                {
                    if(length < 198)
                        literal[length++] = next;

                    content_count++;
                }
            }

            literal[length] = '\0';

            if(content_count != 1 || closed == 0)
                valid = 0;

            if(valid)
                printf("%s -> CHARACTER CONSTANT\n", literal);
            else
                printf("%s -> INVALID CHARACTER CONSTANT\n", literal);
        }

        /* String constants */
        else if(ch == '"')
        {
            char literal[200];
            int length = 0;
            int closed = 0;

            literal[length++] = ch;

            while((next = fgetc(fp)) != EOF)
            {
                if(next == '\n')
                    break;

                if(length < 198)
                    literal[length++] = next;

                if(next == '\\')
                {
                    next = fgetc(fp);

                    if(next == EOF || next == '\n')
                        break;

                    if(length < 198)
                        literal[length++] = next;
                }
                else if(next == '"')
                {
                    closed = 1;
                    break;
                }
            }

            literal[length] = '\0';

            if(closed)
                printf("%s -> STRING CONSTANT\n", literal);
            else
                printf("%s -> INVALID STRING CONSTANT\n", literal);
        }

        /* Operators */
        else if(is_operator(ch))
        {
            print_operator(fp, ch);
        }

        /* Special symbols */
        else if(ch == '(' || ch == ')' ||
                ch == '{' || ch == '}' ||
                ch == '[' || ch == ']' ||
                ch == ';' || ch == ',' ||
                ch == ':' || ch == '.')
        {
            printf("%c -> SPECIAL SYMBOL\n", ch);
        }

        /* Invalid characters */
        else
        {
            printf("%c -> INVALID CHARACTER\n", ch);
        }
    }
}