/**************************************************************************
 * C S 429 EEL interpreter
 *
 * eval.c - This file contains the skeleton of functions to be implemented by
 * you. When completed, it will contain the code used to evaluate an expression
 * based on its AST.
 *
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/

#include "ci.h"

/* Function interfaces */

/* Returns true if the given token is a binary operator and false otherwise */
extern bool is_binop(token_t);
/* Returns true if the given token is a unary operator and false otherwise */
extern bool is_unop(token_t);
/* It might be helpful to note that TOK_QUESTION is the only ternary operator.
 */

char *strrev(char *str);
bool is_type(node_t *nptr, type_t TYPE);
void concat(node_t *nptr);
char *repeat(node_t *nptr, node_t *rep_node);

/* infer_type() - set the type of a non-root node based on the types of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 * (STUDENT TODO)
 */

static void infer_type(node_t *nptr) {
  // check running status - you can ignore this
  if (terminate || ignore_input)
    return;

  // Week 1 TODO: Implement a recursive post-order traversal of the AST.
  // Remember to include a base case.
  if (nptr == NULL)
    return;

  for (int i = 0; i < 3 && nptr->children[i] != NULL; i++) {
    infer_type(nptr->children[i]);
  }

  switch (nptr->node_type) {
  // For each week, you will also need to include error checking for each type.
  // Week 1 TODO: Implement type inference for all operators on int and bool
  // types. Week 2 TODO: Extend type inference to handle operators on string
  // types. Week 3 TODO: Implement tpye evaluation for variables.
  case NT_INTERNAL:
    switch (nptr->tok) {

    // For reference, the identity (do nothing) operator is implemented for you.
    case TOK_IDENTITY:
      nptr->type = nptr->children[0]->type;
      break;
    case TOK_PLUS:
      // Inside the infer_type() function
      if (nptr->children[0]->type == STRING_TYPE) {
        if (nptr->children[1]->type != STRING_TYPE) {
          handle_error(ERR_TYPE);
          return;
        } else {
          nptr->type = STRING_TYPE;
          break;
        }
      }
    case TOK_TIMES:
      if (nptr->children[0]->type == STRING_TYPE) {
        if (nptr->children[1]->type == STRING_TYPE) {
          handle_error(ERR_TYPE);
          return;
        } else {
          nptr->type = STRING_TYPE;
          break;
        }
      }
    case TOK_BMINUS:
    case TOK_DIV:
    case TOK_MOD:
      if (is_type(nptr, INT_TYPE)) {
        nptr->type = INT_TYPE;
        break;
      }
        handle_error(ERR_TYPE);
        return;
    case TOK_AND:
    case TOK_OR:
      if (is_type(nptr, BOOL_TYPE)) {
        nptr->type = BOOL_TYPE;
        break;
      }
      handle_error(ERR_TYPE);
      return;
    case TOK_LT:
    case TOK_GT:
    case TOK_EQ:
      if (is_type(nptr, INT_TYPE) || is_type(nptr, STRING_TYPE)) {
        nptr->type = BOOL_TYPE;
        break;
      }
        handle_error(ERR_TYPE);
        return;
    case TOK_UMINUS:
      if (is_type(nptr, STRING_TYPE)) {
        nptr->type = STRING_TYPE;
        break;
      } else if (is_type(nptr, INT_TYPE)) {
        nptr->type = INT_TYPE;
        break;
      }
        handle_error(ERR_TYPE);
        return;
    case TOK_NOT:
      if (is_type(nptr, BOOL_TYPE)) {
        nptr->type = BOOL_TYPE;
        break;
      }
        handle_error(ERR_TYPE);
        return;
    case TOK_QUESTION:
      if (nptr->children[0]->type != BOOL_TYPE ||
          nptr->children[1]->type != nptr->children[2]->type) {
          handle_error(ERR_TYPE);
          return;
      }
      nptr->type = nptr->children[1]->type;
      break;
    default:
      break;
    }
  case NT_LEAF:
  switch (nptr->tok){
case TOK_ID:;
    entry_t *e = get(nptr->val.sval);
    free(nptr->val.sval);
    nptr->type = e->type;
    if (e != NULL) {
        if(nptr->type == STRING_TYPE) {
            nptr->val.sval = malloc(strlen(e->val.sval) + 1);
            strcpy(nptr->val.sval, e->val.sval);
        } else {
            nptr->val = e->val;
        }   
    } else {
        handle_error(ERR_TYPE);
        return;
    }
    break;
  default:
    break;
  }
    break;
  default:
    break;
  }
  return;
}

