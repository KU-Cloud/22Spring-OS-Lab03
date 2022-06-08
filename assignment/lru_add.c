#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>

#define EMPTY_FRAME -1

int* generate_ref_arr(size_t sz, size_t page_max);
int lru(int* ref_arr, size_t ref_arr_sz, size_t frame_sz, size_t page_max);

int _contains(int* arr, size_t sz, int target);

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
    int i, j, is_fault, target;
    int page_faults = 0;
    
    // Initializing frames
    int* frames = (int*) malloc(sizeof(int) * frame_sz);
    for (i=0; i<frame_sz; i++) frames[i] = EMPTY_FRAME;

    // Initializing additional reference bits
    uint8_t* refbits = (uint8_t*) malloc(sizeof(uint8_t) * frame_sz);
    for (i=0; i<frame_sz; i++) refbits[i] = 0;

    // Iterating reference string
    for (i=0; i<ref_arr_sz; i++) {
        is_fault = _contains(frames, frame_sz, ref_arr[i]);
        target = is_fault; // _contains returns the index of ref_arr[i] if it exists in the frames

        // Miss (page fault occurred)
        if (is_fault == -1) {
            int empty_idx = _contains(frames, frame_sz, EMPTY_FRAME);

            // Checking for empty frame slots
            if (empty_idx != EMPTY_FRAME) {
                target = empty_idx;
            } else {
                // Searching for the victim page that has the minimum refbits
                uint8_t target_refbits = 0b11111111; // max of unsigned int8
                for (j=0; j<frame_sz; j++) {
                    if (refbits[j] <= target_refbits) {
                        target = j;
                        target_refbits = refbits[j];
                    }
                }
            }
            
            // Page Replacement
            frames[target] = ref_arr[i];
            refbits[target] = 0;
            page_faults++;
        }

        // Shifting refbits
        for (j=0; j<frame_sz; j++) refbits[j] >>= 1;
        refbits[target] |= 0b10000000; // setting the first bit to 1

        // Printing current states of frames
        printf("%d | ", ref_arr[i]);
        for(j=0; j<frame_sz; j++) {
            if (frames[j] == -1) printf(". ");
            else printf("%d ", frames[j]);
        }
        if (is_fault == -1) printf("(fault)");
        printf("\n");
    }

    free(refbits);
    free(frames);

    return page_faults;
}

int _contains(int* arr, size_t sz, int target) {
    int i = 0;
    for (i=0; i<sz; i++) {
        if (arr[i] == target) return i;
    }
    return -1;
}


