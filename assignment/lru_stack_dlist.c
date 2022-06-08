#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>

int* generate_ref_arr(size_t sz, size_t page_max);
int lru(int* ref_arr, size_t ref_arr_sz, size_t frame_sz, size_t page_max);

typedef struct LRUCache LRUCache;
typedef struct LRUCachePage LRUCachePage;
LRUCache* new_lru_cache(size_t capacity);
int lru_cache_add(LRUCache* stack, int new_page);
int mark_page_referenced(LRUCache* stack, int page);
void lru_cache_replace(LRUCache* stack, int new_page);
void clear_lru_cache(LRUCache* stack);
void print_lru_cache(LRUCache* stack);

// Driver code
//  Usage: ./program {ref_arr_size} {page_max} {frame_size}
int main(int argc, char** argv) {
    if (argc < 4) return -1;
    
    int ref_arr_sz = atoi(argv[1]);
    int page_max = atoi(argv[2]);
    int frame_sz = atoi(argv[3]);

    int ref[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    int page_faults = lru(ref, 20, 4, 7);
    // int* ref = generate_ref_arr(ref_arr_sz, page_max);
    // int page_faults = lru(ref, ref_arr_sz, frame_sz, page_max);
    printf("Page Faults : %d\n", page_faults);
    // free(ref);

    return 0;
}

int* generate_ref_arr(size_t sz, size_t page_max) {
    int* ref_arr = (int*) malloc(sizeof(int) * sz);
    int i, prev = 0, weight;

    srand(time(NULL));
    for (i=0; i<sz; i++) {
        weight = rand() % 10;
        if (weight < 3) ref_arr[i] = prev; // 30%
        else ref_arr[i] = rand() % (page_max + 1);
        prev = ref_arr[i];
    }

    return ref_arr;
}

int lru(int* ref_arr, size_t ref_arr_sz, size_t frame_sz, size_t page_max) {
    int i, j, hit, is_full, target;
    int page_faults = 0;
    
    // Initializing frames
    LRUCache* stack = new_lru_cache(frame_sz);

    // Iterating reference string
    for (i=0; i<ref_arr_sz; i++) {
        hit = mark_page_referenced(stack, ref_arr[i]);

        // Miss (page fault occurred)
        if (hit == 0) {
            is_full = lru_cache_add(stack, ref_arr[i]);
            if (is_full == 0) lru_cache_replace(stack, ref_arr[i]);
            page_faults++;
        }

        // Printing current states of frames
        printf("%d | ", ref_arr[i]);
        print_lru_cache(stack);
        if (hit == 0) printf("(fault)");
        printf("\n");
    }

    clear_lru_cache(stack);
    free(stack);

    return page_faults;
}

struct LRUCache {
    LRUCachePage* top;
    LRUCachePage* bottom;
    size_t size;
    size_t capacity;
};

struct LRUCachePage {
    LRUCachePage* prev;
    LRUCachePage* next;
    int page;
};

LRUCache* new_lru_cache(size_t capacity) {
    LRUCache* cache = (LRUCache*) malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->top = NULL;
    cache->bottom = NULL;
    return cache;
}

int lru_cache_add(LRUCache* stack, int new_page) {
    if (stack->size >= stack->capacity) return 0;

    LRUCachePage* page = (LRUCachePage*) malloc(sizeof(LRUCachePage));
    page->page = new_page;

    if (stack->size == 0) {
        page->prev = NULL;
        page->next = NULL;
        stack->top = page;
        stack->bottom = page;    
    } else {
        page->prev = stack->top;
        page->next = NULL;

        stack->top->next = page;
        stack->top = page;
    }
    stack->size++;
    return 1;
}

int mark_page_referenced(LRUCache* stack, int page) {
    if (stack->size == 0) return 0;

    LRUCachePage* iter = stack->top;
    while (iter != NULL) {
        if (iter->page == page) break;
        iter = iter->prev;
    }
    if (iter == NULL) return 0;

    if (iter == stack->top) return 1;
    else if (iter == stack->bottom) {
        stack->top->next = iter;
        stack->bottom = iter->next;

        iter->prev = stack->top;
        iter->next->prev = NULL;
        iter->next = NULL;

        stack->top = iter;
    } else {
        iter->prev->next = iter->next;
        iter->next->prev = iter->prev;

        iter->prev = stack->top;
        stack->top->next = iter;
        stack->top = iter;
    }

    return 1;
}

void lru_cache_replace(LRUCache* stack, int new_page) {
    LRUCachePage* victim = stack->bottom;
    victim->next->prev = NULL;
    stack->bottom = victim->next;
    free(victim);

    LRUCachePage* page = (LRUCachePage*) malloc(sizeof(LRUCachePage));
    page->page = new_page;
    page->prev = stack->top;
    page->next = NULL;

    stack->top->next = page;
    stack->top = page;
}

void clear_lru_cache(LRUCache* stack) {
    if (stack->size == 0) return;
    
    LRUCachePage* iter = stack->top;
    LRUCachePage* target;
    while (iter != NULL) {
        target = iter;
        iter = iter->prev;
        free(target);
        stack->size--;
    }
}

void print_lru_cache(LRUCache* stack) {
    printf("[Stack %zu] ", stack->size);

    if (stack->size == 0) {
        printf("empty ");
        return;
    }

    LRUCachePage* iter = stack->top;
    printf("top-> ");
    while (iter != NULL) {
        printf("%d ", iter->page);
        iter = iter->prev;
    }
}