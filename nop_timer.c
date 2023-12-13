#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to execute NOP instructions
void execute_nops(unsigned int count) {
    while (count--) {
        asm volatile("nop");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <count>\n", argv[0]);
        return EXIT_FAILURE;
    }

    unsigned int count = strtoul(argv[1], NULL, 10);
    struct timespec start, end;
    long long elapsed_time;

    // Start the timer
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Execute NOP instructions
    execute_nops(count);

    // Stop the timer
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in nanoseconds
    elapsed_time = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);

    printf("Execution of %u NOP instructions took %lld nanoseconds.\n", count, elapsed_time);

    return EXIT_SUCCESS;
}
