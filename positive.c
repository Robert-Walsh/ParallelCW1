#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define FAILURE 0
#define SUCCESS 1

#define ARRAYLENGTH 100
#define DEREFERENCE(x) *(x)

#define NUMBER_OF_PROCESSORS 4

void printArray(int* array, int length);

int countPositive(int* array, int length);

struct countPositive_ParallelWrapperArgs{
    //args
    int* array;
    int length;
    
    //result
    int result;    
};

void* countPositive_ParallelWrapper(void* args){
    if(args==NULL){
        pthread_exit(0);
    } else {
        struct countPositive_ParallelWrapperArgs* castedArgs = (struct countPositive_ParallelWrapperArgs*)args;
        (*castedArgs).result = countPositive((*castedArgs).array,(*castedArgs).length);
        pthread_exit(0);
    }
}

int countPositiveParallel(int* array, int length, int nThreads){
    if(nThreads<0){return -1;}
    
    if(nThreads==1){
        return countPositive(array,length);
    }else{
        pthread_t*  threads = malloc(sizeof(pthread_t)*nThreads);
        
        struct countPositive_ParallelWrapperArgs* counterArgs = malloc(sizeof(struct countPositive_ParallelWrapperArgs)*nThreads);
        int i;
        
        //Jack's way
        int numberPerThread = length/nThreads;
        
        for(i=0; i < nThreads;++i){
            
            counterArgs[i].result = -1;
            
            /* // Rob's shit way
             if(i==nThreads-1){
                counterArgs[i].array = array+(((length/nThreads)*i)+(length%nThreads);
                counterArgs[i].length = (length/nThreads)+(length%nThreads);
            } else {
                counterArgs[i].array = array+((length/nThreads)*i);
                counterArgs[i].length = (length/nThreads);
            }
            */
            
            /// Jack's awesome way
            counterArgs[i].array = array;
            counterArgs[i].length = (i!=nThreads-1 ? numberPerThread : (numberPerThread + (length%numberPerThread)) );
            
            array += numberPerThread;
            //// Jack's awesome way end
                        
            pthread_create(&threads[i], NULL, &countPositive_ParallelWrapper, (void*)&counterArgs[i]);
        }
        
        for(i = 0; i < nThreads; ++i){
            pthread_join(threads[i], NULL);
        }
        
        int totalCount = 0;
        for(i = 0; i < nThreads; ++i){
            totalCount += counterArgs[i].result;
        }
        
        free(counterArgs);
        free(threads);
        
        return totalCount;
    }
}


int main(){
    time_t t;
    srand((unsigned)time(&t));
    
    int myArray[ARRAYLENGTH];
    int i;
    
    for(i=0;i<ARRAYLENGTH;++i){
        myArray[i] = (rand()%200)-100;        
    }
    
    printArray(&myArray[0], ARRAYLENGTH);
    
    int countSeq, countPar;
    
    struct timeval t0;
    struct timeval t1;
    long elapsed;

    gettimeofday(&t0, 0);
    countSeq = countPositive(&myArray[0], ARRAYLENGTH);
    gettimeofday(&t1, 0);

    elapsed = ((t1.tv_sec - t0.tv_sec)*1000000L +t1.tv_usec) - t0.tv_usec;
  
    printf("Positive Seq count: %d. In time: %ld\n",countSeq,elapsed);
    
    gettimeofday(&t0, 0);
    countPar = countPositiveParallel(&myArray[0], ARRAYLENGTH,4);
    gettimeofday(&t1, 0);
    
    elapsed = ((t1.tv_sec - t0.tv_sec)*1000000L +t1.tv_usec) - t0.tv_usec;
    
    printf("Positive Parallel count: %d. In time: %ld\n",countPar,elapsed);
    
    
}

void printArray(int* array, int length){
    
    int i;
    for(i=0;i<length;++i){
        printf("%d, ", array[i]);
    }
    printf("\n");
}



int countPositive(int* array, int length){
    int counter = 0;
    int i;
    for(i=0; i < length; ++i){
        if(array[i]>=0){
            ++counter;
        }
    }
    return counter;
}