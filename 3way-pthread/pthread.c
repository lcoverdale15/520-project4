#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define MAX_LINES 1000000   // just for initial allocation

char **lines;
int *max_values;
int total_lines = 0;

void *process_lines(void *arg) {
    long tid = (long)arg;
    int start = tid * (total_lines / NUM_THREADS);
    int end = (tid == NUM_THREADS - 1) ? total_lines : start + (total_lines / NUM_THREADS);

    for (int i = start; i < end; i++) {
        int max = 0;
        for (char *p = lines[i]; *p != '\0'; p++) {
            if ((unsigned char)*p > max) {
                max = (unsigned char)*p;
            }
        }
        max_values[i] = max;
    }

    pthread_exit(NULL);
}

int main() {
    FILE *fp = fopen("/homes/dan/625/wiki_dump.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        exit(1);
    }

    size_t capacity = MAX_LINES;
    lines = malloc(capacity * sizeof(char *));
    max_values = malloc(capacity * sizeof(int));
    if (!lines || !max_values) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, fp) != -1) {
        if (total_lines >= capacity) {
            capacity *= 2;
            lines = realloc(lines, capacity * sizeof(char *));
            max_values = realloc(max_values, capacity * sizeof(int));
            if (!lines || !max_values) {
                fprintf(stderr, "Reallocation failed\n");
                exit(1);
            }
        }
        lines[total_lines++] = strdup(line);
    }

    free(line);
    fclose(fp);

    pthread_t threads[NUM_THREADS];

    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, process_lines, (void *)t);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // Print the results in the correct order
    for (int i = 0; i < total_lines; i++) {
        printf("%d: %d\n", i, max_values[i]);
        free(lines[i]);
    }

    free(lines);
    free(max_values);

    printf("Program completed.\n");
    return 0;
}

