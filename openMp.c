#include <omp.h>
#include <stdio.h>

void main(){

#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		int total = omp_get_num_threads();
		printf("Hello world from thread %d of %d\n", tid, total);
	}

	return 0;
}