bool is_type(node_t *nptr, type_t TYPE) {
  for (int i = 0; i < 3 && nptr->children[i] != NULL; i++) {
    if (nptr->children[i]->type != TYPE) {
      return false;
    }
  }
  return true;
}

/* infer_root() - set the type of the root node based on the types of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 */

static void infer_root(node_t *nptr) {
  if (nptr == NULL)
    return;
  // check running status
  if (terminate || ignore_input)
    return;

  // check for assignment
  if (nptr->type == ID_TYPE) {
    infer_type(nptr->children[1]);
  } else {
    for (int i = 0; i < 3; ++i) {
      infer_type(nptr->children[i]);
    }
    if (nptr->children[0] == NULL) {
      logging(LOG_ERROR, "failed to find child node");
      return;
    }
    nptr->type = nptr->children[0]->type;
  }
  return;
}

/* eval_node() - set the value of a non-root node based on the values of
 * children Parameter: A node pointer, possibly NULL. Return value: None. Side
 * effect: The val field of the node is updated. (STUDENT TODO)
 */

static void eval_node(node_t *nptr) {
  // check running status - you can ignore this.
  if (terminate || ignore_input)
    return;

  // Week 1 TODO: Implement a recursive post-order traversal of the AST.
  // Remember to include a base case.
  if (nptr == NULL)
    return;

  int value;
  if (nptr->tok != TOK_QUESTION) {
    for (int i = 0; i < 3 && nptr->children[i] != NULL; i++) {
      eval_node(nptr->children[i]);
    }
  }

  switch (nptr->node_type) {

  case NT_INTERNAL:
    // Week 1 TODO: Implement evaluation for all operators on int and bool
    // types. Week 2 TODO: Extend evaluation to handle operators on string
    // types.
    if (is_unop(nptr->tok)) {
      switch (nptr->tok) {
      case TOK_UMINUS:
        if (nptr->type == STRING_TYPE) {
          nptr->val.sval = strrev(nptr->children[0]->val.sval);
        } else {
          nptr->val.ival = -(nptr->children[0]->val.ival);
        }
        break;
      case TOK_NOT:
        nptr->val.bval = !(nptr->children[0]->val.bval);
        break;
      default:
        break;
      }
    }
    if (is_binop(nptr->tok)) {
      switch (nptr->tok) {
      case TOK_PLUS:
        if (nptr->type == STRING_TYPE) {
          concat(nptr);
        } else {
          nptr->val.ival =
              nptr->children[0]->val.ival + nptr->children[1]->val.ival;
        }
        break;
      case TOK_BMINUS:
        nptr->val.ival =
            nptr->children[0]->val.ival - nptr->children[1]->val.ival;
        break;
      case TOK_TIMES:
if (nptr->type == STRING_TYPE) {
    if (nptr->children[1]->val.ival < 0) {
        handle_error(ERR_EVAL);
        return;
    }
    nptr->val.sval = repeat(nptr->children[0], nptr->children[1]);
} else {
          nptr->val.ival =
              nptr->children[0]->val.ival * nptr->children[1]->val.ival;
        }
        break;
      case TOK_DIV:
        value = nptr->children[1]->val.ival;
        if (value == 0) {
          handle_error(ERR_EVAL);
          return;
        }
        nptr->val.ival = nptr->children[0]->val.ival / value;
        break;
      case TOK_MOD:
        value = nptr->children[1]->val.ival;
        if (value == 0) {
          handle_error(ERR_EVAL);
          return;
        }
        nptr->val.ival = nptr->children[0]->val.ival % value;
        break;
      case TOK_AND:
        nptr->val.bval =
            nptr->children[0]->val.bval && nptr->children[1]->val.bval;
        break;
      case TOK_OR:
        nptr->val.bval =
            nptr->children[0]->val.bval || nptr->children[1]->val.bval;
        break;
      case TOK_LT:
        if (nptr->children[0]->type == STRING_TYPE) {
          nptr->val.bval = (strcmp(nptr->children[0]->val.sval,
                                   nptr->children[1]->val.sval) < 0);
        } else {
          nptr->val.bval =
              nptr->children[0]->val.ival < nptr->children[1]->val.ival;
        }
        break;
      case TOK_GT:
        if (nptr->children[0]->type == STRING_TYPE) {
          nptr->val.bval = (strcmp(nptr->children[0]->val.sval,
                                   nptr->children[1]->val.sval) > 0);
        } else {
          nptr->val.bval =
              nptr->children[0]->val.ival > nptr->children[1]->val.ival;
        }
        break;
      case TOK_EQ:
        if (nptr->children[0]->type == STRING_TYPE) {
          nptr->val.bval = (strcmp(nptr->children[0]->val.sval,
                                   nptr->children[1]->val.sval) == 0);
        } else {
          nptr->val.bval =
              nptr->children[0]->val.ival == nptr->children[1]->val.ival;
        }
        break;
      default:
        break;
      }
    }
    if (nptr->tok == TOK_QUESTION) {
      eval_node(nptr->children[0]);
      if (nptr->children[0]->val.bval) {
        eval_node(nptr->children[1]);
        if (nptr->type == STRING_TYPE) {
          nptr->val.sval = malloc(strlen(nptr->children[1]->val.sval) + 1);
          strcpy(nptr->val.sval, nptr->children[1]->val.sval);
        } else {
          nptr->val.ival = nptr->children[1]->val.ival;
        }
      } else {
        eval_node(nptr->children[2]);
        if (nptr->type == STRING_TYPE) {
          nptr->val.sval = malloc(strlen(nptr->children[2]->val.sval) + 1);
          strcpy(nptr->val.sval, nptr->children[2]->val.sval);
        } else {
          nptr->val.ival = nptr->children[2]->val.ival;
        }
      }
      return;
    }
    // For reference, the identity (do-nothing) operator has been implemented
    // for you.
    if (nptr->tok == TOK_IDENTITY) {
      if (nptr->type == STRING_TYPE) {
        char *res = malloc(strlen(nptr->children[0]->val.sval) + 1);
        strcpy(res, nptr->children[0]->val.sval);
        nptr->val.sval = res;
      } else {
        nptr->val.ival = nptr->children[0]->val.ival;
      }
    }
    break;
    case NT_LEAF:
    break;
  default:
    break;
  }
  return;
}

