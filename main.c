#include <stdio.h>
#include <math.h>
#include <pthread.h>
 
#define FAILURE 0
#define SUCCESS 1
#define DIMENSIONS 5

#define ARRAY2DLOOKUP(SquareArray, Dim, X, Y) (SquareArray)[(Y)*(Dim) + (X)]
 
 
void* arrayManipulator(void* args);
 
/*
*    \param    squareArray    A pointer to a square multidimensional array
*    \param    dim            Dimensions of the square multidimensional array. The width and height of the square               
*                             multidimensional array.
*
*/
void printArray(double* squareArray, int dim);
 
/*
*    This function modifies the data pointed to by the squareArray in iterations
*    where each value is the average of its four neighbours.
*
*    \param    squareArray    A pointer to a square multidimensional array
*    \param    dim            Dimensions of the square multidimensional array. The width and height of the square               
*                             multidimensional array.
*    \param    precision      The solver will finish when the values per iteration in total change less than or equal to this   
*                             value
*
*    \returns  0 when did not finish successfully and 1 when it does finish successfully.
*/
int solver(double* squareArray, int dim, double precision /*, int nThreads*/) {
       
    int x,y;
    double workingTemp;
    double largestChange;
    double ABSed;
 
    do{        
        largestChange = 0.0;
        for(y=1; y<dim-1; ++y){
            for(x=1; x<dim-1; ++x){
 
                workingTemp = (  ARRAY2DLOOKUP(squareArray,dim,x+1,y)
                                    + ARRAY2DLOOKUP(squareArray,dim,x-1,y)
                                    + ARRAY2DLOOKUP(squareArray,dim,x,y+1)
                                    + ARRAY2DLOOKUP(squareArray,dim,x,y-1)
                                   )/4.0;
               
                workingTemp = workingTemp-ARRAY2DLOOKUP(squareArray,dim,x,y);
                ABSed = fabs(workingTemp);
                if(ABSed>largestChange){
                    largestChange = ABSed;
                }
               
                ARRAY2DLOOKUP(squareArray,dim,x,y) += workingTemp;
            }
        }
       
    } while(largestChange<precision);
 
    return SUCCESS;
}
struct arg_struct {
    double* squareArray;
    int dim;
    double precision;
    
    int success;
};

int parallelSolver(double* squareArray, int dim, double precision, int nThreads) {
    
    printf("Entered parallel solver \n");
    
    struct arg_struct args = {
        squareArray, 
        dim,
        precision,
        0
    };
    
    pthread_t thread;
    
    pthread_create(&thread, NULL, arrayManipulator, (void *)&args);
    
    pthread_join(thread, NULL);
 
    return SUCCESS;
}



void* arrayManipulator(void* args){
    printf("Entered arrayManipulator \n");
    int x,y;
    double workingTemp;
    double largestChange;
    double ABSed;
    
    struct arg_struct* arguments =  ((struct arg_struct*)args);
    double* squareArray = arguments->squareArray;
    int dim = arguments->dim;
    double precision = arguments->precision;
    int counter = 0;
    do{     
        
           
        largestChange = 0.0;
        for(y=1; y<dim-1; ++y){
            for(x=1; x<dim-1; ++x){
 
                workingTemp = (  ARRAY2DLOOKUP(squareArray,dim,x+1,y)
                                    + ARRAY2DLOOKUP(squareArray,dim,x-1,y)
                                    + ARRAY2DLOOKUP(squareArray,dim,x,y+1)
                                    + ARRAY2DLOOKUP(squareArray,dim,x,y-1)
                                   )/4.0;
               
                workingTemp = workingTemp-ARRAY2DLOOKUP(squareArray,dim,x,y);
                ABSed = fabs(workingTemp);
                if(ABSed>largestChange){
                    largestChange = ABSed;
                }
               
                ARRAY2DLOOKUP(squareArray,dim,x,y) += workingTemp;
            }
        }
        ++counter;
       
    } while(largestChange>precision);
        
    printf("Number of iterations: %d\n\n", counter);
    
    arguments->success = 1;
    
    return NULL;
}
 
int main(int argc, char *argv[]) {    
    
    /// Square multidimensional array in row-column format. E.g. data[1][2] looks up x=2, y=1  
    double data[DIMENSIONS][DIMENSIONS] =   {
                                                {1.0,2.0,0.5,3.0,2.2},
                                                {1.0,0.1,0.1,6.0,5.0},
                                                {1.0,0.7,0.9,2.9,2.0},
                                                {1.0,2.5,0.5,1.0,7.0},
                                                {1.3,0.1,0.7,2.0,2.1},
                                            };
    printf("\n\n\n\n");
    printf("%lf\n", data[0][0]);
    printf("%lf\n", data[0][4]);
    printf("%lf\n", data[4][0]);
    printf("%lf\n", data[4][4]);
   
   
    printf("\nBefore Solver:\n");
    printArray(&data[0][0], DIMENSIONS);
    printf("\n\n");
   
    if(!parallelSolver(&data[0][0], DIMENSIONS, 0.001, 1)){
        printf("Failed to solve :(");
        return -1; /// Failure
    }
 
    printf("After Solver:\n");
    printArray(&data[0][0], DIMENSIONS);
    printf("\n\n");
   
    return 0;    /// Success
}
 
void printArray(double* squareArray, int dim){
    int y,x;
    for(y = 0; y < dim; ++y){
        for(x = 0; x < dim; ++x){
            printf("%lf\t", ARRAY2DLOOKUP(squareArray,dim, x, y));
        }
        printf("\n");
    }
}