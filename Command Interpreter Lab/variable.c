/**************************************************************************
 * C S 429 EEL interpreter
 * 
 * variable.c - This file contains the skeleton of functions to be implemented
 * for EEL-2. When completed, it will contain the code to maintain a hashtable
 * for defined variables.
 * 
 * Work on it only after finishing EEL-0 and EEL-1.
 * 
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include "ci.h"

table_t *var_table = NULL;
static char *bool_print[] = {"false", "true"};
void free_entry(entry_t *entry);

void init_table(void) {
    var_table = (table_t *) calloc(1, sizeof(table_t));
    if (! var_table) {
        logging(LOG_FATAL, "failed to allocate table");
        return;
    }
    var_table->entries = (entry_t **) calloc(CAPACITY, sizeof(entry_t *));
    if (! var_table->entries) {
        free(var_table);
        logging(LOG_FATAL, "failed to allocate entries");
    }
    return;
}

void delete_entry(entry_t *eptr) {
    if (! eptr) return;
    if (eptr->type == STRING_TYPE) {
        free(eptr->val.sval);
    }
    free(eptr->id);
    free(eptr);
    return;
}

void delete_entries(entry_t *eptr) {
    if (! eptr) return;
    delete_entries(eptr->next);
    delete_entry(eptr);
    return;
}

void delete_table(void) {
    if (! var_table) return;

    for (int i = 0; i < CAPACITY; ++i) {
        delete_entries(var_table->entries[i]);
    }
    free(var_table->entries);
    free(var_table);
    return;
}

/* Pre-defined hash function to index variables by their names. */
unsigned long hash_function(char *s) {
    unsigned long i = 0;
    for (int j=0; s[j]; j++)
        i += s[j];
    return i % CAPACITY;
}

/* init_entry() - provided entry constructor
 * Parameters: Variable name, pointer to a node.
 * Return value: An allocated entry. */
entry_t * init_entry(char *id, node_t *nptr) {
    if (nptr == NULL) {
        logging(LOG_FATAL, "failed to allocate entry");
        return NULL;
    }
    entry_t *eptr = (entry_t *) calloc(1, sizeof(entry_t));
    if (! eptr) {
        logging(LOG_FATAL, "failed to allocate entry");
        return NULL;
    }
    eptr->id = (char *) malloc(strlen(id) + 1);
    if (! id) {
        logging(LOG_FATAL, "failed to allocate entry id");
        free(eptr);
        return NULL;
    }
    strcpy(eptr->id, id);
    eptr->type = nptr->type;
    if (eptr->type == STRING_TYPE) {
        (eptr->val).sval = (char *) malloc(strlen(nptr->val.sval) + 1);
        if (! eptr->val.sval) {
            logging(LOG_FATAL, "failed to allocate string");
            free(eptr->id);
            free(eptr);
            return NULL;
        }
        strcpy(eptr->val.sval, nptr->val.sval);
    } else {
        eptr->val.ival = nptr->val.ival;
    }
    return eptr;
}

/* put() - insert an entry into the hashtable or update the existing entry.
 * Use a linked list to handle collisions.
 * Parameters: Variable name, pointer to a node.
 * Return value: None.
 * Side effect: The entry is inserted into the hashtable, or is updated if
 * it already exists.
 * (STUDENT TODO) 
 */

void put(char *id, node_t *nptr) {
    unsigned long hash_value = hash_function(id);
    entry_t *current_entry = var_table->entries[hash_value];
    
    while(current_entry != NULL) {
        if(strcmp(current_entry->id, id) == 0) {
            entry_t *new_entry = init_entry(id, nptr);
            if (new_entry == NULL) {
                return; // error creating new entry, exit.
            }

            // Free the older entry and replace it with the new one
            current_entry->next = new_entry->next;
            free_entry(current_entry);
            var_table->entries[hash_value] = new_entry;

            return;
        }
        current_entry = current_entry->next;
    }

    entry_t *new_entry = init_entry(id, nptr);
    new_entry->next = NULL;

    if (var_table->entries[hash_value] == NULL) {
        var_table->entries[hash_value] = new_entry;
    } else {
        current_entry = var_table->entries[hash_value];
        while (current_entry->next != NULL) {
            current_entry = current_entry->next;
        }
        current_entry->next = new_entry;
    }
}

void free_entry(entry_t *entry) {
    if (entry != NULL) {
        if (entry->id != NULL) {
            free(entry->id);
        }
        if (entry->type == STRING_TYPE && entry->val.sval != NULL) {
            free(entry->val.sval);
        }
        free(entry);
    }
}

/* get() - search for an entry in the hashtable.
 * Parameter: Variable name.
 * Return value: Pointer to the matching entry, or NULL if not found.
 * (STUDENT TODO) 
 */

entry_t* get(char* id) {
    unsigned long hash_value = hash_function(id);
    entry_t *current_entry = var_table->entries[hash_value];
    while(current_entry != NULL) {
        if(strcmp(current_entry->id, id) == 0) {
            return current_entry;
        }
        current_entry = current_entry->next;
    }
    return NULL;
}

void print_entry(entry_t *eptr) {
    if (! eptr) return;
    switch (eptr->type) {
        case INT_TYPE:
            fprintf(outfile, "%s = %d; ", eptr->id, eptr->val.ival);
            break;
        case BOOL_TYPE:
            fprintf(outfile, "%s = %s; ", eptr->id, bool_print[eptr->val.bval]);
            break;
        case STRING_TYPE:
            fprintf(outfile, "%s = \"%s\"; ", eptr->id, eptr->val.sval);
            break;
        default:
            logging(LOG_ERROR, "unsupported entry type for printing");
            break;
    }
    print_entry(eptr->next);
    return;
}

void print_table(void) {
    if (! var_table) {
        logging(LOG_ERROR, "variable table doesn't exist");
        return;
    }
    fprintf(outfile, "\t");
    for (int i = 0; i < CAPACITY; ++i) {
        print_entry(var_table->entries[i]);
    }
    fprintf(outfile, "\n");
    return;
}
