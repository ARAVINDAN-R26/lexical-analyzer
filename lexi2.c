#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"

int paren = 0, bracket = 0, brace = 0;

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
    {
        for (int i = 0; tok[i]; i++)
            if (!isdigit(tok[i]))
            {
                printf(RED "INVALID NUMBER -> %s\n" RESET, tok);
                return;
            }
        printf(MAGENTA "NUMBER -> %s\n" RESET, tok);
    }
    else if (isalpha(tok[0]) || tok[0] == '_')
    {
        for (int i = 1; tok[i]; i++)
            if (!isalnum(tok[i]) && tok[i] != '_')
            {
                printf(RED "INVALID IDENTIFIER -> %s\n" RESET, tok);
                return;
            }
        printf(BLUE "IDENTIFIER -> %s\n" RESET, tok);
    }
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
        perror("File");
        return 1;
    }

    char line[1024];
    char tok[256];
    int ti = 0;

    int in_string = 0;
    int in_char = 0;
    int in_comment = 0;

    while (fgets(line, sizeof(line), fp))
    {
        int only_space_before = 1;

        for (int i = 0; line[i]; i++)
        {
            char c = line[i];
            char next = line[i + 1];

            if (!isspace(c))
                only_space_before = 0;

            /* ---------- block comment ---------- */
            if (in_comment)
            {
                if (c == '*' && next == '/')
                {
                    in_comment = 0;
                    i++;
                }
                continue;
            }

            if (!in_string && !in_char && c == '/' && next == '*')
            {
                in_comment = 1;
                i++;
                continue;
            }

            /* ---------- single comment ---------- */
            if (!in_string && !in_char && c == '/' && next == '/')
                break;

            /* ---------- preprocessor ---------- */
            if (!in_string && !in_char && only_space_before && c == '#')
                break;

            /* ---------- string ---------- */
            if (!in_char && c == '"')
            {
                in_string = !in_string;
                continue;
            }

            /* ---------- char ---------- */
            if (!in_string && c == '\'')
            {
                in_char = !in_char;
                continue;
            }

            /* ---------- escape inside literal ---------- */
            if ((in_string || in_char) && c == '\\')
            {
                i++; /* skip escaped char */
                continue;
            }

            if (in_string || in_char)
                continue;

            /* ---------- identifier/number ---------- */
            if (isalnum(c) || c == '_')
            {
                tok[ti++] = c;
                continue;
            }

            /* flush token */
            if (ti)
            {
                tok[ti] = '\0';
                classify(tok);
                ti = 0;
            }

            /* ---------- operators ---------- */
            if (isOperator(c))
            {
                printf(YELLOW "OPERATOR -> %c\n" RESET, c);

                if (c == '(')
                    paren++;
                if (c == ')')
                    paren--;
                if (c == '[')
                    bracket++;
                if (c == ']')
                    bracket--;
                if (c == '{')
                    brace++;
                if (c == '}')
                    brace--;
            }
        }

        if (ti)
        {
            tok[ti] = '\0';
            classify(tok);
            ti = 0;
        }
    }

    fclose(fp);

    printf("\n");

    if (paren != 0 && bracket != 0 && brace != 0)
    {
        if (paren)
            printf(RED "Unbalanced () = %d\n" RESET, paren);
        if (bracket)
            printf(RED "Unbalanced [] = %d\n" RESET, bracket);
        if (brace)
            printf(RED "Unbalanced {} = %d\n" RESET, brace);
    }
}
