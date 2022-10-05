/**
* @file p2-parser.c
* @brief Compiler phase 2: parser
*
* Names: Joselyne Tran and Dakota Scott
*/
 
#include "p2-parser.h"
ASTNode* parse_expressions(TokenQueue* input);
 
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
       t = INT;
   } else if (token_str_eq("bool", token->text)) {
       t = BOOL;
   } else if (token_str_eq("void", token->text)) {
       t = VOID;
   } else {
       Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
   }
 
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
 
/**
* @brief Parse and return a literal
*
* @param input Token queue to modify
*/
ASTNode* parse_literal(TokenQueue* input) {
   Token* token = TokenQueue_remove(input);
   int lineNum = get_next_token_line(input);
 
   //
   if (token->type == DECLIT) {
       return LiteralNode_new_int(atoi(token->text), lineNum);
 
   } else if (token->type == HEXLIT) {
       return LiteralNode_new_int((int)strtol(token->text, NULL, 16), lineNum);
  
   } else if (token->type == STRLIT) {
       char *result = token->text+1; // removes first character
       result[strlen(result)-1] = '\0'; // removes last character
       return LiteralNode_new_string(result, lineNum);
 
   } else if ( token_str_eq("true", token->text) ) {
       return LiteralNode_new_bool(true, lineNum);
 
   } else if (token_str_eq("false", token->text) ) {
       return LiteralNode_new_bool(false, lineNum);
 
   } else {
       Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
 
   }
 
   Token_free(token);
   return NULL;
}
 
/**
* @brief Parse and return a location node
*
* @param input Token queue to modify
* @c MAX_TOKEN_LEN characters long)
*/
ASTNode* parse_location(TokenQueue* input) {
   char identifier[MAX_TOKEN_LEN];
   int lineNum = get_next_token_line(input);
   ASTNode* index = NULL;
   parse_id(input, identifier);
 
   // checking for index
   if (check_next_token(input, SYM, "[")) {
       match_and_discard_next_token (input, SYM, "[");
 
       // literal is place holder because we don't have the other expressions set up
       index = parse_expressions(input);
       match_and_discard_next_token (input, SYM, "]");
   }
 
   return LocationNode_new (identifier, index, lineNum);
}
 
/**
* @brief Parse and return args list
*
* @param input Token queue to modify
*/
NodeList* parse_args(TokenQueue* input) {
   NodeList* args_list = NodeList_new();
   if (!check_next_token(input, SYM, ")")) {
       char param_id[MAX_TOKEN_LEN];
       ASTNode* arg = parse_expressions(input);
       NodeList_add(args_list, arg);
       while (check_next_token(input, SYM, ",")) {
           match_and_discard_next_token(input, SYM, ",");
           memset(param_id, 0, sizeof(param_id));
           NodeList_add(args_list, parse_expressions(input));
       }
   }
 
   return args_list;
  
}
 
/**
* @brief Parse and return function call node
*
* @param input Token queue to modify
*/
ASTNode* parse_function_call(TokenQueue* input) {
   char func_id[MAX_TOKEN_LEN];
   int lineNum = get_next_token_line(input);
   parse_id(input, func_id);
   match_and_discard_next_token(input, SYM, "(");
   NodeList* args = parse_args(input);
   match_and_discard_next_token(input, SYM, ")");
 
   return FuncCallNode_new(func_id, args, lineNum);
}
 
 
/**
* @brief Parse and return a block node
*        ONLY HANDLING LOCATIONS AND LITERALS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_base_expressions(TokenQueue* input) {
   ASTNode* node = NULL;
 
   // (function call)
   if (strcmp(TokenQueue_peek(input)->next->text, "(") == 0) {
       node = parse_function_call(input);
   // subexpression
   } else if (check_next_token(input, SYM, "(")) {
        match_and_discard_next_token(input, SYM, "(");
       node = parse_expressions(input);
       match_and_discard_next_token(input, SYM, ")");
   // location
   } else if (check_next_token_type(input, ID)) {
       node = parse_location(input);
   // literal
   } else {
       node = parse_literal(input);
   }
 
   return node;
}
 
/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_expressions(TokenQueue* input) {
   ASTNode* expression = parse_base_expressions(input);
   return expression;
}
 
/**
* @brief Parse and return a Variable declaration node
*
* @param input Token queue to modify
* @c MAX_TOKEN_LEN characters long)
*/
ASTNode* parse_vardecl (TokenQueue* input)
{
   // setting up id buffer and line number
   char identifier[MAX_TOKEN_LEN];
   int lineNum = get_next_token_line(input);
 
   // finds variable type, identifier, and semicolon
   DecafType variable_type = parse_type(input);
   parse_id(input, identifier);
   match_and_discard_next_token(input, SYM, ";");
 
   return VarDeclNode_new(identifier, variable_type, false, 1, lineNum);
}
 
