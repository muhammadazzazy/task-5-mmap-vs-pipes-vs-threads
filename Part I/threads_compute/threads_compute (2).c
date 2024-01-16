#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// int parsum = 0;
pthread_mutex_t mut;

int add(int a, int b) { return a + b; }

// struct for thread attributes, to be used by each thread for computation

struct threaddef {
  int first;
  int last;
  int *threadsnums;
  int parsum;
};

void *threadfun(void *arg) {
  struct threaddef *th = (struct threaddef *)arg;
  for (int i = th->first; i < th->last; i++) {
    pthread_mutex_lock(&mut);
    th->parsum += th->threadsnums[i];
    pthread_mutex_unlock(&mut);
  }
}

unsigned long int threads_compute(char filepath[200], int n_threads,
                                  int (*f)(int, int)) {
  int *numbers = malloc(sizeof(int) * 1000);
  int read, counter = 0;
  unsigned long int finalresult = 0;
  int n;

  pthread_t *threads = (pthread_t)malloc(sizeof(pthread_t) * n_threads);

  struct threaddef *th;
  th = malloc(sizeof(struct threaddef) * n_threads);

  FILE *infile;
  infile = fopen(filepath, "r");

  if (infile == NULL) {
    printf("Error opening file");
  }

  while (fscanf(infile, "%d", &read) != EOF) {
    numbers[counter] = read;
    counter++;
  }
  fclose(infile);

  // divide nums equally
  int num_per_thread = counter / n_threads;

  // assigning the first and last elements in each thread
  for (int i = 0; i < n_threads; i++) {
    th[i].first = i * num_per_thread;
    th[i].parsum = 0;
    if (i != n_threads - 1) {
      th[i].last = th[i].first + num_per_thread;
    } else {
      th[i].last = counter;
    }
  }
  // assigning the numbers values of the struct
  for (int i = 0; i < n_threads; i++) {
    int count = 0;
    th[i].threadsnums = malloc(sizeof(int) * (th[i].last - th[i].first));
    for (int j = th[i].first; j < th[i].last; j++) {
      th[i].threadsnums[count] = numbers[j];
      count++;
    }
  }

  // create threads and join for waiting for them to terminate
  for (int i = 0; i < n_threads; i++) {
    pthread_create(&threads[i], NULL, threadfun, (void *)&th[i]);
    pthread_join(threads[i], NULL);
  }
  pthread_mutex_destroy(&mut);

  // add the sums of each thread and store it in final result
  for (int i = 0; i < n_threads; i++) {
    finalresult = f(th[i].parsum, finalresult);
  }
  return finalresult;
}

int main(int argc, char *argv[]) {
  pthread_mutex_init(&mut, NULL);
  clock_t start, end;
  double cpu_time_used;
  char filepath[] = "numbers.txt";
  start = clock();
  unsigned long int result = threads_compute(filepath, 500, &add);
  end = clock();
  cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
  printf("result is %d \n", result);
  printf("thread Computation took %.6f seconds to execute \n", cpu_time_used);
  return 0;
}