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

    Regex* symbols = Regex_new("^\\(|\\)|\\+|\\*|\\-|\\!|\\||\\?");
    Regex* int_constants = Regex_new("^0|[1-9]+[0-9]*");
    Regex* identifiers = Regex_new("^[a-zA-Z][0-9a-zA-Z_]*");

    Regex* hex = Regex_new("^0x[a-fA-F0-9]+");
    Regex* string_literals = Regex_new("^\\\".*\\\"");
 
    /* read and handle input */
    char match[MAX_TOKEN_LEN];
    while (*text != '\0') {
 
        /* match regular expressions */
        if (Regex_match(whitespace, text, match)) {
            /* ignore whitespace */
        } else if (Regex_match(hex, text, match)) {
            TokenQueue_add(tokens, Token_new(HEXLIT, match, -1));

        } else if (Regex_match(string_literals, text, match)) {
            TokenQueue_add(tokens, Token_new(STRLIT, match, -1));

        } else if (Regex_match(symbols, text, match)) {
            TokenQueue_add(tokens, Token_new(SYM, match, -1));

        } else if (Regex_match(int_constants, text, match)) {
            TokenQueue_add(tokens, Token_new(DECLIT, match, -1));

        } else if (Regex_match(identifiers, text, match)) {
            TokenQueue_add(tokens, Token_new(ID, match, -1));

        } else {
            Error_throw_printf("Invalid token!\n");
        }
 
        /* skip matched text to look for next token */
        text += strlen(match);
    }
 
    /* clean up */
    Regex_free(whitespace);
    Regex_free(symbols);
    Regex_free(int_constants);
    Regex_free(identifiers);
    Regex_free(hex);
    Regex_free(string_literals);
 
    return tokens;
}

