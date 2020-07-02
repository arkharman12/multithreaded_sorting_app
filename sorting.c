#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>       /* log2 */


/* Global mutex */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Declaration of thread condition variable 
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 
  
int slice;
int n;  // size of array
int m; // # of threads

// Barrier: 
//int remain = n;
//pthread_cond_t cv;


// array for random numbers
int *array; 

struct tsk {
	int id;
	int low;
	int high;
};

// merge two parts
// merge function for merging two parts
void merge(int low, int mid, int high)
{   
    int i = 0;
    int j = 0;
    int k = 0;
    int left_length = mid - low + 1;
    int right_length = high - mid;
    int left_array[left_length];
    int right_array[right_length];
    
    for (int i = 0; i < left_length; i ++) {
        left_array[i] = array[low + i];
    }
    
    for (int j = 0; j < right_length; j ++) {
        right_array[j] = array[mid + 1 + j];
    }
    
    i = 0;
    j = 0;
	
    while (i < left_length && j < right_length) {
        if (left_array[i] <= right_array[j]) {
            array[low + k] = left_array[i];
            i ++;
        } else {
            array[low + k] = right_array[j];
            j ++;
        }
        k ++;
    }
    
    while (i < left_length) {
        array[low + k] = left_array[i];
        k ++;
        i ++;
    }
    while (j < right_length) {
        array[low + k] = right_array[j];
        k ++;
        j ++;
    }
}



// merge sort
void merge_sort(int low, int high){
	int mid = low + (high - low) / 2;
	
	if( low < high){
		// calling first half
		merge_sort(low, mid);
		
		// calling middle part
		merge_sort(mid+1, high);
		
		// merge the two halfs
		merge(low,mid,high);
	}
}


void mergeSplitArray(int id , int collection){
	for(int i=0; i < id; i = i + 2){
		int low = i * (slice * collection);
		int right = ((i + 2) * slice * collection) - 1;
		int middle = low + (slice * collection) - 1;
		merge(low, middle, right);
	}
}


void localSync(int n){
	int bob = n +1;
	merge(0, (n / 2 - 1) / 2, n / 2 - 1);
	merge(n / 2, n / 2 + (n - 1 - n / 2) / 2, n - 1);
	merge(0, (n - 1) / 2, n - 1);
}

void *start(void *arg){
	struct tsk *tsk = arg;
	int low;
	int high;
	int levels = log2 (m); // gets the # of levels 
	// get low and high of each thread
	low = tsk->low;
	high=tsk->high;
	//printf("%d", low);
	
	// mid-point
	int mid = low + (high - low) / 2;
	if (low < high){
		merge_sort(low,mid);
		merge_sort(mid+1,high);
		merge(low, mid, high);
		
	}
	int i;
	
	/* Local Synchronization
	
	for(i = 0; i < levels; i++){
		
		if(id % pow(i+1,2)  ==  0){
			pthread_mutex_lock(&mutex);
			pthread_cond_wait(&cond, &mutex); 
			pthread_mutex_unlock(&mutex);
			merge(low,mid,high)
		}else{
			pthread_cond_singal(&cond1)
		}
	}
	
	*/
	
	
	/*Barrier Synchronization
	pthread_mutex_lock(&mutex);
	remain--; 
	if (remain == 0) { pthread_cond_broadcast(&cond; }
	else {
		while (remain != 0) { pthread_cond_wait(&cond, &m); }
	}
	merge(low,mid,high)
	pthread_mutex_unlock(&mmutex;
	
	
	*/
}
	

int main(int argc, char** argv) 
{ 	
	n = atoll(argv[1]);
	m = atoll(argv[2]);

	 FILE *fptr;
	 fptr = fopen(argv[3],"w");
	if(fptr == NULL)
	{
	  printf("Error!");   
	  exit(1);             
	}
		slice = n / m;
	printf("The size of array is: %d\n",n);
	printf("The # of threads is: %d\n",m);
	
	array = malloc(sizeof(int) * n);
	struct tsk *tsk;
	srand(time(NULL));
	
	int i;
	/* Unsorted Array */
	printf("Unsorted Array\n");
	for(i = 0; i < n; i++){
		array[i] = rand() % 100;
		printf("%d ",array[i]);
	}
	printf("\n");
	
	 pthread_t tid[m];  /* the thread identifier */
	 struct tsk tsklist[m];
	
	for(i=0; i<n;i++){
		tsk = &tsklist[i];
		tsk->id = i;
		tsk->low = i * slice;
		tsk->high = (i+1) * slice - 1;		
	}
	
	/*Create the threads*/
    for(int i=0;i<m;i++){
		tsk = &tsklist[i];		
        pthread_create(&tid[i],NULL,start,tsk);
    }
	
    /* wait for the thread to exit */
    for (int i=0; i<m; i++){
         pthread_join(tid[i], NULL);
    }	
	
	/* 1st Round Sorted Array */
	printf("1st Round Sorted Array \n");
	for(i = 0; i < n; i++){
		printf("%d ",array[i]);
	}
	printf("\n");
	
	mergeSplitArray(m,1);
	
	printf("Final Sorted Array \n");
	int temp;
	for(i = 0; i < n; i++){
		temp  = array[i];
		printf("%d ",array[i]);
		fprintf(fptr,"%d\n",temp);
	}
	printf("\n");

   fclose(fptr);


	return 0; 
} 


