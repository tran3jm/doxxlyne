/**
 * @file p1-lexer.c
 * @brief Compiler phase 1: lexer
 * 
 * Dakota and Joselyne
 */
#include "p1-lexer.h"

TokenQueue* lex (char* text)
{
    TokenQueue* tokens = TokenQueue_new();
 
    /* compile regular expressions */
    Regex* whitespace = Regex_new("^[ \n]");
    Regex* letter = Regex_new("^[a-zA-Z]+");

    Regex* symbols = Regex_new("^\\(|\\)|\\+|\\*");
    Regex* int_constants = Regex_new("^0|[1-9]+[0-9]*");
    Regex* identifiers = Regex_new("^[a-zA-Z][0-9a-zA-Z_]*");
 
    /* read and handle input */
    char match[MAX_TOKEN_LEN];
    while (*text != '\0') {
 
        /* match regular expressions */
        if (Regex_match(whitespace, text, match)) {
            /* ignore whitespace */
        } else if (Regex_match(letter, text, match)) {
            /* TODO: implement line count and replace placeholder (-1) */
            TokenQueue_add(tokens, Token_new(ID, match, -1));
        } else if (Regex_match(symbols, text, match)) {
            /* TODO: implement line count and replace placeholder (-1) */
            TokenQueue_add(tokens, Token_new(SYM, match, -1));
        } else if (Regex_match(int_constants, text, match)) {
            /* TODO: implement line count and replace placeholder (-1) */
            TokenQueue_add(tokens, Token_new(DECLIT, match, -1));
        } else if (Regex_match(identifiers, text, match)) {
            /* TODO: implement line count and replace placeholder (-1) */
            TokenQueue_add(tokens, Token_new(ID, match, -1));
        } else {
            Error_throw_printf("Invalid token!\n");
        }
 
        /* skip matched text to look for next token */
        text += strlen(match);
    }
 
    /* clean up */
    Regex_free(whitespace);
    Regex_free(letter);
 
    return tokens;
}

