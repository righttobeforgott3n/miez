#ifndef GENERIC_LINKED_LIST_H
#define GENERIC_LINKED_LIST_H

#include <stddef.h>

typedef struct generic_linked_list_t* generic_linked_list;

int
generic_linked_list_new(generic_linked_list* out_self);

int
generic_linked_list_free(generic_linked_list self);

int
generic_linked_list_size(generic_linked_list self, size_t* out_size);

int
generic_linked_list_insert_first(generic_linked_list self, void* data);
int
generic_linked_list_insert_last(generic_linked_list self, void* data);

int
generic_linked_list_remove_first(generic_linked_list self, void** out_data);

int
generic_linked_list_remove_last(generic_linked_list self, void** out_data);

#endif  // GENERIC_LINKED_LIST_H