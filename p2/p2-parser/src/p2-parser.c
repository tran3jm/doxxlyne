/**
 * @file p2-parser.c
 * @brief Compiler phase 2: parser
 * 
 * Names: Joselyne Tran and Dakota Scott
 */

#include "p2-parser.h"

/*
 * helper functions
 */

/**
 * @brief Look up the source line of the next token in the queue.
 * 
 * @param input Token queue to examine
 * @returns Source line
 */
int get_next_token_line (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input\n");
    }
    return TokenQueue_peek(input)->line;
}

/**
 * @brief Check next token for a particular type and text and discard it
 * 
 * Throws an error if there are no more tokens or if the next token in the
 * queue does not match the given type or text.
 * 
 * @param input Token queue to modify
 * @param type Expected type of next token
 * @param text Expected text of next token
 */
void match_and_discard_next_token (TokenQueue* input, TokenType type, const char* text)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected \'%s\')\n", text);
    }
    Token* token = TokenQueue_remove(input);
    if (token->type != type || !token_str_eq(token->text, text)) {
        Error_throw_printf("Expected \'%s\' but found '%s' on line %d\n",
                text, token->text, get_next_token_line(input));
    }
    Token_free(token);
}

/**
 * @brief Remove next token from the queue
 * 
 * Throws an error if there are no more tokens.
 * 
 * @param input Token queue to modify
 */
void discard_next_token (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input\n");
    }
    Token_free(TokenQueue_remove(input));
}

/**
 * @brief Look ahead at the type of the next token
 * 
 * @param input Token queue to examine
 * @param type Expected type of next token
 * @returns True if the next token is of the expected type, false if not
 */
bool check_next_token_type (TokenQueue* input, TokenType type)
{
    if (TokenQueue_is_empty(input)) {
        return false;
    }
    Token* token = TokenQueue_peek(input);
    return (token->type == type);
}

/**
 * @brief Look ahead at the type and text of the next token
 * 
 * @param input Token queue to examine
 * @param type Expected type of next token
 * @param text Expected text of next token
 * @returns True if the next token is of the expected type and text, false if not
 */
bool check_next_token (TokenQueue* input, TokenType type, const char* text)
{
    if (TokenQueue_is_empty(input)) {
        return false;
    }
    Token* token = TokenQueue_peek(input);
    return (token->type == type) && (token_str_eq(token->text, text));
}

/**
 * @brief Parse and return a Decaf type
 * 
 * @param input Token queue to modify
 * @returns Parsed type (it is also removed from the queue)
 */
DecafType parse_type (TokenQueue* input)
{
    Token* token = TokenQueue_remove(input);
    if (token->type != KEY) {
        Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
    }

    DecafType t = VOID;
    if (token_str_eq("int", token->text)) {
        //printf("%s", token->text);

        t = INT;
    } else if (token_str_eq("bool", token->text)) {
        //printf("%s", token->text);

        t = BOOL;
    } else if (token_str_eq("void", token->text)) {
        //printf("%s", token->text);

        t = VOID;
    } else {

        Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    //printf("%s, %d", token->text, token->type);

    Token_free(token);
    return t;
}
//Used for debugging, didn't feel like typing printf every time
void print(char* str) {
    printf("%s", str);
}
/**
 * @brief Parse and return a Decaf identifier
 * 
 * @param input Token queue to modify
 * @param buffer String buffer for parsed identifier (should be at least
 * @c MAX_TOKEN_LEN characters long)
 */
void parse_id (TokenQueue* input, char* buffer)
{
    Token* token = TokenQueue_remove(input);
    if (token->type != ID) {
        Error_throw_printf("Invalid ID '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    snprintf(buffer, MAX_ID_LEN, "%s", token->text);
    Token_free(token);
}

ASTNode* parse_vardecl (TokenQueue* input) 
{
    char identifier[MAX_TOKEN_LEN];
    int lineNum = get_next_token_line(input);
    DecafType variable_type = parse_type(input);
    parse_id(input, identifier);
    match_and_discard_next_token(input, SYM, ";");
    return VarDeclNode_new(identifier, variable_type, false, 1, lineNum);
}

ASTNode* parse_funcdecl (TokenQueue* input) {
    int source_line = get_next_token_line(input);
    //printf("%s", input->head->text);
    //printf("%s", TokenQueue_remove(input)->type);
    //printf("%s", input->head->text);
    char* def_word = TokenQueue_remove(input)->text;
    // printf("%s", def_word);
    //Something needs to be done in order to not send "def" to parse_type
    DecafType return_type = parse_type(input);
    printf("%d", return_type);
    char func_name[MAX_TOKEN_LEN]; 
    parse_id(input, func_name);
    // printf("%s", func_name);

    ParameterList* parameters = ParameterList_new();
    ASTNode* body = ASTNode_new(BLOCK, source_line);
    // printf("%s", func_name);

    return FuncDeclNode_new(func_name, return_type, parameters, body, source_line);
}

ASTNode* parse_program (TokenQueue* input)
{
    NodeList* vars = NodeList_new();
    NodeList* funcs = NodeList_new();
    //char token_buffer[MAX_TOKEN_LEN];
    //Token* curr_tok;
    while (!TokenQueue_is_empty(input))
    {
        if (check_next_token(input, KEY, "def"))
        {
            NodeList_add(funcs, parse_funcdecl(input));
        }
        else
        {
            NodeList_add(vars, parse_vardecl(input));
        }
    }
    return ProgramNode_new(vars, funcs);
}

ASTNode* parse (TokenQueue* input)
{
    if (input == NULL) {
        Error_throw_printf("Null Input");
    }
    return parse_program(input);
}
