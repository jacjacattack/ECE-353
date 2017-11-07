/*

Code Developed by:
Dennis Donoghue (Spire ID 28707934)
Jacqueline Lagasse (Spire ID 28645176)

ECE 353 Lab 3
2016-11-14

*/


                                                // include statements for required libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

                                                // global variables
#define BOLTZMAN 0.00008617                     // boltzman constant k, unit is eV/degKelvin
#define Ea 0.8                                  // activation energy Ea, unit is eV

double *capArray;                               // declare array for thermal capacitance 
double *resArray;                               // declare array for thermal resistance
double *powerArray;                             // declare array for power
double *kArray;                                 // declare array for k valuess
double *temperatureArray;                       // declare array for final temperature values
double *ageArray;                               // delcare array for ages of cores

int coreCount=0;                                // initialized core count.
int intTime=1;                                  // initializes the number of samples given in powerFile and required in outputFile

double ambientTemp=300;                         // if ambient temp is not given, it is assumed to be 300
double h = 0.005;                               // define step size

                                                // forward initialiation of subroutines
double rk();                                    // Runge-Kutta algorithm, used to solve differential equation for T                               
double f();                                     // f computes the derivative of T, dT/dt
double sum();                                   // sum computes the summattion of the difference in core temp over resistance
void initTemp();                                // init temp initializes the temperature of all cores to ambient temp before computations



////////////////////////////////////////


 
double rk(int tao){ // function to solve a system of n differential equations, returns a double 
    int core = 0;
    for(core=0; core<4; core++){                // compute all k1 values first, must be done seperately becuase k2 depends on k1
        *(kArray + core) = h * f( *(powerArray+(tao*coreCount)+core), *(capArray+core), sum(core,0) ); // computes and stores k1 for each core
    }
    for(core=0; core<4; core++){                // compute all k2 values first, must be done seperately becuase k3 depends on k3    
        *(kArray + 4 + core) = h * f( *(powerArray+(tao*coreCount)+core), *(capArray+core), sum(core,1) ); // computes and stores k2 for each core
    }
    for(core=0; core<4; core++){                // compute all k3 values first, must be done seperately becuase k4 depends on k3   
        *(kArray + 8 + core) = h * f( *(powerArray+(tao*coreCount)+core), *(capArray+core), sum(core,3) ); // computes and stores k3 for each core
    }
    for(core=0; core<4; core++){                // compute all k4 values last    
        *(kArray + 12 + core) = h * f( *(powerArray+(tao*coreCount)+core), *(capArray+core), sum(core,4) ); // computes and stores k4 for each core
    }
    for(core=0; core<4; core++){                // rewrite temp values in temperature array
        *(temperatureArray+core) = *(temperatureArray+core) + ((*(kArray+core) + (*(kArray+4+core)*2) + (*(kArray+8+core)*2) + *(kArray+12+core) ) / 6);
    }
    return 0;
} // end rk



////////////////////////////////////////



double f(double omega, double cap, double sum){ // computes value for dT/dt with specific values
    return ((omega-sum)/cap);                   // returns dT/dt
}                                               // end f



//////////////////////////////////////////////////



