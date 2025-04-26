#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 2048
#define MAX_LINES 100000
#define FILE_PATH "/homes/dan/625/wiki_dump.txt"

int find_max_char(const char *line){
	int max = 0;
	for(const char *p = line; *p != '\0'; p++){
		if((unsigned char)*p > max){
			max = (unsigned char)*p;
		}
	}
	return max;
}

int main(int argc, char *argv[]){
	int rank, size;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	char **lines = NULL;
	int *max_values = NULL;
	int total_lines = 0;
	
	if(rank == 0){
		FILE *fp = fopen(FILE_PATH, "r");
		if(!fp){
			perror("Failed to open file");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
		
		lines = malloc(MAX_LINES * sizeof(char *));
		char buffer[MAX_LINE_LENGTH];
		
		while (fgets(buffer, sizeof(buffer), fp)){
			if(total_lines >= MAX_LINES) break;
			lines[total_lines] = strdup(buffer);
			if(!lines[total_lines]){
				MPI_Abort(MPI_COMM_WORLD, 1);
			}
			total_lines++;
		}
		
		fclose(fp);
	}
	
	MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	int base_lines = total_lines / size;
	int remainder = total_lines % size;
	int local_count = base_lines + (rank < remainder ? 1 : 0);
	
	char *flat_lines = NULL;
	int *sendcounts = NULL;
	int *displs = NULL;
	int total_bytes = total_lines * MAX_LINE_LENGTH;
	
	if(rank == 0){
		flat_lines = malloc(total_bytes);
		for(int i = 0; i < total_lines; i++){
			snprintf(flat_lines + i * MAX_LINE_LENGTH, MAX_LINE_LENGTH, "%s", lines[i]);
		}
		
		sendcounts = malloc(size * sizeof(int));
		displs = malloc(size * sizeof(int));
		
		int offset = 0;
		for(int i = 0; i < size; i++){
			int count = base_lines + (i < remainder ? 1 : 0);
			sendcounts[i] = count * MAX_LINE_LENGTH;
			displs[i] = offset;
			offset += sendcounts[i];
		}
	}
	
	char *local_flat = malloc(local_count * MAX_LINE_LENGTH);
    MPI_Scatterv(flat_lines, sendcounts, displs, MPI_CHAR,
                 local_flat, local_count * MAX_LINE_LENGTH, MPI_CHAR,
                 0, MPI_COMM_WORLD);

    char **local_lines = malloc(local_count * sizeof(char *));
    for (int i = 0; i < local_count; i++) {
        local_lines[i] = &local_flat[i * MAX_LINE_LENGTH];
    }

    int *local_max = malloc(local_count * sizeof(int));
    for (int i = 0; i < local_count; i++) {
        local_max[i] = find_max_char(local_lines[i]);
    }

    int *recvcounts = NULL;
    int *displs_gather = NULL;
    if(rank == 0){
        max_values = malloc(total_lines * sizeof(int));
        recvcounts = malloc(size * sizeof(int));
        displs_gather = malloc(size * sizeof(int));
        for(int i = 0; i < size; i++){
            recvcounts[i] = base_lines + (i < remainder ? 1 : 0);
            displs_gather[i] = (i == 0) ? 0 : (displs_gather[i-1] + recvcounts[i-1]);
        }
    }

    MPI_Gatherv(local_max, local_count, MPI_INT, max_values, recvcounts, displs_gather, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank == 0){
        for(int i = 0; i < total_lines; i++){
            printf("%d: %d\n", i, max_values[i]);
            free(lines[i]);
        }
        free(lines);
        free(max_values);
        free(flat_lines);
        free(sendcounts);
        free(displs);
        free(recvcounts);
        free(displs_gather);
    }

    free(local_flat);
    free(local_lines);
    free(local_max);

    MPI_Finalize();
	return 0;
}