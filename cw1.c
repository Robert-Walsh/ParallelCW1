#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define FAILURE 0
#define SUCCESS 1
#define ARRAY2DLOOKUP(SquareArray, Dim, X, Y) (SquareArray)[(Y)*(Dim) + (X)]

pthread_mutex_t printLock;
pthread_barrier_t barrierEndWhile;
pthread_barrier_t barrierEnterLoop;
pthread_barrier_t barrierEndForLoop;

int fUnderPrecision = 1;
int fSignalEnd = 0;

struct segment_LinkedListNode {
	double* segmentStart;
	int segmentLength;

	struct segment_LinkedListNode* next;
};

struct arg_struct {
	struct segment_LinkedListNode linkedListStart;

	double precision;
	int totalSquareArrayDimensions;
};

void printArray(double* squareArray, int dim);
void* arrayManipulator(void* args);
void deleteArgs(struct arg_struct* args);
void deleteArgsLinkedList(struct segment_LinkedListNode* args);

/*
*    This function organises the values pointed to by the squareArray 
*    into segments based on the number of threads and the number of calculatable values.
*    It creates arguments containing relevant information that each thread needs. 
*
*    \param    squareArray    A pointer to a square multidimensional array
*    \param    dim            Dimensions of the square multidimensional array. The width and height of the square               
*                             multidimensional array.
*    \param    precision      The solver will finish when the values per iteration in total change less than or equal to this   
*                             value
*    \param    nThreads       The number of threads used to solve the problem. 
*
*    \returns  0 when did not finish successfully and 1 when it does finish successfully.
*/
int solver(double* squareArray, int dim, double precision, int nThreads) {

    //calculate number of values that will be modified
    int rows = dim-2;
    int numberOfNumbers = rows*rows;
    
    pthread_t threads[nThreads];
    struct arg_struct arguments[nThreads]; 

    //Initialise our barrier used in arrayManipulator
    pthread_barrier_init(&barrierEndWhile, NULL, nThreads);
    pthread_barrier_init(&barrierEnterLoop, NULL, nThreads);
    pthread_barrier_init(&barrierEndForLoop, NULL, nThreads);

    //Initialise flags
    fUnderPrecision = 1;
    fSignalEnd = 0;
   
    int i=0;
    
    //Calculate number of values for each thread, and the remainder
    int totalData = (dim-2)*(dim-2);
    int sizePerThread = totalData/nThreads;
    int remainder = totalData%nThreads;
    int currentIt = dim+1;
    int rowEnd = dim*2 - 1;
    
    //Here we create the segments for each thread
    for(i=0; i<nThreads; ++i){
        if ((currentIt + 1) % dim == 1) {
			++currentIt;
		} else if ((currentIt + 1) % dim == 0) {
			currentIt += 2;
		}
        arguments[i].precision = precision;
        arguments[i].totalSquareArrayDimensions = dim;
		int currentSegment = 0;
		if (i == nThreads - 1) { sizePerThread += remainder; }

		struct segment_LinkedListNode* linkedListIterator = &arguments[i].linkedListStart;
		linkedListIterator->next = NULL;
		linkedListIterator->segmentStart = squareArray + currentIt;

		int sizePerSegment = sizePerThread;
		int sizeCountdown = sizePerThread;
		if (currentIt >= rowEnd) { rowEnd += dim; } // We've reached the rowEnd, set rowEnd as new rowEnd
		while (1) {
			if ((currentIt + 1) % dim == 1) {
				++currentIt;
			} else if (sizeCountdown == 0 || currentIt >= rowEnd) {

				linkedListIterator->segmentLength = sizePerSegment - sizeCountdown;
				sizePerSegment -= linkedListIterator->segmentLength;
				++currentSegment;

				if (currentIt >= rowEnd) { rowEnd += dim; ++currentIt; }

				if (sizeCountdown == 0 || currentIt >= (dim*(dim - 1)) - 1) {
					break;
				}

				linkedListIterator->next = (struct segment_LinkedListNode*)malloc(sizeof(struct segment_LinkedListNode));
				linkedListIterator = linkedListIterator->next;
				linkedListIterator->next = NULL;

				if ((currentIt + 1) % dim == 0) {
					++currentIt;
				}

				linkedListIterator->segmentStart = squareArray + (++currentIt);
			}
			else {
				--sizeCountdown;
				++currentIt;
			}
		}

        pthread_create(&threads[i], NULL, arrayManipulator, (void *)&arguments[i]);
    }

    //join threads
    int j;
    for (j = 0; j < nThreads; j++){
        pthread_join(threads[j], NULL);
        //deleteArgs(&arguments[i]);
    }

    //reset state of fSignalEnd
    fSignalEnd = 0; 
    
    return 0;
}