double sum(int coreIndex, int kConstant){       // computes summation of temperatures
    int cnt = 0;
    double sum = 0;
    double numerator, denominator;
    if(kConstant==0){                           // for computing the first k constant k1
        for(cnt=0; cnt<coreCount; cnt++){           // needs to count up to # cores + 1 to include ambient temperature
            if(cnt != coreIndex){                   // skip indexes that match coreIndex, redundant
                // sum = previous sum + [T@coreIndex+k - T@cnt-k] / RcoreIndex,cnt
                sum = sum + ( ( *(temperatureArray + coreIndex) - *(temperatureArray + cnt) ) / *(resArray + (coreIndex*coreCount) + cnt) );
            }
        }
    }
    else if(kConstant==1 || kConstant==2){
        for(cnt=0; cnt<coreCount; cnt++){           // needs to count up to # cores + 1 to include ambient temperature
            if(cnt != coreIndex){                   // skip indexes that match coreIndex, redundant
                // sum = previous sum + [T@coreIndex+k/2 - T@cnt-k/2] / RcoreIndex,cnt
                numerator = *(temperatureArray+coreIndex) + (*(kArray+((kConstant-1)*coreCount)+coreIndex)/2)  - (*(temperatureArray+cnt)+(*(kArray+((kConstant-1)*coreCount)+cnt))/2);
                denominator = *(resArray+(coreIndex*coreCount)+cnt);
                sum = sum + (numerator/denominator);
//                sum = sum + ( (*(temperatureArray+coreIndex)+(*(kArray+((kConstant-1)*coreCount)+coreIndex)/2) ) - (*(temperatureArray+cnt)+(*(kArray+((kConstant-1)*coreCount)+cnt))/2 ) ) / *(resArray+(coreIndex*coreCount)+cnt) );
            }
        } 
    }
    else{
        for(cnt=0; cnt<coreCount; cnt++){           // needs to count up to # cores + 1 to include ambient temperature
            if(cnt != coreIndex){                   // skip indexes that match coreIndex, redundant
                // sum = previous sum + [T@coreIndex+k/2 - T@cnt-k/2] / RcoreIndex,cnt
                numerator = *(temperatureArray+coreIndex) + *(kArray+((kConstant-1)*coreCount)+coreIndex)  - *(temperatureArray+cnt)+(*(kArray+((kConstant-1)*coreCount)+cnt));
                denominator = *(resArray+(coreIndex*coreCount)+cnt);
                sum = sum + (numerator/denominator);
//                sum = sum + ( ( (*(temperatureArray+coreIndex) + (*(kArray+((kConstant-1)*coreCount)+coreIndex) ) - (*(temperatureArray+cnt) + (*(kArray+((kConstant-1)*coreCount)+cnt)) ) ) / *(resArray+(coreIndex*coreCount)+cnt) );
            }
        }         
    }
} // end sum



//////////////////////////////////////////////////




void initTemp(double ambTemp, int count){       // sets intial temp of each core to ambient temp
    int y = 0;                                  // begins loop at zero    
    for(y = 0; y<count; y++){                   // increments to each core
        *(temperatureArray + y) = ambTemp;      // each core = ambient temp
    }
}                                               // end initTemp



//////////////////////////////////////////////////



