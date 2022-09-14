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
    Regex* space = Regex_new("^[ \t]");

    Regex* symbols = Regex_new("^\\(|\\)|\\+|\\*|\\-|\\!|\\=|\\||\\?|\\:|\\;|\\{|\\}|(\\>\\=)|(\\<\\=)|(\\=\\=)");
    Regex* int_constants = Regex_new("^0|[1-9]+[0-9]*");
    Regex* identifiers = Regex_new("^[a-zA-Z][0-9a-zA-Z_]*");

    Regex* hex = Regex_new("^0x[a-fA-F0-9]+");
    Regex* string_literals = Regex_new("^\\\"[^\'\"]*\\\"");
    Regex* valid_keywords = Regex_new("^def|^if|^else|^while|^break|^continue|^int|^bool|^void|^true|^false([^a-zA-Z0-9])");
    Regex* invalid_keywords = Regex_new("^for|^callout|^class|^interface|^extends|^implements|^new|^string|^float|^double|^null");
    Regex* comments = Regex_new("//.*");

    /* read and handle input */
    char match[MAX_TOKEN_LEN];
    
    int placeholder = 1;
    while (*text != '\0') {

        // Thoughts for B test multiple
        // Loop with some boolean variable, 
        // once it gets to the end of the line, 
        // then increment placeholder that way 
        //multiple tokens can be on the same line
        
        /* match regular expressions */
        // printf("%s\n", text);
        if (Regex_match(whitespace, text, match)) {
            /* ignore whitespace */
            if (text[0] == '\n') {
                placeholder++;
            }
        } else if (Regex_match(space, text, match)) {

        } else if (Regex_match(valid_keywords, text, match)) {
            TokenQueue_add(tokens, Token_new(KEY, match, placeholder));

        } else if (Regex_match(invalid_keywords, text, match)) {
            Error_throw_printf("Invalid token!\n");

        } else if (Regex_match(identifiers, text, match)) {
            TokenQueue_add(tokens, Token_new(ID, match, placeholder));
            
        } else if (Regex_match(hex, text, match)) {
            TokenQueue_add(tokens, Token_new(HEXLIT, match, placeholder));
            
        } else if (Regex_match(string_literals, text, match)) {
            TokenQueue_add(tokens, Token_new(STRLIT, match, placeholder));

        } else if (Regex_match(symbols, text, match)) {
            TokenQueue_add(tokens, Token_new(SYM, match, placeholder));

        } else if (Regex_match(int_constants, text, match)) {
            TokenQueue_add(tokens, Token_new(DECLIT, match, placeholder));

        } else if (Regex_match(comments, text, match)) {

        } else {
            Error_throw_printf("Invalid token!\n");
        }

        /* skip matched text to look for next token */
        // printf("%s\n", match);
        text += strlen(match);
    }
 
    /* clean up */
    Regex_free(whitespace);
    Regex_free(symbols);
    Regex_free(int_constants);
    Regex_free(identifiers);
    Regex_free(hex);
    Regex_free(string_literals);
    Regex_free(valid_keywords);
    Regex_free(invalid_keywords);
    Regex_free(comments);
    return tokens;
}

