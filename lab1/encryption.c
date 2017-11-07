/*
 
 Matthew Bolognese
 Jacqueline Lagasse
 ECE 353 Lab 1
 2016-10-03
 
 Note for user:
 Please enter the key file, message file, and output file parameters in this order when using the code, or else the message will not be encrypted/decrypted properly.
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


#define KLENGTH 256 // maximum number of characters used in keyfile


void fillKeyArray(char k[], FILE *kf){  // this fills the key array with values from the key file
    char temp;                          // temporary character placeholder
    int count=0;                        // integer used for counting characters put in key array
    // initialize key array from key file
    while ( ((temp = fgetc(kf))!=EOF) && (count<KLENGTH)) { // reads keyfile until end of file, or until key array is full
        k[count]=temp;                  // sets key[count] to the character retrieved by the file
        count++;                        // increments count by one
    }
    // fills up the remainder of indexes in key array if keyfile contains less than 256 characters
    // this ensures that there are no null values in the key array, which can cause the decryption to fail
    while(count<KLENGTH){               // runs until key array is filled
        k[count]=(count%KLENGTH);       // sets key[count] to a random value
        count++;                        // increments count by one
    }
} // end of fillKeyArray


void permutateArrays(char s[], char t[], char k[]){ // this generates swaps to prepare for generating keystream
    // declare variables
    int j=0;
    int i;
    char temp;
    for(i=0; i<KLENGTH; i++){
        s[i] = i;                       // initialize S array
        t[i] = k[i%KLENGTH];            // initialize T array
    }
    // swaps for each key index
    for(i=0; i<KLENGTH; i++){
        j = ((j+s[i]+t[i])%KLENGTH);    // compute j
        // swap s[i] and s[j]
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
    }
} // end of permutateArrays


char generateKeyByte(char s[]){ // this algorithm generates characters for the keystream
    // declare variables
    static int x=0;
    static int y=0;
    char t;
    char tmp;
    char keyByte;
    // compute x and y values
    x=((x+1)%KLENGTH);
    y=((y+s[x])%KLENGTH);
    // swap s[x] and s[y]
    tmp = s[x];
    s[x] = s[y];
    s[y] = tmp;
    // return char value at s[t]
    t = ((s[x]+s[y])%KLENGTH);
    return keyByte = s[t];
}  // end of generateKeyByte


int main(int argc, char *argv[]){ // main method begins
    
    FILE *keyf;         // pointer for the keyfile file
    FILE *messagef;     // pointer for the message file
    FILE *outputf;      // pointer for the output file
    
    char keyStream;     // stores a single character value from the keyStream
    int messageChar;    // stores a single character value from the message file
    int outputChar;     // stores a single character value from the output file
    
    char key[KLENGTH];  // char array to store values of keyfile
    char S[KLENGTH];    // char array to store values of S, used in encryption algorithm
    char T[KLENGTH];    // char array to store values of T, used in encryption algorithms
    
    // checks if the program has been passed the proper amount of files
    if(argc==4){ // if argc = 4, then three parameter files have been passed, and encryption continues
        
        keyf = fopen(argv[1], "r");     // sets the first parameter file as keyfile
        messagef = fopen(argv[2], "r"); // sets the second parameter file as message file
        outputf = fopen(argv[3], "w");  // sets the third parameter file as output file
        
        assert(keyf != NULL);           // asserts that keyfile is not null
        assert(messagef != NULL);       // asserts that message file is not null
        assert(outputf != NULL);        // asserts that output file is not null
        
        fillKeyArray(key,keyf); // Step 1: Fill key array with keyfile values
        
        permutateArrays(S, T, key); // Step 2: Do initialization and permutation of arrays for keystring algorithm
        
        while((messageChar = fgetc(messagef)) != EOF){ // steps through and retrieves each char of the message file until end of file
            
            keyStream = generateKeyByte(key); // Step 3: Generate Keystream

            outputChar=messageChar^keyStream; // Step 4: Compute XOR
            
            fputc(outputChar,outputf); // Step 5: print XOR result to output file
        }
        
        fclose(keyf);       // closes keyfile
        fclose(messagef);   // closes message file
        fclose(outputf);    // closes output file
        
    }
    
    // if more or less than 3 files have been passed, the program will not execute fully and will throw the below exception
    else{printf("Insifficient number of files entered.\nEncryption cannot be performed.\n");}
    
    return 0; // return integer to satisty main method and terminate program
    
} // end of main