int main (int argc, char * argv[]){             // main method
    
    int i;                                      // for loop incrementer
            
////////////////////////////////////////////////// Data Input

    if(argc==4 || argc==5){                     // checks if the program has been passed the proper amount of files
                                                // if argc = 4, then three parameter files have been passed, and encryption continues
        
        FILE *paramFile;                        // pointer for the paramter file
        FILE *powerFile;                        // pointer for the power file
        FILE *ambientFile;                      // pointer for the ambient file
        FILE *outputFile;                       // pointer for the output file

        paramFile = fopen(argv[1], "r");        // sets the first parameter file as paramFile
        powerFile = fopen(argv[2], "r");        // sets the second parameter file as powerFile
        assert(paramFile != NULL);              // asserts that paramFile is not null
        assert(powerFile != NULL);              // asserts that powerFile is not null
 

        if(argc==4){                            // if we are NOT using optional ambient parameter
        
            outputFile = fopen(argv[3], "w");   // sets the third parameter file as outputFile
            assert(outputFile != NULL);         // asserts that outputFile is not null
            
        }else{                                  // if we are ARE using optional ambient parameter
        
            ambientFile = fopen(argv[3], "r");  // sets the third parameter file as ambientFile
            outputFile = fopen(argv[4], "r");   // sets the fourth parameter file as outputFile
            assert(ambientFile != NULL);        // asserts that ambientFile is not null
            assert(outputFile != NULL);         // asserts that outputFile is not null
            
            fscanf(ambientFile, "%lf", &ambientTemp); // reads in ambientFile to ambientTemp
            fclose(ambientFile);                // rewinds ambient file
        }
                                                // read parameters for capacitors, resistors, power from input file
        
        int linecount=0;                        // line count variable NOTE!!!!!!!!!!!! I CHANGED THIS FROM =1 to =0
        char c;                                 // temp character c
        while((c = getc(paramFile)) != EOF){    // while loop for finding # of cores 
            if(c=='\n'){                        // increment line count for newline
                linecount++;
            }
        }
        coreCount=linecount-1;                  // # of cores is the linecount of paramFile-1;
        printf("coreCount: %i \n",coreCount);
        rewind(paramFile);                      // rewinds paramfile for reading in data.
        
        while((c = getc(powerFile)) != EOF){    // while loop for finding # of samples in powerFile
            if(c=='\n'){                        // increment line count for newline
                intTime++;
            }
        }
        rewind(powerFile);                      // rewinds power file for reading in data.
        printf("initTime: %i \n",intTime);
        
        capArray=(double*)calloc((coreCount), sizeof(double)); // callocates space for capArray given the coreCount
        for(i=0;i<coreCount;i++){
            fscanf(paramFile, "%lf", &capArray[i]); // populates core capacitance array
//            printf("%lf\n", capArray[i]);       // prints array values to stdout
        }
            
        i=0;                                    // resets incrementer int
        resArray=(double*)calloc((coreCount*coreCount), sizeof(double)); // callocates space for resArray given the coreCount
        while(fscanf(paramFile,"%lf", &resArray[i]) != EOF){
//            printf("%lf\n", resArray[i]);       // prints array values to stdout
            i++;                                //populates intercore resistance array
        }
        i=0;                                    // resets incrementer int
        
        powerArray=(double*)calloc(((coreCount)*intTime), sizeof(double)); // callocates space for powerArray given the coreCount
        while(fscanf(powerFile,"%lf", &powerArray[i]) != EOF){
//            printf("%lf\n", powerArray[i]);     // prints array values to stdout
            i++;                                //populates power sample array.
        }
        
        
        
////////////////////////////////////////////////// Data Manipulation
                         


        double h = 0.005;                       // initialize step value

        double omega = 0;                       // current temperature T(t+h)
        double capacitance = 0;                 // previous temperature T(t)
        
        kArray = (double *) calloc(16, sizeof(double)); // allocate memory for k array 
        temperatureArray = (double *)calloc(coreCount, sizeof(double)); // allocate memory for temperature array
        
        initTemp(ambientTemp, coreCount);       // initialize temperature array with ambient values
        
        int tao = 0;                          // count for power loop
        for(tao=0; tao<intTime; tao++){   // repeat process for each power value
            
            int repeat = 0;
            while(repeat<10000){
              rk(tao); // run Runge Kutta algroithm to solve differential equation
              repeat++;
            }
            
            
            
////////////////////////////////////////////////// Data Output

                                                // print data to outputFile.txt
            printf("Time: %i; T1: %f; T2: %f; T3: %f; T4: %f\n",tao,*(temperatureArray),*(temperatureArray+1),*(temperatureArray+2),*(temperatureArray+3));
//            fprintf(outputFile,);             // print to output file here: time, c1 temp, c1 age, c2 temp, c2 age... all on same line
            
        }                                       // end power loop

                                        

////////////////////////////////////////////////// Cleaning Up

/* I would like to free all allocated memory, however doing so causes an error
free(): invalid next size (fast): 0x0000000001d82970 ***

                                                // free pointer-allocated data
        free(temperatureArray);
        free(kArray);
        free(capArray);
        free(resArray);
        free(powerArray);
*/        
        fclose(paramFile);                      // closes paramFile
        fclose(powerFile);                      // closes powerFile
        fclose(outputFile);                     // closes outputFile
    
    }                                           // end of main if statement
    
    
    
    // if more or less than 3 files have been passed, the program will not execute fully and will throw the below exception
    else{printf("Insifficient number of files entered.\nFunction cannot be performed.\n");}

    return 0;                                   // return integer to satisfy main method and terminate program

}                                               // end main

