#ifndef PSEUDO_GENERIC_LINKED_LIST_H
#define PSEUDO_GENERIC_LINKED_LIST_H

#include <stddef.h>

typedef struct pseudo_generic_linked_list_t *pseudo_generic_linked_list;
pseudo_generic_linked_list pseudo_generic_linked_list_new();
void pseudo_generic_linked_list_free(pseudo_generic_linked_list list);
size_t pseudo_generic_linked_list_size(pseudo_generic_linked_list list);
int pseudo_generic_linked_list_insert_first(pseudo_generic_linked_list list,
                                            void* data);
int pseudo_generic_linked_list_insert_last(pseudo_generic_linked_list list,
                                           void* data);
int pseudo_generic_linked_list_remove_first(pseudo_generic_linked_list list,
                                            void** data);
int pseudo_generic_linked_list_remove_last(pseudo_generic_linked_list list,
                                           void** data);


// @todo move into another module?
// typedef struct pseudo_generic_linked_list_iterator* pseudo_generic_linked_list_iterator;
// pseudo_generic_linked_list_iterator pseudo_generic_linked_list_iterator_create(pseudo_generic_linked_list list);
// void* pseudo_generic_linked_list_iterator_current(pseudo_generic_linked_list_iterator it);
// int pseudo_generic_linked_list_iterator_next(pseudo_generic_linked_list_iterator it);
// int pseudo_generic_linked_list_iterator_prev(pseudo_generic_linked_list_iterator it);
// void pseudo_generic_linked_list_iterator_free(pseudo_generic_linked_list_iterator it);

#endif // PSEUDO_GENERIC_LINKED_LIST_H