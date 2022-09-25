/**
 * Names: Ian Lips, Matthew Dim
 * @file p1-lexer.c
 * @brief Compiler phase 1: lexer
 */
#include "p1-lexer.h"

TokenQueue* lex (char* text)
{
    TokenQueue* tokens = TokenQueue_new();

    /* compile regular expressions */
    Regex* whitespace = Regex_new("^[ \n\t]");
    Regex* symbol = Regex_new("^(\\(|\\{|\\[|\\]|\\}|\\)|\\,|\\;|=|\\+|-|\\*|/|%|<|>|<=|>=|==|!=|&&|(\\|\\|)|!)");
    Regex* decimalIntConst = Regex_new("^(0|[1-9][0-9]*)");
    Regex* hexidecimalConst = Regex_new("^0x[a-f0-9]+");
    Regex* identifier = Regex_new("^[A-Za-z][A-Za-z_0-9]*");
    Regex* stringLit = Regex_new("^\"[^\"]*\"");
    Regex* comment = Regex_new("^//[ ]*[^\n]*");
    Regex* keyword = Regex_new("^((\\bdef\\b)|(\\bif\\b)|(\\belse\\b)|(\\bwhile\\b)|(\\breturn\\b)|(\\bbreak\\b)|(\\bcontinue\\b)|(\\bint\\b)|(\\bbool\\b)|(\\bvoid\\b)|(\\btrue\\b)|(\\bfalse\\b))");
    Regex* unimplementedKeyword = Regex_new("^((\\bfor\\b)|(\\bcallout\\b)|(\\bclass\\b)|(\\binterface\\b)|(\\bextends\\b)|(\\bimplements\\b)|(\\bnew\\b)|(\\bthis\\b)|(\\bstring\\b)|(\\bfloat\\b)|(\\bdouble\\b)|(\\bnull\\b))");

    /* read and handle input */
    char match[MAX_TOKEN_LEN];
    int line_number = 1;

    if (text == NULL)
    {
        Regex_free(whitespace);
        Regex_free(symbol);
        Regex_free(decimalIntConst);
        Regex_free(hexidecimalConst);
        Regex_free(identifier);
        Regex_free(stringLit);
        Regex_free(comment);
        Regex_free(keyword);
        Regex_free(unimplementedKeyword);

        Error_throw_printf("Invalid token! 1\n");

        return tokens;
    }

    while (*text != '\0') {

        /* match regular expressions */
        if (Regex_match(whitespace, text, match)) {
            /* ignore whitespace */
            if (strstr(match, "\n") != NULL) {
                line_number = line_number + 1;
            }
        } else if(Regex_match(comment, text, match)) {
            line_number = line_number + 1;
            text += 1;
        } else if(Regex_match(identifier, text, match)) {
            if (Regex_match(keyword, text, match))
            {
                TokenQueue_add(tokens, Token_new(KEY, match, line_number));
            }
            else if (Regex_match(unimplementedKeyword, text, match))
            {
                Regex_free(whitespace);
                Regex_free(symbol);
                Regex_free(decimalIntConst);
                Regex_free(hexidecimalConst);
                Regex_free(identifier);
                Regex_free(stringLit);
                Regex_free(comment);
                Regex_free(keyword);
                Regex_free(unimplementedKeyword);

                Error_throw_printf("Invalid token!\n");
            }
            else
            {
                TokenQueue_add(tokens, Token_new(ID, match, line_number));
            }
        } else if(Regex_match(symbol, text, match)) {
            TokenQueue_add(tokens, Token_new(SYM, match, line_number));
        } else if (Regex_match(hexidecimalConst, text, match)) {
            TokenQueue_add(tokens, Token_new(HEXLIT, match, line_number));
        } else if(Regex_match(decimalIntConst, text, match)) {
            TokenQueue_add(tokens, Token_new(DECLIT, match, line_number));
        } else if(Regex_match(stringLit, text, match)) {
            TokenQueue_add(tokens, Token_new(STRLIT, match, line_number));
        } else {
            Regex_free(whitespace);
            Regex_free(symbol);
            Regex_free(decimalIntConst);
            Regex_free(hexidecimalConst);
            Regex_free(identifier);
            Regex_free(stringLit);
            Regex_free(comment);
            Regex_free(keyword);
            Regex_free(unimplementedKeyword);
            
            Error_throw_printf("Invalid token!\n");
        }

        /* skip matched text to look for next token */
        text += strlen(match);
    }

    /* clean up */
    Regex_free(whitespace);
    Regex_free(symbol);
    Regex_free(decimalIntConst);
    Regex_free(hexidecimalConst);
    Regex_free(identifier);
    Regex_free(stringLit);
    Regex_free(comment);
    Regex_free(keyword);
    Regex_free(unimplementedKeyword);

    return tokens;
}
