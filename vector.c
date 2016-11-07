#include <stdio.h>
#include <stdlib.h>
#include "vector.h"
#include <string.h>
#include <time.h>

void vector_init(vector *v)
{
    v->capacity = VECTOR_INIT_CAPACITY;
    v->total = 0;
    v->macAddresses = malloc(sizeof(void *) * v->capacity);
    v->names = malloc(sizeof(void *) * v->capacity);
    v->processed = malloc(sizeof(void *) * v->capacity);
    v->times = malloc(sizeof(void *) * v->capacity);
}

int vector_total(vector *v){
    return v->total;
}

void set_vector_total(vector *v, int newtotal){
	v->total = newtotal;
}

void vector_resize(vector *v, int capacity){
    #ifdef DEBUG_ON
    printf("vector_resize: %d to %d\n", v->capacity, capacity);
    #endif

    void **newMacAddresses = realloc(v->macAddresses, sizeof(void *) * capacity);
    if (newMacAddresses) {
        v->macAddresses = newMacAddresses;
        v->capacity = capacity;
    }

    void **newNames = realloc(v->names, sizeof(void *) * capacity);
    if (newNames) {
        v->names = newNames;
    }

    void **newProcessed = realloc(v->processed, sizeof(void *) * capacity);
    if (newProcessed) {
        v->processed = newProcessed;
    }

    void **newTimes = realloc(v->times, sizeof(int *) * capacity);
    if (newTimes) {
        v->times = newTimes;
    }
}

void vector_add(vector *v, void *macAddress, void *name, void *processed, int *time){
    if (v->capacity == v->total)
        vector_resize(v, v->capacity * 2);
    int newindex = (v->total++);
    v->macAddresses[newindex] = macAddress;
    v->names[newindex] = name;
    v->processed[newindex] = processed;
    v->times[newindex] = time;
}

void vector_set_macAddress(vector *v, int index, void *macAddress){
    if (index >= 0 && index < v->total)
        v->macAddresses[index] = macAddress;
}

void vector_set_processed(vector *v, int index, void *processed){
    if (index >= 0 && index < v->total)
        v->processed[index] = processed;
}

void vector_set_name(vector *v, int index, void *name){
    if (index >= 0 && index < v->total)
        v->names[index] = name;
}

void vector_set_time(vector *v, int index, int *time){
    if (index >= 0 && index < v->total)
        v->times[index] = time;
}

void *vector_get_macAddress(vector *v, int index){
    if (index >= 0 && index < v->total)
        return v->macAddresses[index];
    return NULL;
}

void *vector_get_name(vector *v, int index){
    if (index >= 0 && index < v->total)
        return v->names[index];
    return NULL;
}
void *vector_get_processed(vector *v, int index){
    if (index >= 0 && index < v->total)
        return v->processed[index];
    return NULL;
}

int vector_get_time(vector *v, int index){
    if (index >= 0 && index < v->total)
        return v->times[index];
    return NULL;
}


void vector_delete(vector *v, int index){
    if (index < 0 || index >= v->total)
        return;

    v->macAddresses[index] = NULL; //set index to null
    v->names[index] = NULL;
    for (int i = index; i < v->total-1; i++) {
    		v->macAddresses[i] = v->macAddresses[i+1]; // set the current index to the next one
    		v->macAddresses[i + 1] = NULL;
    		v->names[i] = v->names[i+1]; // set the current index to the next one
    		v->names[i + 1] = NULL;
            v->processed[i] = v->processed[i+1]; // set the current index to the next one
            v->processed[i + 1] = NULL;
            v->times[i] = v->times[i+1]; // set the current index to the next one
            v->times[i + 1] = NULL;
    }

    v->total--;
    if (v->total > 0 && v->total == v->capacity / 4)
        vector_resize(v, v->capacity / 2);
}

int vector_contains(vector *v, char *element){
	int contains = 255;
	for(int i = 0; i < v->total; i ++){
		char *vectorElement = v->macAddresses[i];
		if(strcmp(vectorElement, element) == 0){
			contains = 0;
			return contains;
		}else{
			contains = 1;
		}
	}
	return contains;
}

int vector_get_index(vector *v, char *macAddress){
    int index = 0;
    for(int i = 0; i < v->total; i ++){
        char *vectorElement = v->macAddresses[i];
        if(strcmp(vectorElement, macAddress) == 0){
            index = i;
            return index;
        }
    }

    return index;
}

void vector_free(vector *v){
    free(v->macAddresses);
    free(v->names);
}



