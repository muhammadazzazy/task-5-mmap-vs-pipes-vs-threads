#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

typedef struct Number{
	unsigned long int num;
	struct Number *next;
} Number;

void append(Number** v, unsigned long int number){
	Number* new_number;
    new_number = (Number*)malloc(sizeof(Number));
    new_number->num = number;
    new_number->next = NULL;
    if(*v == NULL){
        (*v) = new_number;
    }
    else{
        Number *temp;
        temp = (*v);
        while(temp->next != NULL){	
            temp = temp->next;
        }
        temp->next = new_number;
    }
    return;
}

unsigned long int add(unsigned long int a, unsigned long int b){
	return a+b;
}

unsigned long int mul(unsigned long int a, unsigned long int b){
	return a*b;
}

unsigned long int max(unsigned long int a, unsigned long int b){
	if(a > b)
		return a;
	else
		return b;
}

unsigned long int min(unsigned long int a, unsigned long int b){
	if(a < b)
		return a;
	else
		return b;
}

unsigned long int and(unsigned long int a, unsigned long int b){
	return a & b;
}

unsigned long int or(unsigned long int a, unsigned long int b){
	return a | b;
}

unsigned long int xor(unsigned long int a, unsigned long int b){
	return a^b;
}

void print(unsigned long int* ptr, int N){
	printf("memory\n");
	for(int i = 0; i < N; i++){
		printf("%lu\n", ptr[i]);
	}
	return;
}

unsigned long int mmap_compute(int n_proc, char* filepath, unsigned long int f(unsigned long int, unsigned long int)){
	unsigned long int result = 0;
	Number* head = NULL; 
	FILE* fh = fopen(filepath, "r");
	int N = 0;
	unsigned long int val;
	while(!feof(fh)){
		fscanf(fh, "%lu", &val);
		append(&head, val);
		N++;
	}
	unsigned long int *ptr = mmap(NULL, (N+n_proc+1)*sizeof(unsigned long int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	if(ptr == MAP_FAILED){
		printf("Mapping Failed\n");
	}
	for(int i = 0; i < N; i++){
		ptr[i] = head->num;
		head = head->next;
	}
	for(int i = 0; i < n_proc-1; i++){
		pid_t child_pid = fork();
		
		if(child_pid == 0) {
			if((N-1)/n_proc == 0)
				ptr[N+i+1] = (*f)(ptr[N+i], ptr[N+i+1]);
			else {
				for(int j = 0; j < (N-1)/n_proc; j++){
					ptr[N+i+j+1] = (*f)(ptr[N+i+j], ptr[N+i+j+1]);
					ptr[N+n_proc]++;
				}
			}
			exit(0);
		}
		
	}
	wait(NULL);
	for(int k = ptr[N+n_proc-1]; k < N-1; k++){
		ptr[k+1] = (*f)(ptr[k], ptr[k+1]);
		ptr[N+n_proc]++;
	}
	
	result = ptr[N-1];
	munmap(ptr, (N+1)*(sizeof(unsigned long int)));
	return result;

}

int main(int argc, char* argv[]){
	if(argc != 4){
		printf("Three args required!\n");
		exit(1);
	}
	// clock_t start, end;
	// start = clock();
	int n_proc = atoi(argv[1]);
	char filepath[100];
	memcpy(filepath, argv[2], strlen(argv[2]));
	char function[3];
	memcpy(function, argv[3], strlen(argv[3]));
	int index;
	if(strcmp(function, "add"))
		index = 0;
	else if(strcmp(function, "mul"))
		index = 1;
	else if(strcmp(function, "max"))
		index = 2;
	else if(strcmp(function, "min"))
		index = 3;
	else if(strcmp(function, "and"))
		index = 4;
	else if(strcmp(function, "or"))
		index = 5;
	else if((function,"xor"))
		index = 6;
	unsigned long int (*f[])(unsigned long int, unsigned long int) = {add, mul, max, min, and, or, xor};
	printf("%lu\n", mmap_compute(n_proc, filepath, *f[index]));
	// end = clock();
	// double execution_time = ((double)(end - start))/CLOCKS_PER_SEC;
	// printf("%f\n", execution_time);
	return 0;
}