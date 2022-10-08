/**
* @file p2-parser.c
* @brief Compiler phase 2: parser
*
* Names: Joselyne Tran and Dakota Scott
*/
#include "p2-parser.h"
#include <string.h>
ASTNode* parse_expressions(TokenQueue* input);
ASTNode* parse_block(TokenQueue* input);
BinaryOpType find_binop(BinaryOpType binop[], int binop_amounts, TokenQueue* input);
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

void manip_nl_character(char* input, char* returnBuffer) {
    int len = strlen(input);
    char out_line[256];
    memset(out_line, 0, sizeof(out_line));
    char backslash = '\\';
    int i       = 0;
    int index   = 0;
    int n_slashes = 0;
    while(i <= len){
        if(input[i] == backslash) {
            n_slashes++;
            i++;
            char next_char = input[i];
            if(next_char == 'n') {
                out_line[index] = '\n';
            } else if(next_char == 't') {
                out_line[index] = '\t';
            } else {
                Error_throw_printf("Invalid escaped character %c", input[i]);
            }
        } else {
            out_line[index] = input[i];
        }
        index++;
        i++;
    }
    snprintf(returnBuffer, strlen(out_line) + n_slashes + 1, "%s", out_line);
}

/**
* @brief Parse and return a literal
*
* @param input Token queue to modify
*/
ASTNode* parse_literal(TokenQueue* input) {
    Token* token = TokenQueue_remove(input);
    int lineNum = get_next_token_line(input);
    char manipulated_string[MAX_ID_LEN];
    //
    if (token->type == DECLIT) {
        return LiteralNode_new_int(atoi(token->text), lineNum);
    } else if (token->type == HEXLIT) {
        return LiteralNode_new_int((int)strtol(token->text, NULL, 16), lineNum);
    } else if (token->type == STRLIT) {
        char *result = token->text+1; // removes first character
        result[strlen(result) -1 ] = '\0'; // removes last character
        manip_nl_character(result, manipulated_string);
        return LiteralNode_new_string(manipulated_string, lineNum);
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
    if (check_next_token_type(input, ID)) {
        if (strcmp(TokenQueue_peek(input)->next->text, "(") == 0) {
            node = parse_function_call(input);
        } else {
            node = parse_location(input);
        }
    // subexpression
    } else if (check_next_token(input, SYM, "(")) {
        match_and_discard_next_token(input, SYM, "(");
        node = parse_expressions(input);
        match_and_discard_next_token(input, SYM, ")");
    // literal
    } else {
        node = parse_literal(input);
    }
    return node;
}


/**
* @brief Parse and return a literal
*
* @param input Token queue to modify
*/
BinaryOpType find_binop(BinaryOpType binop[], int binop_amounts, TokenQueue* input) {
    
    Token* token = TokenQueue_peek(input);

    // string to check for binop
    Token* symbs[MAX_TOKEN_LEN];
    int k = 0;
    int size_symbs;
    int i;
    while (token != NULL && strcmp(token->text, ";") != 0 && strcmp(token->text, ")")
        && strcmp(token->text, "]") && strcmp(token->text, ",")) {
        if (token->type == SYM) {
            symbs[k] = token;
            k++;
        } 
        token = token->next;
    }

    for (size_symbs = k-1; size_symbs >= 0; size_symbs--) {
        for (i = 0; i < binop_amounts; i++) {
            if (strcmp(symbs[size_symbs]->text, BinaryOpToString(binop[i])) == 0 ) {
                return binop[i];
            }
        }
    }
    return INVALID;
}


/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_unary(TokenQueue* input, UnaryOpType ut, int source_line) {
    return UnaryOpNode_new(ut, parse_base_expressions(input), source_line);
}

/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_bin_math_primary(TokenQueue* input) {

    ASTNode* root = parse_base_expressions(input);
    int source_line = get_next_token_line(input);

    BinaryOpType arith_primary[3] = {MULOP, DIVOP, MODOP};
    BinaryOpType bo = find_binop(arith_primary, 4, input);

    while (bo != INVALID) 
    {
        match_and_discard_next_token(input, SYM, BinaryOpToString(bo));
        ASTNode* new_root = BinaryOpNode_new(bo, root, 
            parse_base_expressions(input), source_line);
        root = new_root;
        bo = find_binop(arith_primary, 3, input);
    }

    return root;
}

/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_bin_math_secondary(TokenQueue* input) {

    ASTNode* root = parse_bin_math_primary(input);
    int source_line = get_next_token_line(input);

    BinaryOpType arith_secondary[2] = {ADDOP, SUBOP};
    BinaryOpType bo = find_binop(arith_secondary, 2, input);

    while (bo != INVALID) 
    {
        match_and_discard_next_token(input, SYM, BinaryOpToString(bo));
        ASTNode* new_root = BinaryOpNode_new(bo, root, 
            parse_bin_math_secondary(input), source_line);
        root = new_root;
        bo = find_binop(arith_secondary, 2, input);
    }

    return root;
}

/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_bin_relations(TokenQueue* input) {

    ASTNode* root = parse_bin_math_secondary(input);
    int source_line = get_next_token_line(input);

    BinaryOpType relations[4] = {LTOP, LEOP, GEOP, GTOP};
    BinaryOpType bo = find_binop(relations, 4, input);

    while (bo != INVALID) 
    {
        match_and_discard_next_token(input, SYM, BinaryOpToString(bo));
        ASTNode* new_root = BinaryOpNode_new(bo, root, 
            parse_bin_math_secondary(input), source_line);
        root = new_root;
        bo = find_binop(relations, 4, input);
    }

    return root;
}

/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_bin_equality(TokenQueue* input) {

    ASTNode* root = parse_bin_relations(input);
    int source_line = get_next_token_line(input);

    BinaryOpType equality[2] = {EQOP, NEQOP};
    BinaryOpType bo = find_binop(equality, 2, input);

    while (bo != INVALID) 
    {
        match_and_discard_next_token(input, SYM, BinaryOpToString(bo));
        ASTNode* new_root = BinaryOpNode_new(bo, root, 
            parse_bin_relations(input), source_line);
        root = new_root;
        bo = find_binop(equality, 2, input);
    }

    return root;
}

/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_bin_AND(TokenQueue* input) {

    ASTNode* root = parse_bin_equality(input);
    int source_line = get_next_token_line(input);

    int k = 0;

    while (check_next_token(input, SYM, "&&")) 
    {
        match_and_discard_next_token(input, SYM, "&&");
        ASTNode* new_root = BinaryOpNode_new(ADDOP, root, 
            parse_bin_equality(input), source_line);
        root = new_root;
        k++;
    }

    return root;
}

/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_bin_OR(TokenQueue* input) {

    ASTNode* root = parse_bin_AND(input);
    int source_line = get_next_token_line(input);

    while (check_next_token(input, SYM, "||")) {
        match_and_discard_next_token(input, SYM, "||");
        ASTNode* new_root = BinaryOpNode_new(OROP, root, 
            parse_bin_AND(input), source_line);
        root = new_root;
    }
    return root;
}


/**
* @brief Parse and return a block node
*        ONLY IS HANDLING BASE EXPRESSIONS FOR NOW. *****
*
* @param input Token queue to modify
*/
ASTNode* parse_expressions(TokenQueue* input) {
    ASTNode* expression = NULL;
    int lineNum = get_next_token_line(input);
    BinaryOpType binops[13] = { OROP, ANDOP, EQOP, NEQOP, 
    LTOP, LEOP, GEOP, GTOP, ADDOP, SUBOP, MULOP, DIVOP,
     MODOP};

    // binary expression
    if (!check_next_token(input, SYM, "!") && 
        !check_next_token(input, SYM, "-") &&
        find_binop(binops, 13, input) != INVALID) {
        expression = parse_bin_OR(input);
    // unary expression = !
    } else if (check_next_token(input, SYM, "!")) {
        match_and_discard_next_token(input, SYM, "!");
        expression = parse_unary(input, NOTOP, lineNum);
    // unary expression: -
    } else if (check_next_token(input, SYM, "-")) {
        match_and_discard_next_token(input, SYM, "-");
        expression = parse_unary(input, NEGOP, lineNum);
    } else {
        expression = parse_base_expressions(input);
    }

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
   bool isarray = false;
   int array_length = 1;
 
   // finds variable type, identifier, and semicolon
   DecafType variable_type = parse_type(input);
   parse_id(input, identifier);
   if (check_next_token(input, SYM, "[")) {
       match_and_discard_next_token(input, SYM, "[");
       if (check_next_token_type(input, DECLIT)) {
           isarray = true;
           Token* arr_token = TokenQueue_remove(input);
           array_length = atoi(arr_token->text);
           free(arr_token);
       }
       match_and_discard_next_token(input, SYM, "]");
   }
   match_and_discard_next_token(input, SYM, ";");
   return VarDeclNode_new(identifier, variable_type, isarray, array_length, lineNum);
}
 
/**
* @brief Parse and return a Variable declaration node
*
* @param input Token queue to modify
* @c MAX_TOKEN_LEN characters long)
*/
ASTNode* parse_while(TokenQueue* input, int source_line)
{
   ASTNode* cond_expression = NULL;
   ASTNode* while_block = NULL;
 
   discard_next_token(input);
   match_and_discard_next_token(input, SYM, "(");
   cond_expression = parse_expressions(input);
   match_and_discard_next_token(input, SYM, ")");
   while_block = parse_block(input);
 
   return WhileLoopNode_new(cond_expression, while_block, source_line);
}
 
/**
* @brief Parse and return a Variable declaration node
*
* @param input Token queue to modify
* @c MAX_TOKEN_LEN characters long)
*/
ASTNode* parse_conditional (TokenQueue* input, int source_line)
{
   ASTNode* cond_expression = NULL;
   ASTNode* if_block = NULL;
   ASTNode* else_block = NULL;
 
   discard_next_token(input);
   match_and_discard_next_token(input, SYM, "(");
   cond_expression = parse_expressions(input);
   match_and_discard_next_token(input, SYM, ")");
   if_block = parse_block(input);
 
   // check for else statement
   if (check_next_token(input, KEY, "else")) {
       discard_next_token(input);
       else_block = parse_block(input);
   }
   return ConditionalNode_new(cond_expression, if_block, else_block, source_line);
}
 
ASTNode* parse_assignment (TokenQueue* input, int source_line) {
    ASTNode* loc = parse_location(input);
    match_and_discard_next_token (input, SYM, "=");
    ASTNode* lit = parse_expressions(input);
    match_and_discard_next_token (input, SYM, ";");

    return AssignmentNode_new(loc, lit, source_line);
}

/**
* @brief Parse and return a block node
*
* @param input Token queue to modify
*/
ASTNode* parse_block(TokenQueue* input) {
   NodeList* vars = NodeList_new();
   NodeList* stments = NodeList_new();
 
   int lineNum = get_next_token_line(input);
   match_and_discard_next_token (input, SYM, "{");
   Token* token = TokenQueue_peek(input);
 
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
       int source_line = get_next_token_line(input);
       // if next time is identifier, most likely start of assignment?
        if (check_next_token(input, KEY, "if")) {
           //  if ‘(’ Expr ‘)’ Block (else Block)?
           ASTNode* conditional = parse_conditional(input, source_line);
           NodeList_add(stments, conditional);
 
        } else if (check_next_token(input, KEY, "while")) {
           ASTNode* while_node = parse_while(input, source_line);
           NodeList_add(stments, while_node);
 
        } else if (strcmp(TokenQueue_peek(input)->next->text, "(") == 0) {
           ASTNode* func_call = parse_function_call(input);
           match_and_discard_next_token (input, SYM, ";");
           NodeList_add(stments, func_call);
 
       } else if (check_next_token_type(input, ID)) {
           ASTNode* assignment = parse_assignment(input, source_line);
 
           // check for same location type and literal type
           NodeList_add(stments, assignment);
 
       } else if (check_next_token(input, KEY, "return")) {
           ASTNode* expression = NULL;
           discard_next_token(input);
           if (!check_next_token(input, SYM, ";")) {
               expression = parse_expressions(input);
           }
           match_and_discard_next_token(input, SYM, ";");
           NodeList_add(stments, ReturnNode_new(expression, source_line));
 
       } else if (token_str_eq("continue", token->text) ) {
           discard_next_token(input);
           match_and_discard_next_token(input, SYM, ";");
           NodeList_add(stments, ContinueNode_new(source_line));
 
       } else if (token_str_eq("break", token->text) ) {
           discard_next_token(input);
           match_and_discard_next_token(input, SYM, ";");
           NodeList_add(stments, BreakNode_new(source_line));
 
       } else {
           Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
       }
   }
   match_and_discard_next_token (input, SYM, "}");
 
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
* @brief Parse and returns Operand ASTNode
*
* @param input Token queue to modify
* @param source_line Line the operand occurs on
*/
ASTNode* parse_operand(TokenQueue* input) {
   return parse_expressions(input);
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
  ASTNode* body = parse_block(input);
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


