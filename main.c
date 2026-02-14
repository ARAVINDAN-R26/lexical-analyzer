#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN 100
#define MAX_KEYWORDS 32
 

int current_line = 1;

const char *keywords[] = {
    "int", "float", "char", "void", "if", "else", "while", "for",
    "return", "break", "continue", "switch", "case", "default",
    "struct", "union", "enum", "typedef", "static", "extern",
    "const", "volatile", "signed", "unsigned", "short", "long",
    "double", "include", "define", "pragma", "ifndef", "endif",
    "else if"};
 
typedef enum
{
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_LITERAL,
    TOKEN_NUMBER,
    TOKEN_CHARACTER,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_HEXADECIMAL,
    TOKEN_OCTAL,
    TOKEN_UNKNOWN,
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    char value[MAX_TOKEN];
} Token;

int isKeyword(const char *str)
{
    for (int i = 0; i < MAX_KEYWORDS; i++)
    {
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int check_line_syntax(char *line, int line_num)
{
    // Remove trailing whitespace
    char *end = line + strlen(line) - 1;
    while (end > line && isspace(*end)) end--;
    *(end + 1) = '\0';

    // Skip empty lines
    if (strlen(line) == 0) return 0;

    // Skip preprocessor directives
    if (line[0] == '#') return 0;

    // Skip comments (simple check)
    if (strstr(line, "//") || strstr(line, "/*")) return 0;

    // Check if the line ends with semicolon, brace, comma, parenthesis, or backslash
    char last = line[strlen(line) - 1];
    if (last == ';' || last == '{' || last == '}' || last == ',' || last == ')' || last == '\\') return 0;

    // If the line contains assignment or function call, it likely needs a semicolon
    if (strchr(line, '=') || strchr(line, '('))
    {
        printf("Syntax error: Missing semicolon at line %d\n", line_num);
        return 1;
    }

    return 0;
}

void skipComments(FILE *file, int c)
{
    if (c == '/')
    {
        int next = fgetc(file);
        if (next == '/')
        {
            while ((c = fgetc(file)) != EOF && c != '\n')
                ;
        }
        else if (next == '*')
        {
            int comment_start_line = current_line;
            while ((c = fgetc(file)) != EOF)
            {
                if (c == '\n')
                    current_line++;
                if (c == '*' && (next = fgetc(file)) == '/')
                    break;
                ungetc(next, file);
            }
            if (c == EOF)
            {
                printf("Error: Unclosed multi-line comment starting at line %d\n", comment_start_line);
                exit(1);
            }
        }
        else
        {
            ungetc(next, file);
        }
    }
}

Token getNextToken(FILE *file)
{
    int prev_line;
    Token token;
    int c;

    while ((c = fgetc(file)) != EOF)
    {
    
        if (c == '\n')
        {
            current_line++;
        }
        if (isspace(c))
            continue;
        if (c == '#')
        {
            while ((c = fgetc(file)) != EOF && c != '\n')
                ;
            continue;
        }
        if (c == '/')
        {
            skipComments(file, c);
            continue;
        }
        break;
    }

    if (c == EOF)
    {
        token.type = TOKEN_EOF;
        return token;
    }

    if (isalpha(c) || c == '_')
    {
        // identifier or keyword
        int i = 0;
        token.value[i++] = c;
        while ((c = fgetc(file)) != EOF && (isalnum(c) || c == '_'))
        {
            token.value[i++] = c;
        }
        if (c != EOF)
            ungetc(c, file);
        token.value[i] = '\0';
        if (isKeyword(token.value))
            token.type = TOKEN_KEYWORD;
        else
            token.type = TOKEN_IDENTIFIER;
    }
    else if (isdigit(c))
    {
        // number
        int i = 0;
        token.value[i++] = c;
        while ((c = fgetc(file)) != EOF && (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-' || (i==1 && c=='x') || (i==1 && c=='X')))
        {
            token.value[i++] = c;
        }
        if (c != EOF)
            ungetc(c, file);
        token.value[i] = '\0';
        if (token.value[0] == '0' && (token.value[1] == 'x' || token.value[1] == 'X'))
        {
            token.type = TOKEN_HEXADECIMAL;
        }
        else if (token.value[0] == '0' && isdigit(token.value[1]) && token.value[1] >= '0' && token.value[1] <= '7')
        {
            token.type = TOKEN_OCTAL;
        }
        else
        {
            token.type = TOKEN_NUMBER;
        }
    }
    else if (c == '.')
    {
        // check if followed by digit
        int next = fgetc(file);
        if (next != EOF && isdigit(next))
        {
            // number
            token.value[0] = '.';
            token.value[1] = next;
            int i = 2;
            while ((c = fgetc(file)) != EOF && (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-'))
            {
                token.value[i++] = c;
            }
            if (c != EOF)
                ungetc(c, file);
            token.value[i] = '\0';
            token.type = TOKEN_NUMBER;
        }
        else
        {
            // operator .
            if (next != EOF)
                ungetc(next, file);
            token.value[0] = '.';
            token.value[1] = '\0';
            token.type = TOKEN_OPERATOR;
        }
    }
    else if (c == '"')
    {
        // string literal
        int i = 0;
        token.value[i++] = c;
        while ((c = fgetc(file)) != EOF && c != '"')
        {
            token.value[i++] = c;
            if (c == '\\')
            {
                c = fgetc(file);
                if (c != EOF)
                    token.value[i++] = c;
            }
        }
        if (c == '"')
            token.value[i++] = c;
        token.value[i] = '\0';
        if (c != '"')
        {
            printf("Error: Unclosed string literal at line %d\n", current_line);
            exit(1);
        }
        token.type = TOKEN_LITERAL;
    }
    else if (c == '\'')
    {
        // character literal
        int i = 0;
        token.value[i++] = c;
        c = fgetc(file);
        if (c != EOF)
            token.value[i++] = c;
        if (c == '\\')
        {
            c = fgetc(file);
            if (c != EOF)
                token.value[i++] = c;
        }
        c = fgetc(file);
        if (c == '\'')
            token.value[i++] = c;
        token.value[i] = '\0';
        if (c != '\'')
        {
            printf("Error: Unclosed character literal at line %d\n", current_line);
            exit(1);
        }
        token.type = TOKEN_CHARACTER;
    }
    else if (strchr("=<>!&|^%+-*/(){}[]", c))
    {
        // operator
        int i = 0;
        token.value[i++] = c;
        int next = fgetc(file);
        if (next != EOF)
        {
            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
                (c == '<' && (next == '=' || next == '<')) ||
                (c == '>' && (next == '=' || next == '>')) ||
                (c == '&' && next == '&') ||
                (c == '|' && next == '|') ||
                (c == '+' && next == '+') ||
                (c == '-' && next == '-') ||
                (c == '*' && next == '=') ||
                (c == '/' && next == '=') ||
                (c == '%' && next == '=') ||
                (c == '^' && next == '=') ||
                (c == '|' && next == '=') ||
                (c == '&' && next == '='))
            {
                token.value[i++] = next;
            }
            else
            {
                ungetc(next, file);
            }
        }
        token.value[i] = '\0';
        token.type = TOKEN_OPERATOR;
    }
    else if (strchr(";,", c))
    {
        token.value[0] = c;
        token.value[1] = '\0';
        token.type = TOKEN_DELIMITER;
    }
    else
    {
        token.value[0] = c;
        token.value[1] = '\0';
        token.type = TOKEN_UNKNOWN;
    }


    return token;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        printf("Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    // Check for syntax errors line by line
    char line[1024];
    int line_num = 1;
    int has_error = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (check_line_syntax(line, line_num))
            has_error = 1;
        line_num++;
    }
    if (has_error)
    {
        fclose(file);
        return 1;
    }

    // Rewind file for tokenization
    rewind(file);
    current_line = 1; // Reset line counter

    Token token;
    const char *typeStr[] = {"IDENTIFIER", "KEYWORD", "LITERAL", "NUMBER", "CHARACTER", "OPERATOR",
                             "DELIMITER", "HEXADECIMAL", "OCTAL", "UNKNOWN", "EOF"};

    int brace_count = 0;
    int paren_count = 0;
    int bracket_count = 0;

    while ((token = getNextToken(file)).type != TOKEN_EOF)
    {
        if (token.type == TOKEN_UNKNOWN)
        {
            printf("Lexical error: Unknown token '%s' at line %d\n", token.value, current_line);
            fclose(file);
            return 1;
        }

        // Check for delimiters
        if (token.type == TOKEN_DELIMITER || token.type == TOKEN_OPERATOR)
        {
            if (strcmp(token.value, "{") == 0)
                brace_count++;
            else if (strcmp(token.value, "}") == 0)
                brace_count--;
            else if (strcmp(token.value, "(") == 0)
                paren_count++;
            else if (strcmp(token.value, ")") == 0)
                paren_count--;
            else if (strcmp(token.value, "[") == 0)
                bracket_count++;
            else if (strcmp(token.value, "]") == 0)
                bracket_count--;

            if (brace_count < 0 || paren_count < 0 || bracket_count < 0)
            {
                printf("Syntax error: Unmatched closing delimiter '%s' at line %d\n", token.value, current_line);
                fclose(file);
                return 1;
            }
        }

        printf("%-15s : %s\n", typeStr[token.type], token.value);
    }

    if (brace_count != 0)
    {
        printf("Syntax error: Unmatched braces. Missing %d closing brace(s)\n", brace_count);
        fclose(file);
        return 1;
    }
    if (paren_count != 0)
    {
        printf("Syntax error: Unmatched parentheses. Missing %d closing parenthesis(es)\n", paren_count);
        fclose(file);
        return 1;
    }
    if (bracket_count != 0)
    {
        printf("Syntax error: Unmatched brackets. Missing %d closing bracket(s)\n", bracket_count);
        fclose(file);
        return 1;
    }

    fclose(file);

    return 0;
}