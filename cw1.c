#include <stdio.h>
#include <math.h>
#include <pthread.h>
 
#define FAILURE 0
#define SUCCESS 1
#define DIMENSION 5 //the dimension of the square 
#define PRECISION 0.001 //the precision to average the numbers to
#define ARRAY2DLOOKUP(SquareArray, Dim, X, Y) (SquareArray)[(Y)*(Dim) + (X)]

int NUM_OF_THREADS = 5;


pthread_mutex_t lock; 

struct arg_struct {
    int CALCULATIONS_PER_THREAD;
    double *ARRAY_START; 
};

void printArray(double* squareArray, int dim);

void* arrayManipulator(void* args);

int solver(double* squareArray, int dim, double precision, int nThreads) {

    printf("Entered solver \n\n");
    
    int rows = DIMENSION-2; 
    int numberOfNumbers = rows*rows;
    
    //set number of threads as the max amount of rows we can work on at any time
    if(NUM_OF_THREADS>((DIMENSION-1)/2)){
        NUM_OF_THREADS = (DIMENSION-1)/2;
    }
    
    pthread_t threads[NUM_OF_THREADS];   
    struct arg_struct arguments[NUM_OF_THREADS]; //make an array of structs
    
    printf("%d", NUM_OF_THREADS);
    printf("hello");   
   
    int i=0;
    for(i=0; i<NUM_OF_THREADS; ++i){
        arguments[i].ARRAY_START = &squareArray[(i*2)*DIMENSION];
        
        pthread_create(&(threads[i]), NULL, arrayManipulator, (void *)&arguments[i]);
    }
     
    //join threads
    int j;
    for (j = 0; j < NUM_OF_THREADS; j++)
    {
        pthread_join(threads[j], NULL);
    }    
}

void* arrayManipulator(void* arguments){
    
    struct arg_struct *args = arguments;
    double *array_start = args->ARRAY_START;
    
    printf("%p\n", (void *) &array_start);
    


    printf("HEEEEEEEEEEEEEEELLLLLLLLLLLOOOOOOOOOOO WOOOOOOOOOOOOOORLLLLLLLLLLLLLLDDDDDDDD");
}


int main(int argc, char *argv[]) {    
    
    /// Square multidimensional array in row-column format. E.g. data[1][2] looks up x=2, y=1  
    double data[DIMENSION][DIMENSION] =   {
                                            {1.0,2.0,0.5,3.0,2.2},
                                            {1.0,0.1,0.1,6.0,5.0},
                                            {1.0,0.7,0.9,2.9,2.0},
                                            {1.0,2.5,0.5,1.0,7.0},
                                            {1.3,0.1,0.7,2.0,2.1},
                                        };
                                    
    
    
    if(!DIMENSION>2){
        printf("Array size not large enough");
        return 0;
    }
    
    
    pthread_mutex_init(&lock, NULL); 
    
    printf("\nBefore Solver:\n");
    printArray(&data[0][0], DIMENSION);
    printf("\n\n");
    
    
    solver(&data[0][0], DIMENSION, PRECISION, NUM_OF_THREADS);
        
    
   
    // if(!){
    //     printf("Failed to solve");
    //     return -1; /// Failure
    // }
 
    // int a = (5-1)/2;
    // int b = (6-1)/2;
    // int c = (7-1)/2;
    // int d = (8-1)/2;
    // int e = (9-1)/2;
    // int f = (10-1)/2;
    // int g = (11-1)/2;
    // int h = (12-1)/2;
    
    // printf("%d\n", a);
    // printf("%d\n", b);
    // printf("%d\n", c);
    // printf("%d\n", d);
    // printf("%d\n", e);
    // printf("%d\n", f);
    // printf("%d\n", g);
    // printf("%d\n", h);
     
    printf("After Solver:\n");
    printArray(&data[0][0], DIMENSION);
    printf("\n\n");
    
    pthread_mutex_destroy(&lock);
    return 0;    /// Success
}

void printArray(double* squareArray, int dim){
    int y,x;
    for(y = 0; y < dim; ++y){
        for(x = 0; x < dim; ++x){
            printf("%lf\t", ARRAY2DLOOKUP(squareArray, dim, x, y));
        }
        printf("\n");
    }
}