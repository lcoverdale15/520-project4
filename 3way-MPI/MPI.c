//Reference used for better understanding of MPI: https://carleton.ca/rcs/rcdc/introduction-to-mpi/#:~:text=The%20Message%20Passing%20Interface%20(MPI,by%20passing%20messages%20between%20processes.
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 100000
#define MAX_LINE_LENGTH 2048

char **lines;
int *max_values;
int total_lines = 0;
size_t len = 0;

//Function for finding our max character in the line
int process_lines(const char *line){
	int max = 0; //initializing the max value at 0 so we know our starting point
	for(const char *p = line; *p != '\0'; p++){ //Checking every character in the line until we get to the null pointer
		if((unsigned char)*p > max){
			max = (unsigned char)*p; //if the ascii value of the character is greater than our current max, set it
		}
	}
	return max; //return the maximum value we found
}

int main(int argc, char *argv[]){
	int rank, size;
	
	MPI_Init(&argc, &argv); //initializing the MPI frame: https://www.mpich.org/static/docs/v3.3/www3/MPI_Init.html
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); //Determines the rank of the calling process in the communicator: https://www.mpich.org/static/docs/v3.3/www3/MPI_Comm_rank.html
	MPI_Comm_size(MPI_COMM_WORLD, &size); //Size of the group associated with the communicator: https://www.mpich.org/static/docs/latest/www3/MPI_Comm_size.html
	
	if(rank == 0){ //Checking if the rank of our process is 0
		FILE *fp = fopen("/homes/dan/625/wiki_dump.txt", "r"); //open the file
		if(!fp){ //check for errors opening the file
			perror("Failed to open file");
			MPI_Abort(MPI_COMM_WORLD, 1); //Terminates all processes in our MPI environment
		}
		
		lines = malloc(MAX_LINES * sizeof(char *)); //Allocate memory for lines
		max_values = malloc(MAX_LINES * sizeof(int)); //allocate memory for our max numbers
		if(!lines || !max_values)
		{
			fprintf(stderr, "Memory allocation failed\n");
			MPI_Abort(MPI_COMM_WORLD, 1); //Terminates all processes in our MPI environment
		}
		
		char *line = NULL; //create a buffer for one line of text
		
		while (getline(&line, &len, fp) != -1){ //Read a line into the buffer
			if(total_lines >= MAX_LINES) break; //If we have read the maximum amount of lines, stop reading
			lines[total_lines++] = strdup(line); //Allocate memory and copy string from buffer into lines at index total_lines
		}
		
		free(line);
		fclose(fp);
	}
	
	MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD); //Broadcast a message from process with rank 0 to all other processes: https://www.mpich.org/static/docs/v3.1/www3/MPI_Bcast.html
	
	int base_lines = total_lines / size; //Minimum number of lines each process should handle
	int remainder = total_lines % size; //Extra lines left after the above division
	int local_count = base_lines + (rank < remainder ? 1 : 0); //If our rank is less than remainder, we put one extra line; else we get base_lines
	
	char *flat_lines = NULL; //Big block of lines flattened into one char array
	int *sendcounts = NULL; //How many bytes each process should get
	int *displs = NULL; //Where each process's chunk starts inside of flat_lines
	int total_bytes = total_lines * MAX_LINE_LENGTH; //Total number of bytes as long as each line takes up MAX_LINE_LENGTH bytes
	
	if(rank == 0){ //For our initial rank
		flat_lines = malloc(total_bytes); //initialize flat_lines
		for(int i = 0; i < total_lines; i++){ 
			snprintf(flat_lines + i * MAX_LINE_LENGTH, MAX_LINE_LENGTH, "%s", lines[i]); //Copy each line into flat_lines, giving each line MAX_LINE_LENGTH even if text is shorter
		}
		
		sendcounts = malloc(size * sizeof(int)); //Initialize sendcounts
		displs = malloc(size * sizeof(int)); //initialize displs
		
		int offset = 0;
		for(int i = 0; i < size; i++){
			int count = base_lines + (i < remainder ? 1 : 0); //Calculate how many lines process gets
			sendcounts[i] = count * MAX_LINE_LENGTH; //Multiply to get bytes
			displs[i] = offset; //set to where data starts in flat_lines for our process
			offset += sendcounts[i]; //update offset for the next process (i)
		}
	}
	
	char *local_flat = malloc(local_count * MAX_LINE_LENGTH); //allocate memory for the process itself to hold a part of flat_lines
    MPI_Scatterv(flat_lines, sendcounts, displs, MPI_CHAR, local_flat, local_count * MAX_LINE_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD); //Scatter data from flat_lines into local_flat: https://www.mpi-forum.org/docs/mpi-1.1/mpi-11-html/node72.html
	//Every process now should have its own chunk of lines
    char **local_lines = malloc(local_count * sizeof(char *)); //turn local_flat into an array of pointers
    for (int i = 0; i < local_count; i++) {
        local_lines[i] = &local_flat[i * MAX_LINE_LENGTH];
    }

    int *local_max = malloc(local_count * sizeof(int)); //Find the local max for each line
    for (int i = 0; i < local_count; i++) {
        local_max[i] = process_lines(local_lines[i]);
    }
	//Each process should now have an array of max line values for its lines

    int *recvcounts = NULL; //Tells how many results are coming back
    int *displs_gather = NULL; //Tells where to place the process's results in max_values
    if(rank == 0){
        recvcounts = malloc(size * sizeof(int));
        displs_gather = malloc(size * sizeof(int));
        for(int i = 0; i < size; i++){
            recvcounts[i] = base_lines + (i < remainder ? 1 : 0);
            displs_gather[i] = (i == 0) ? 0 : (displs_gather[i-1] + recvcounts[i-1]);
        }
    }

    MPI_Gatherv(local_max, local_count, MPI_INT, max_values, recvcounts, displs_gather, MPI_INT, 0, MPI_COMM_WORLD); //Gather all local max values into max_values on rank 0: https://www.mpi-forum.org/docs/mpi-1.1/mpi-11-html/node70.html#Node70

    if(rank == 0){
        for(int i = 0; i < total_lines; i++){
            printf("%d: %d\n", i, max_values[i]); //print max ascii value for each line
            free(lines[i]); //free memory
        }
		//Freeing memory
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

    MPI_Finalize(); //Shut down MPI cleanly
	return 0;
}
