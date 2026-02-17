#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define RESET "\033[0m"

int brace = 0;

char *keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else",
    "enum", "extern", "float", "for", "goto", "if", "int", "long", "register", "return",
    "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
    "unsigned", "void", "volatile", "while"};

int isKeyword(char *tok)
{
    for (int i = 0; i < 32; i++)
        if (strcmp(tok, keywords[i]) == 0)
            return 1;
    return 0;
}

int isOperator(char c)
{
    char ops[] = "()+-*/%=[]{}&|^!~<>.,;:";
    for (int i = 0; ops[i]; i++)
        if (c == ops[i])
            return 1;
    return 0;
}

void classify(char *tok)
{
    if (isKeyword(tok))
        printf(GREEN "KEYWORD -> %s\n" RESET, tok);

    else if (isdigit(tok[0]))
        printf(MAGENTA "NUMBER -> %s\n" RESET, tok);

    else
        printf(BLUE "IDENTIFIER -> %s\n" RESET, tok);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s file.c\n", argv[0]);
        return 0;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp)
    {
        perror("file");
        return 1;
    }

    char line[1024];
    char tok[256];
    int ti = 0;

    int in_string = 0;
    int in_char = 0;
    int in_comment = 0;
    int comment_start = 0;
    int lineNo = 0;

    while (fgets(line, sizeof(line), fp))
    {
        lineNo++;

        int paren = 0, bracket = 0;

        for (int i = 0; line[i]; i++)
        {
            char c = line[i];
            char n = line[i + 1];

            /* ===== BLOCK COMMENT MODE ===== */
            if (in_comment)
            {
                if (c == '*' && n == '/')
                {
                    in_comment = 0;
                    i++;
                }
                continue;
            }

            /* start comment */
            if (!in_string && !in_char && c == '/' && n == '*')
            {
                in_comment = 1;
                comment_start = lineNo;
                i++;
                continue;
            }

            /* single line comment */
            if (!in_string && !in_char && c == '/' && n == '/')
                break;

            /* string */
            if (!in_char && c == '"')
            {
                in_string = !in_string;
                continue;
            }

            /* char */
            if (!in_string && c == '\'')
            {
                in_char = !in_char;
                continue;
            }

            /* escape */
            if ((in_string || in_char) && c == '\\')
            {
                i++;
                continue;
            }

            if (in_string || in_char)
                continue;

            /* token build */
            if (isalnum(c) || c == '_')
            {
                tok[ti++] = c;
                continue;
            }

            if (ti)
            {
                tok[ti] = '\0';
                classify(tok);
                ti = 0;
            }

            /* operators */
            if (isOperator(c))
            {
                printf(YELLOW "OPERATOR -> %c\n" RESET, c);

                if (c == '(')
                    paren++;
                if (c == ')')
                {
                    if (--paren < 0)
                    {
                        printf(RED "Line %d unexpected ')'\n" RESET, lineNo);
                        exit(1);
                    }
                }

                if (c == '[')
                    bracket++;
                if (c == ']')
                {
                    if (--bracket < 0)
                    {
                        printf(RED "Line %d unexpected ']'\n" RESET, lineNo);
                        exit(1);
                    }
                }

                if (c == '{')
                    brace++;
                if (c == '}')
                {
                    if (--brace < 0)
                    {
                        printf(RED "Line %d unexpected '}'\n" RESET, lineNo);
                        exit(1);
                    }
                }
            }
        }

        if (in_string)
        {
            printf(RED "Line %d: unterminated string\n" RESET, lineNo);
            exit(1);
        }
        if (in_char)
        {
            printf(RED "Line %d: unterminated char\n" RESET, lineNo);
            exit(1);
        }
        if (paren || bracket)
        {
            printf(RED "Line %d: unclosed () or []\n" RESET, lineNo);
            exit(1);
        }
    }

    fclose(fp);

    /* FINAL COMMENT CHECK */
    if (in_comment)
    {
        printf(RED "Error: comment started at line %d not closed\n" RESET, comment_start);
        return 1;
    }

    /* FINAL BRACE CHECK */
    if (brace)
    {
        printf(RED "Error: unclosed {} count = %d\n" RESET, brace);
        return 1;
    }

    printf(GREEN "\nNo syntax errors detected\n" RESET);
}