void concat(node_t *nptr) {
  char *str1 = nptr->children[0]->val.sval;
  char *str2 = nptr->children[1]->val.sval;
  char *res = malloc(strlen(str1) + strlen(str2) + 1);
  strcpy(res, str1);
  strcat(res, str2);
  nptr->val.sval = res;
}

char *repeat(node_t *str_node, node_t *rep_node) {
    char *str = str_node->val.sval;
    int times = rep_node->val.ival;
    int len = strlen(str);
    char *res = malloc(len * times + 1);
    res[0] = '\0';

    for (int i = 0; i < times; i++)
        strcat(res, str);

    return res;
}

/* eval_root() - set the value of the root node based on the values of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The val dield of the node is updated.
 */

void eval_root(node_t *nptr) {
  if (nptr == NULL)
    return;
  // check running status
  if (terminate || ignore_input)
    return;

  // check for assignment
  if (nptr->type == ID_TYPE) {
    eval_node(nptr->children[1]);
    if (terminate || ignore_input)
      return;

    if (nptr->children[0] == NULL) {
      logging(LOG_ERROR, "failed to find child node");
      return;
    }
    put(nptr->children[0]->val.sval, nptr->children[1]);
    return;
  }

  for (int i = 0; i < 2; ++i) {
    eval_node(nptr->children[i]);
  }
  if (terminate || ignore_input)
    return;

  if (nptr->type == STRING_TYPE) {
    (nptr->val).sval = (char *)malloc(strlen(nptr->children[0]->val.sval) + 1);
    if (!nptr->val.sval) {
      logging(LOG_FATAL, "failed to allocate string");
      return;
    }
    strcpy(nptr->val.sval, nptr->children[0]->val.sval);
  } else {
    nptr->val.ival = nptr->children[0]->val.ival;
  }
  return;
}

/* infer_and_eval() - wrapper for calling infer() and eval()
 * Parameter: A pointer to a root node.
 * Return value: none.
 * Side effect: The type and val fields of the node are updated.
 */

void infer_and_eval(node_t *nptr) {
  infer_root(nptr);
  eval_root(nptr);
  return;
}

/* strrev() - helper function to reverse a given string
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str) {
  char *strcopy = malloc(strlen(str) + 1);
  strcpy(strcopy, str);
  int end = strlen(strcopy) - 1;
  int start = 0;
  while (start < end) {
    char temp = strcopy[start];
    strcopy[start] = strcopy[end];
    strcopy[end] = temp;
    start++;
    end--;
  }
  return strcopy;
}

/* cleanup() - frees the space allocated to the AST
 * Parameter: The node to free.
 */
void cleanup(node_t *nptr) {
  // Week 2 TODO: Recursively free each node in the AST
  if (nptr == NULL)
    return;
  for (int i = 0; i < 3 && nptr->children[i] != NULL; i++) {
    cleanup(nptr->children[i]);
  }
  if ((nptr->type == STRING_TYPE || nptr->type == ID_TYPE) && nptr->val.sval != NULL) {
    free(nptr->val.sval);
  }
  free(nptr);
  nptr = NULL;
  
  return;
}