/**
* @brief Parse and return a block node
*
* @param input Token queue to modify
*/
ASTNode* parse_block(TokenQueue* input) {
 
   NodeList* vars = NodeList_new();
   NodeList* stments = NodeList_new();
 
   Token* token = TokenQueue_peek(input);
   int lineNum = get_next_token_line(input);
 
   // make sure variable declarations go first'
   while (token_str_eq("bool", token->text) ||
           token_str_eq("int", token->text) ||
           token_str_eq("void", token->text)) {
       NodeList_add(vars, parse_vardecl(input));
       token = TokenQueue_peek(input);
   }
 
   // parsing for statements
   while (!check_next_token(input, SYM, "}")) {
       token = TokenQueue_peek(input);
       lineNum = get_next_token_line(input);
       // if next time is identifier, most likely start of assignment?
       if (check_next_token_type(input, ID)) {
           ASTNode* loc = parse_location(input);
           match_and_discard_next_token (input, SYM, "=");
           ASTNode* lit = parse_expressions(input);
           match_and_discard_next_token (input, SYM, ";");
 
           // check for same location type and literal type
           NodeList_add(stments, AssignmentNode_new(loc, lit, lineNum));
 
       } else if ( token_str_eq("return", token->text) ) {
           ASTNode* expression = NULL;
           discard_next_token(input);
           if (!check_next_token(input, SYM, ";")) {
               expression = parse_expressions(input);
           }
           match_and_discard_next_token(input, SYM, ";");
           NodeList_add(stments, ReturnNode_new(expression, lineNum));
 
       } else if (token_str_eq("continue", token->text) ) {
           discard_next_token(input);
           match_and_discard_next_token(input, SYM, ";");
           NodeList_add(stments, ContinueNode_new(lineNum));
 
       } else if (token_str_eq("break", token->text) ) {
           discard_next_token(input);
           match_and_discard_next_token(input, SYM, ";");
           NodeList_add(stments, BreakNode_new(lineNum));
 
       } else {
           Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
       }
   }
 
   return BlockNode_new(vars, stments, lineNum);
}
 
 
/**
* @brief Parse and returns parameters list
*
* @param input Token queue to modify
*/
ParameterList* parse_parameters_list(TokenQueue* input) {
   ParameterList* parameterlist = ParameterList_new();
   char param_id[MAX_TOKEN_LEN];
 
   DecafType param_type = parse_type(input);
   parse_id(input, param_id);
   ParameterList_add_new (parameterlist, param_id, param_type);
 
   while (check_next_token(input, SYM, ",")) {
       match_and_discard_next_token (input, SYM, ",");
       memset(param_id, 0, sizeof(param_id));
       param_type = parse_type(input);
       parse_id(input, param_id);
       ParameterList_add_new (parameterlist, param_id, param_type);
   }
 
   return parameterlist;
}
 
/**
* @brief Parse and returns function node
*
* @param input Token queue to modify
*/
ASTNode* parse_funcdecl (TokenQueue* input) {
   char func_name[MAX_TOKEN_LEN];
   int source_line = get_next_token_line(input);
   ParameterList* parameters = ParameterList_new();
 
   // discard def
   discard_next_token(input);
 
   // finds return type and function name
   DecafType return_type = parse_type(input);
   parse_id(input, func_name);
 
   // finds parameters if any
   match_and_discard_next_token (input, SYM, "(");
   if (!check_next_token(input, SYM, ")")) {
       parameters = parse_parameters_list(input);
   }
   match_and_discard_next_token (input, SYM, ")");
 
   // parse block node
   match_and_discard_next_token (input, SYM, "{");
   ASTNode* body = parse_block(input);
   match_and_discard_next_token (input, SYM, "}");
   return FuncDeclNode_new(func_name, return_type, parameters, body, source_line);
}
 
 
/**
* @brief Parsing and building tree off of program
*
* @param input Token queue to modify
*/
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
 
 