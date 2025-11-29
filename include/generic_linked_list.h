#ifndef GENERIC_LINKED_LIST_H
#define GENERIC_LINKED_LIST_H

#include <stddef.h>

typedef struct generic_linked_list_t* generic_linked_list;

int
generic_linked_list_new(generic_linked_list* out_self);

int
generic_linked_list_set_free_function(generic_linked_list self,
                                      void (*free_function)(void*));

int
generic_linked_list_get_free_function(generic_linked_list self,
                                      void (**out_free_function)(void*));

int
generic_linked_list_set_copy_function(generic_linked_list self,
                                      int (*copy_function)(void*, void**));

int
generic_linked_list_get_copy_function(generic_linked_list self,
                                      int (**out_copy_function)(void*, void**));

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

typedef struct generic_linked_list_iterator_t* generic_linked_list_iterator;

int
generic_linked_list_begin(generic_linked_list ll,
                          generic_linked_list_iterator* out_self);

int
generic_linked_list_end(generic_linked_list ll,
                        generic_linked_list_iterator* out_self);

int
generic_linked_list_reverse_begin(generic_linked_list ll,
                                  generic_linked_list_iterator* out_self);

int
generic_linked_list_reverse_end(generic_linked_list ll,
                                generic_linked_list_iterator* out_self);

int
generic_linked_list_iterator_get(generic_linked_list_iterator self,
                                 void** out_data);

int
generic_linked_list_iterator_next(generic_linked_list_iterator self);

int
generic_linked_list_iterator_prev(generic_linked_list_iterator self);

int
generic_linked_list_iterator_is_valid(generic_linked_list_iterator self);

int
generic_linked_list_iterator_is_end(generic_linked_list_iterator self);

int
generic_linked_list_iterator_remove(generic_linked_list_iterator self,
                                    generic_linked_list_iterator* out_next);

int
generic_linked_list_iterator_insert_before(
    generic_linked_list_iterator self, void* data,
    generic_linked_list_iterator* out_new_iter);

int
generic_linked_list_iterator_insert_after(
    generic_linked_list_iterator self, void* data,
    generic_linked_list_iterator* out_new_iter);

int
generic_linked_list_iterator_distance(generic_linked_list_iterator start,
                                      generic_linked_list_iterator end,
                                      size_t* out_distance);

int
generic_linked_list_iterator_advance(generic_linked_list_iterator self,
                                     size_t n);

int
generic_linked_list_iterator_find(generic_linked_list_iterator start,
                                  generic_linked_list_iterator end,
                                  void* target, int (*compare)(void*, void*),
                                  generic_linked_list_iterator* out_found);

int
generic_linked_list_iterator_free(generic_linked_list_iterator self);

#endif

// @todo move the generic_linked_list related modules into a separate repo and
// include it through git sub-module.