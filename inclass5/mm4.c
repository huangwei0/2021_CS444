#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>

// gcc -g -Wall -pthread -lrt -o mm4 mm4.c

#ifndef MAX_DIM
# define MAX_DIM 4000
#endif // MAX_DIM

#ifndef DEF_DIM
# define DEF_DIM 1000
#endif // DEF_DIM

static float **matrix1 = NULL;
static float **matrix2 = NULL;
static float **result = NULL;
static int dim = DEF_DIM;
static int num_threads = 1;

float **alloc_matrix(void);
void free_matrix(float **);
void init(float **, float **, float **);
void op_mat(float **);
void *mult(void *);
int get_next_row(void);

void
op_mat(float **mat1)
{
 FILE *op = NULL;
 int i = -1;
 int j = -1;

#define FILE_NAME "mm4.txt"
    op = fopen(FILE_NAME, "w");
    if (op == NULL) {
      perror("Could not open file:" FILE_NAME);
      exit(17);
    }
    for(i = 0; i < dim; i++){
      for(j = 0; j < dim ; j++){
        fprintf(op, "%8.2f", result[i][j]);
      }
      fprintf(op, "\n");
    }
    fclose(op);
}

float **
alloc_matrix(void)
{
  float **mat =  malloc(dim * sizeof(float *));
  int i = -1;

  for(i = 0; i < dim; i++){
    mat[i] =  malloc(dim * sizeof(float));
  }
  return mat;
}

void
free_matrix(float **mat)
{
  int i = -1;

  for (i = 0; i < dim; i++){
    free(mat[i]);
  }
  free (mat);
}

void
init(float **mat1, float **mat2, float **res)
{
  int i = -1;
  int j = -1;

  for(i = 0; i < dim; i++){
    for(j = 0; j < dim; j++){
      mat1[i][j] = (i + j) * 2.0;
      mat2[i][j] = (i + j) * 3.0;
      res[i][j] = 0.0;
    }
  }
}

#define SHARED_THREADS 0
#define SHARED_PROCS 1
static sem_t lock;

int
get_next_row(void)
{
  static int next_row = 0;
  int cur_row = 0;

  sem_wait(&lock);
  cur_row = next_row++;
  sem_post(&lock);

  return cur_row;
}

void *
mult(void *vid)
{
  int i = -1;
  int j = -1;
  int k = -1;
//  long tid =(long) (vid);

  for(i = get_next_row(); i < dim; i =get_next_row()){
    for(j = 0; j < dim; j++) {
      for(k = 0; k < dim; k++){
        result[i][j] += matrix1[i][k] * matrix2[k][j];
      }
    }
  }
  pthread_exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[])
{
pthread_t *threads = NULL;
long tid = 0;

  {
    int opt = -1;

    while ((opt = getopt(argc, argv, "t:d:h")) != -1){
      switch (opt) {
        case 't':
        num_threads = atoi(optarg);
          break;
        case 'd':
          dim = atoi (optarg);
          if(dim < DEF_DIM) {
            dim = DEF_DIM;
          }
          if (dim > MAX_DIM){
            dim = MAX_DIM;
          }
          break;
        case 'h':
          break;
        default:
          break;
      }
    }
  }

if(num_threads < 1){
    num_threads = 1;
}
threads =  malloc(num_threads * sizeof(pthread_t));

  matrix1 = alloc_matrix();
  matrix2 = alloc_matrix();
  result = alloc_matrix();

  init(matrix1, matrix2, result);
  sem_init(&lock, SHARED_THREADS, 1);

  for(tid = 0; tid < num_threads; tid++){
    pthread_create(&threads[tid], NULL, mult, (void *) tid);
  }
  for(tid = 0; tid < num_threads; tid++){
    pthread_join(threads[tid], NULL);
  }

  op_mat(result);

  free_matrix(matrix1);
  free_matrix(matrix2);
  free_matrix(result);
  matrix1 = matrix2 = result = NULL;
  free(threads);

  return EXIT_SUCCESS;
}
