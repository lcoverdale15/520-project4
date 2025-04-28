//Additional Reference: https://curc.readthedocs.io/en/latest/programming/OpenMP-C.html
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 1000000

char **lines;
int *max_values;
int total_lines = 0;
int NUM_THREADS;

//Function for finding our max character in the line
void process_lines(int i)
{
        int max = 0; //initializing the max value at 0 so we know our starting point
        for (char *p = lines[i]; *p != '\0'; p++) //checking every character in the line
        {
            if ((unsigned char)*p > max)
            {
                max = (unsigned char)*p; //if the ascii value of the character is greater than our current max, set it
            }
        }
        max_values[i] = max; //Save the max character for line i
}

int main()
{
    FILE *fp = fopen("/homes/dan/625/wiki_dump.txt", "r"); //open the file
    if (!fp) //check for errors opening the file
    {
        perror("Failed to open file");
        exit(1);
    }

    size_t capacity = MAX_LINES; //setting the capacity of our arrays
    lines = malloc(capacity * sizeof(char *)); //Allocating memory for lines
    max_values = malloc(capacity * sizeof(int)); //allocate memory for our max numbers
    if (!lines || !max_values) //Check memory allocated correctly
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    char *line = NULL; //Initialize a pointer buffer for a single line
    size_t len = 0; //initial size of the buffer, will be updated by getline()

    while (getline(&line, &len, fp) != -1)
    {
        if (total_lines >= capacity) //reallocate memory if necessary
        {
            capacity *= 2;
            lines = realloc(lines, capacity * sizeof(char *));
            max_values = realloc(max_values, capacity * sizeof(int));
            if (!lines || !max_values)
            {
                fprintf(stderr, "Reallocation failed\n");
                exit(1);
            }
        }
        lines[total_lines++] = strdup(line); //allocating and copying buffer
    }

    free(line); //clean up
    fclose(fp); //clean up
	
	#pragma omp parallel for //OpenMP parallelizing for loop across threads
	for (int i = 0; i < total_lines; i++)
	{
		process_lines(i); //process each line independently
	}
		
    for (int i = 0; i < total_lines; i++) //print the max values at the lines
    {
        printf("%d: %d\n", i, max_values[i]);
        free(lines[i]);
    }

    free(lines); //clean up
    free(max_values); //clean up

    printf("Program completed.\n");
    return 0;
}