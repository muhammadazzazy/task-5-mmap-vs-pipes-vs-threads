#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void parallel_compute(char filename[200], int n_proc,
                      float (*fun_ptr)(float, float)) {
  int N = 0, start, stop;
  float result, data[1000];

  // read data

  FILE *infile;
  infile = fopen(filename, "r");
  if (infile == NULL) {
    fprintf(stderr, "\nError opening file\n");
    exit(-1);
  }

  while (!feof(infile)) {
    fscanf(infile, "%f", &data[N]);
    N++;
    if (N > 1000) {
      printf("Invalid Input ! File is too large\n");
      exit(0);
    }
  }

  fclose(infile);

  if (n_proc == 0) {
    printf("Invalid Input ! \n");
    exit(0);
  }
  if (n_proc > N)
    n_proc = N + 1; // maximum # of process possible
  if (n_proc == 1)  // only parent process
  {
    result = data[0];
    for (int i = 1; i < N; i++)
      result = (*fun_ptr)(result, data[i]);
    printf("Final Result = %f \n", result);
    exit(0);
  }

int num_per_proc = N/n_proc ;

int pipes[2];
  if(pipe(pipes)==-1) printf("Error with creating pipe \n");
  int pids[n_proc];
  for(int i =0 ; i<n_proc ;i++)
    {

    pids[i]=fork();
      if(pids[i]==0)
      {
        float partical_result;
        if(i==0)
        {
          int start =1;
          int stop = num_per_proc;
          partical_result=data[0];
          for(int i=start ;i<stop;i++)
            partical_result = (*fun_ptr)(partical_result, data[i]); 
        }
        else
        {
          int start =i*num_per_proc;
          int stop;
          if(i==n_proc-1) stop=N;
          else stop = start+num_per_proc;
          read(pipes[0],&partical_result ,sizeof(float));
          for(int i=start ;i<stop;i++)
            partical_result = (*fun_ptr)(partical_result, data[i]); 
        }
        write(pipes[1],&partical_result ,sizeof(float));
        exit(0);
      }
        wait(NULL);
    }
  read(pipes[0],&result ,sizeof(float));
  printf("Final Result Equals %f \n",result);
    
}

float add(float a, float b) { return a + b; }

float multiply(float a, float b) { return a * b; }

int main() {
  char filename[50];
  printf("Enter File Path\n");
  scanf("%s", filename);

  int n_proc;
  printf("Enter Number of Process\n");
  scanf("%d", &n_proc);

  float (*fun_ptr)(float, float) = &add;

  parallel_compute(filename, n_proc, fun_ptr);

  return 0;
}