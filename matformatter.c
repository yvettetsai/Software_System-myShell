/*
 * 
 * Name: matformatter.c
 * 
 * Written by:
 *   Cody Doucette
 *   Yvette Tsai
 * 
 * Description:
 *   This program accepts an input matrix from stdin and writes to stdout the 
 *   transpose of the given matrix.
 * 
 * Sample interaction:
 * 
 *   1 2 3         // Input matrix.
 *   4 5 6
 *   
 *   1 4           // Output, transposed matrix. 
 *   2 5
 *   3 6
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>

#define TEMP_FILE   "./matformatter_temp.txt"

int **a;                   // Matrices.
int **t;

int a_rows = 0;            // Dimensions of matrices.
int a_cols = 0;


             /******************************************
              * 
              *            MATRIX FUNCTIONS
              * 
              ******************************************/


/*
 * countline(): computes the number of integers on a given line.
 */
int countline(char *line) {

    int total = 0;

    while (*line != '\n') {
        if (isdigit(*line) || (*line == '-')) {
            total++;
            while (isdigit(*(++line)));
        } 
        else if (isspace(*line)) {
            line++;
        }
        else {
            printf("Invalid input character '%c'. Terminating.\n", *line);
            exit(1);
        }
    }

    return total;
}

/*
 * initialize_array(): allocates and returns an array of the given dimensions.
 */
int **initialize_array(int rows, int cols) {

    int **array;
    array = (int**) malloc(rows * sizeof (int*));
    int i;
    for (i = 0; i < rows; i++)
        array[i] = (int*) malloc(cols * sizeof (int));

    return array;
}

/*
 * free_array(): frees the allocated memory of a given array.
 */
void free_array(int **array, int rows) {
    int i;

    for (i = 0; i < rows; i++)
        free(array[i]);
    
    free(array);
}

/*
 * fill_array(): fill the given array with the integers found on the given line.
 */
void fill_array(int *array, char *line) {

    char *pch;
    int i = 0;

    pch = strtok(line, " ");

    while (pch != NULL) {
        array[i++] = atoi(pch);
        pch = strtok(NULL, " ");
    }
}

/*
 * transpose(): calculates the transpose of a given matrix.
 */
void transpose() {
    
    int i, j;
    
    for (i = 0; i < a_rows; i++) {
        for (j = 0; j < a_cols; j++) {
            t[j][i] = a[i][j];
        }
    }
    
}


/*
 * main(): reads an input matrix from stdin and eventually calculates the 
 * transpose of the matrix. 
 */
int main(int argc, char *argv[]) {
    
    int nbytes = 256;
    char *line = (char *) malloc(nbytes + 1);

    FILE *fout;

    if ((fout = fopen(TEMP_FILE, "w+")) == NULL) {
        printf("Error: Cannot open file for processing matrices. Terminating.\n");
        exit(1);
    }

    while (1) {

        getline(&line, &nbytes, stdin);
        fprintf(fout, "%s", line);

        int num = countline(line);

        if (num == 0) {

            if (a_rows == 0) {
                printf("Error: No input matrix. Terminating.\n");
                exit(1);
            }
            
            break;

        } else {
            a_rows++;
            if (a_cols == 0) {
                a_cols = num;
            } else if (num != a_cols) {
                printf("Error: Inconsistent number of columns in A. "
                       "Terminating.\n");
                exit(1);
            }
        } 

    }
    

    if (fseek(fout, 0, SEEK_SET) < 0) {
        printf("Error with file to process matrices. Terminating.\n");
        exit(1);
    }

    a = initialize_array(a_rows, a_cols);
    t = initialize_array(a_cols, a_rows);

    int i;
    for (i = 0; i < a_rows; i++) {
        getline(&line, &nbytes, fout);
        fill_array(a[i], line);
    }
    
    free(line);
    fclose(fout);
    remove(TEMP_FILE);
    
    transpose(a);
    
    int j;

    // Print out product matrix.
    for (i = 0; i < a_cols; i++) {
        for (j = 0; j < a_rows; j++) {
            printf("%d ", t[i][j]);
        }
        printf("\n");   
    }
    printf("\n");

    free_array(a, a_rows);
    free_array(t, a_cols);
    
    return 0;
}