void* arrayManipulator(void* arguments){
    fUnderPrecision = 1;

    struct arg_struct *args = arguments;

	struct segment_LinkedListNode* linkedListIterator = &args->linkedListStart;

    int dim = args->totalSquareArrayDimensions;
    double precision = args->precision;

    int j, iteration;
    iteration = 0;
    do{
        fUnderPrecision = 1;

        pthread_barrier_wait (&barrierEnterLoop);
        for(linkedListIterator = &args->linkedListStart; linkedListIterator!=NULL;linkedListIterator = linkedListIterator->next ){
            for(j = 0; j < linkedListIterator->segmentLength; ++j){
                double prevIteration = (linkedListIterator->segmentStart)[j];
                (linkedListIterator->segmentStart)[j] = ( (linkedListIterator->segmentStart)[j-dim]
                                                        + (linkedListIterator->segmentStart)[j-1]
                                                        + (linkedListIterator->segmentStart)[j+1]
                                                        + (linkedListIterator->segmentStart)[j+dim]
                                                      ) / 4.0;

                if(fUnderPrecision == 1 && fabs((linkedListIterator->segmentStart)[j]-prevIteration) > precision){
                    fUnderPrecision = 0;
                }
            }
        }

        pthread_barrier_wait (&barrierEndForLoop);
        if(fSignalEnd == 0 && fUnderPrecision == 1){
            fSignalEnd = 1;
        }
        pthread_barrier_wait (&barrierEndWhile);

        ++iteration;
    } while(fSignalEnd == 0);
}


int main(int argc, char *argv[]) {

    static const int dimension = 5;
    static const int numberOfThreads = 2;
    static const double precision = 0.001; //the precision to average the numbers to
    
    /// Square multidimensional array in row-column format. E.g. data[1][2] looks up x=2, y=1
    double data[5][5] =   {
                            {1.0,2.0,0.5,3.0,2.2},
                            {1.0,0.1,0.1,6.0,5.0},
                            {1.0,0.7,0.9,2.9,2.0},
                            {1.0,2.5,0.5,1.0,7.0},
                            {1.3,0.1,0.7,2.0,2.1},
                        };

    if(!dimension>2){
        printf("Array size not large enough");
        return 0;
    }

    pthread_mutex_init(&printLock, NULL);

    printf("\nBefore Solver:\n");
    printArray(&data[0][0], dimension);
    printf("\n\n");

    solver(&data[0][0], dimension, precision, numberOfThreads);

    printf("After Solver:\n");
    printArray(&data[0][0], dimension);
    printf("\n\n");

    pthread_mutex_destroy(&printLock);
    return 0;   
}

void deleteArgs(struct arg_struct* args) {
	deleteArgsLinkedList(&args->linkedListStart);
	args->precision = 0.0;
	args->totalSquareArrayDimensions = 0;
}


void deleteArgsLinkedList(struct segment_LinkedListNode* args) {
	if (args != NULL) {

		if (args->next != NULL) {
			deleteArgsLinkedList(args->next);
		}
		args->next = NULL;
		args->segmentStart = NULL;
		args->segmentLength = 0;
	}
}


void printArray(double* squareArray, int dim){
    pthread_mutex_lock(&printLock);

    int y,x;
    for(y = 0; y < dim; ++y){
        for(x = 0; x < dim; ++x){
            printf("%lf\t", ARRAY2DLOOKUP(squareArray, dim, x, y));
        }
        printf("\n");
    }
    printf("\n");

    pthread_mutex_unlock(&printLock);
}
