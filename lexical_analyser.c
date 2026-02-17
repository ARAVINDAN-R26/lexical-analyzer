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
#define CYAN "\033[36m"


unsigned int curly = 0, square = 0, flower = 0;

void classify(char *tok)
{
    char *keywords[] = {
        "auto", "break", "case", "char", "const", "continue",
        "default", "do", "double", "else", "enum", "extern",
        "float", "for", "goto", "if", "int", "long",
        "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void",
        "volatile", "while"};

    // 1. Check keyword
    for (int i = 0; i < 32; i++)
    {
        if (strcmp(tok, keywords[i]) == 0)
        {
            printf(GREEN "KEYWORD -> %s\n" RESET, tok);
            return;
        }
    }

    // 2. Check number (integer constant)
    if(isdigit(tok[0]))
    {
       for (int i = 0; tok[i]; i++)
        {
            if (!isdigit(tok[i]))
            {
                printf("Invalid value %s\n", tok);
                return;
               // exit(0);
            }
        }
        printf(MAGENTA "NUMBER -> %s\n" RESET, tok);
        return;
    }

    //3. check identifier
    if (isalpha(tok[0]) || tok[0] == '_')
    {
        int valid = 1;
        for (int i = 1; tok[i]; i++)
        {
            if (!isalnum(tok[i]) && tok[i] != '_')
            {
                valid = 0;
                break;
            }
        }
        if (valid)
        {
            printf(BLUE "IDENTIFIER -> %s\n" RESET, tok);
            return;
        }
        else
        {
            printf("Invalid Identifier %s\n", tok);
           // exit(0);
           return;
        }

    }
    
}

char operators[] = {'(', ')', '+', '-', '*', '%', '/', '[', ']', '.', '=', '{', '}', '&', '|', '^', '!', '~', '<', '>'};

int isoperator(char op)
{
    int flag = 0;
    for (int i = 0; i < sizeof(operators); i++)
    {
        if(operators[i] == op)
        {
            if(op == '(')
            {
                curly++;
            }
            else if(op == ')')
            {
                curly--;
            }
            else if(op == '[')
            {
                square++;
            }
            else if(op == ']')
            {
                square--;
            }

            flag = 1;
            break;
        }
    }
    if(flag)
        return 1;
    else
        return 0;
}

void segregate(char *line)
{
    char tok[100];
    int i = 0;

    for (int j = 0; line[j] != '\0'; j++)
    {
        // Identifier / keyword / number
        if (isalnum(line[j]) || line[j] == '_')
        {
            tok[i++] = line[j];
        }
        else 
        {
            if (i > 0)
            {
                tok[i] = '\0';
                i = 0;
                classify(tok);
            }
        }

        
        if(line[j] == ';')
        {
            printf(MAGENTA"SYMBOL -> %c\n"RESET, line[j]);
        }

        if(isoperator(line[j]))
        {
            tok[i++] = line[j++];
            if (line[j] == '+' || line[j] == '-' || line[j] == '=' || line[j] == '&' || line[j] == '|')
            {
                tok[i++] = line[j];
            }
            else
            {
                j--;
            }
            tok[i++] = '\0';
            i = 0;
            printf(YELLOW "OPERATOR -> %s\n" RESET, tok);
        }

        // String literal
        if (line[j] == '"')
        {
            tok[i++] = line[j++];
            while (line[j] != '"' && line[j] != '\0')
            {
                tok[i++] = line[j++];
            }
            tok[i++] = '"';
            tok[i] = '\0';
            printf(GREEN "STRING LITERAL -> %s\n" RESET, tok);
            i = 0;
        }


        if(line[j] == '\'')
        {
            tok[i++] = line[j++];
            
            if(line[j+1] == '\'')
            {
                tok[i++] = line[j++];
                tok[i++] = line[j++];
                if(line[j] == '\'')
                    tok[i++] = line[j];
                else
                    j--;
                tok[i] = '\0';
                printf(BLUE"CHARACTER -> %s\n"RESET, tok);
            }
            else if((line[j+1] == '0' && line[j] == '\\') || line[j+1] == '\\')
            {
                tok[i++] = line[j++];
                tok[i++] = line[j++];
                tok[i++] = line[j];
                tok[i] = '\0';
                printf(BLUE "CHARACTER -> %s\n" RESET, tok);
            }
            else
            {
                printf(RED"ERROR: Unterminated single quotes\n"RESET);
            }
        }
    }
    
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf(RED "ERROR: No arguments passed.\nUsage: %s <filename>\n" RESET, argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        printf(RED"Error opening file\n"RESET);
        return 1;
    }

    char line[1024];
    char c;
    int i;

    while (fgets(line, sizeof(line), file))
    {
        i = 0;
        while(line[i] != '\0')
        {
            if(line[i] == '/')
            {
                if(line[i+1] == '/')
                {
                    //single line comment
                    break; //ignore rest of the line
                }
                else if(line[i+1] == '*')
                {
                    //multi line comment
                    int flag = 1;
                    while((c = fgetc(file)) != EOF)
                    {
                        if(c == '*' && fgetc(file) == '/')
                        {
                            flag = 0;
                            break; //end of multi line comment
                        }
                    }
                    if(flag)
                    {
                        printf(RED "Error: Unterminated comment\n" RESET);
                        return 1;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    printf(RED "ERROR: Invalid comment\n" RESET);
                    exit(1);
                }
            }
            else if(line[i] == '#')
            {
                //preprocessor directive
                break; //ignore rest of the line
            }
            else if(line[i] == '{')
            {
                printf(YELLOW "OPERATOR -> { \n" RESET);
                flower++;                
            }
            else if(line[i] == '}')
            {
                printf(YELLOW "OPERATOR -> } \n" RESET);
                flower--;                
            }
            else if(isalnum(line[i]) )
            {
             
                segregate(line);
                break;
            }
            i++;
        }
        if (square)
        {
            printf(RED "ERROR: unterminated ']' brackets\n" RESET);
            exit(1);
        }
        if (curly)
        {
            printf(RED "ERROR: unterminated ')' brackets\n" RESET);
            exit(0);
        }
    }
  
    if(flower)
    {
        printf(RED"ERROR: unterminated %d '}' brackets\n"RESET, flower);
    }
}