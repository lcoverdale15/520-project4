#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_SIZE 16
#define ALPHABET_SIZE 26
#define NUM_THREADS 4

pthread_mutex_t mutexsum;
char **char_array = NULL;
int char_counts[ALPHABET_SIZE];
long total_lines = 0;

void read_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    long capacity = 100000;  // Start with 100k lines, grow if needed
    char_array = malloc(capacity * sizeof(char *));
    if (!char_array) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    char buffer[STRING_SIZE + 2]; // +2 for newline and null terminator
    while (fgets(buffer, sizeof(buffer), fp)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';

        if (total_lines >= capacity) {
            capacity *= 2;
            char_array = realloc(char_array, capacity * sizeof(char *));
            if (!char_array) {
                perror("Realloc failed");
                exit(EXIT_FAILURE);
            }
        }

        char_array[total_lines] = malloc(STRING_SIZE);
        strncpy(char_array[total_lines], buffer, STRING_SIZE);
        total_lines++;
    }

    fclose(fp);
}

void *count_array(void *arg) {
    long thread_id = (long)arg;
    long start = thread_id * (total_lines / NUM_THREADS);
    long end = (thread_id == NUM_THREADS - 1) ? total_lines : start + (total_lines / NUM_THREADS);

    int local_counts[ALPHABET_SIZE] = {0};

    for (long i = start; i < end; i++) {
        for (int j = 0; j < STRING_SIZE && char_array[i][j]; j++) {
            char c = char_array[i][j];
            if (c >= 'a' && c <= 'z') {
                local_counts[c - 'a']++;
            }
        }
    }

    pthread_mutex_lock(&mutexsum);
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        char_counts[i] += local_counts[i];
    }
    pthread_mutex_unlock(&mutexsum);

    pthread_exit(NULL);
}

void print_results() {
    int total = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        total += char_counts[i];
        printf(" %c %d\n", (char)(i + 'a'), char_counts[i]);
    }
    printf("\nTotal characters: %d\n", total);
}

int main() {
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;

    pthread_mutex_init(&mutexsum, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    read_file("/homes/dan/625/wiki_dump.txt");

    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], &attr, count_array, (void *)i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    print_results();

    // Cleanup
    for (long i = 0; i < total_lines; i++) {
        free(char_array[i]);
    }
    free(char_array);

    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutexsum);
    pthread_exit(NULL);
}
