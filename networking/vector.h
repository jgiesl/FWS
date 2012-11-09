/**
 * Hamid Alipour
 */
 
#ifndef __VECTOR__
#define __VECTOR__
 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
 
#define VECTOR_INIT_SIZE    4
#define VECTOR_HASSPACE(v)  (((v)->num_elems + 1) <= (v)->num_alloc_elems)
#define VECTOR_INBOUNDS(i)	(((int) i) >= 0 && (i) < (v)->num_elems)
#define VECTOR_INDEX(i)		((char *) (v)->elems + ((v)->elem_size * (i)))
 
typedef struct
{
	void *elems;
	size_t elem_size;
	size_t num_elems;
	size_t num_alloc_elems;
    void (*free_func)(void *);
} vector_t;
 
extern void vector_init(vector_t*, size_t, size_t, void (*free_func)(void*));
extern void vector_dispose(vector_t*);
extern void vector_copy(vector_t*, vector_t*);
extern void vector_insert(vector_t*, void*, size_t index);
extern void vector_insert_at(vector_t*, void *, size_t index);
extern void vector_push(vector_t*, void*);
extern void vector_pop(vector_t*, void*);
extern void vector_shift(vector_t*, void*);
extern void vector_unshift(vector_t*, void*);
extern void vector_get(vector_t*, size_t, void*);
extern void vector_remove(vector_t*, size_t);
extern void vector_transpose(vector_t*, size_t, size_t);
extern size_t vector_length(vector_t*);
extern size_t vector_size(vector_t*);
extern void vector_get_all(vector_t*, void*);
extern void vector_cmp_all(vector_t*, void*, int (*cmp_func)(const void*, const void*));
extern void vector_qsort(vector_t*, int (*cmp_func)(const void*, const void*));
static void vector_grow(vector_t*, size_t);
static void vector_swap(void*, void*, size_t);
 
#endif
