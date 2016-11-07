#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_INIT_CAPACITY 10

#define VECTOR_INIT(vec) vector vec; vector_init(&vec)
#define VECTOR_ADD(vec, item, item2, item3, item4) vector_add(&vec, (void *) item, (void *) item2,(void *) item3,(int *) item4)
#define VECTOR_SET_MACADDRESS(vec, id, item) vector_set_macAddress(&vec, id, (void *) item)
#define VECTOR_SET_NAME(vec, id, item) vector_set_name(&vec, id, (void *) item)
#define VECTOR_SET_PROCESSED(vec, id, item) vector_set_processed(&vec, id, (void *) item)
#define VECTOR_SET_TIME(vec, id, item) vector_set_time(&vec, id, (int *) item)
#define VECTOR_GET_MACADDRESS(vec, type, id) (type) vector_get_macAddress(&vec, id)
#define VECTOR_GET_NAME(vec, type, id) (type) vector_get_name(&vec, id)
#define VECTOR_GET_PROCESSED(vec, type, id) (type) vector_get_processed(&vec, id)
#define VECTOR_GET_TIME(vec, type, id) (type) vector_get_time(&vec, id)

#define VECTOR_DELETE(vec, id) vector_delete(&vec, id)
#define VECTOR_TOTAL(vec) vector_total(&vec)
#define VECTOR_FREE(vec) vector_free(&vec)

typedef struct vector {
    char **macAddresses;
    char **names;
    char **processed;
    int **times;
    int capacity;
    int total;
} vector;

void vector_init(vector *);
int vector_total(vector *);
void vector_resize(vector *, int);
void vector_add(vector *, void *, void *, void *, int *);
void vector_set_macAddress(vector *, int, void *);
void vector_set_name(vector *, int, void *);
void vector_set_processed(vector *, int, void *);
void vector_set_time(vector *, int, int *);
void *vector_get_macAddress(vector *, int);
void *vector_get_name(vector *, int);
void *vector_get_processed(vector *, int);
int vector_get_time(vector *, int);
void vector_delete(vector *, int);
void vector_free(vector *);
int vector_contains(vector *, char *);
void set_vector_total(vector *, int);
int vector_get_index(vector *v, char *macAddress);
#endif
