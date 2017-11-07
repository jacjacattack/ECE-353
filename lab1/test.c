/*

Matthew Bolognese
Jacqueline Lagasse
ECE 353 Lab 1
2016-10-03

Notes for user:
1.
Please enter the key file, message file, and output file parameters in this order when using the code, or else the message will not be encrypted/decrypted properly.
2.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


#define KLENGTH 256


// This algorithm generates characters for the keystream
char generateKeyByte(char s[]){
    static int x=0;
    static int y=0;
    char t;
    char tmp;
    char keyByte;
    x=(x+1)%KLENGTH;
    y=(y+s[x])%KLENGTH;
    tmp = s[x];
    s[x] = s[y];
    s[y] = tmp;
    t = ((s[x]+s[y])%KLENGTH);
    keyByte = s[t];
    return keyByte;
}  // end of generateKeyByte


int main(int argc, char *argv[]){

    // File global variables
    FILE *keyf;
    FILE *messagef;
    FILE *outputf;

    // char global variables
    char keyStream;
    char messageChar;

    // char array global variables
    char key[KLENGTH];
    char S[KLENGTH];
    char T[KLENGTH];
    
    // checks if the program has been passed the proper amount of files
    if(argc==4){
   	 
   	 // opens each file, keyfile and message file is read, output is write
   	 keyf = fopen(argv[1], "r");
   	 messagef = fopen(argv[2], "r");    
   	 outputf = fopen(argv[3], "w");

   	 // asserts that the given files exist and are not null before proceeding
   	 assert(keyf != NULL);
   	 assert(messagef != NULL);
   	 assert(outputf != NULL);

   	 // Step 1: Fill key array with keyfile values
   	 char temp;
   	 int count=0;
   	 // reads keyfile until end of file, or key array is full
   	 while ( ((temp = fgetc(keyf))!=EOF) && (count<KLENGTH) ) {
   		 key[count]=temp;
   		 count++;
   	 }
/* THIS DOES IN FACT HELP MAKE MORE ACCURATE OUTPUT FOR SMALL KEYFILES
   	 // if keyfile is less than 256 characters, this fills up the remainder of indexes in key array
   	 // this ensures that there are no null values in the key array   	 
   	 while(count<KLENGTH){
   		 key[count]='a'+count;
   		 count++;   	 
   	 }
*/
   	 // Step 2: Do initialization and permutation of arrays for keystring algorithm
   	 int j=0;
   	 int i;
   	 for(i=0; i<KLENGTH; i++){
   		 S[i] = i;
   		 T[i] = key[i%KLENGTH];
   		 j = ((j+S[i]+T[i])%KLENGTH);
   		 temp = S[i];
   		 S[i] = S[j];
   		 S[j] = temp;
   	 }

   	 // Step 3: Generate Keystream
   	 // reads message character by character until the end of file
   	 while((messageChar = fgetc(messagef)) != EOF){
   		 keyStream = generateKeyByte(key);
   		 // Step 4: Compute XOR
   		 messageChar^=keyStream;
   		 // Step 5: print XOR result to output file
   		 fputc(messageChar,outputf);
   	 }

   	 // close files
   	 fclose(keyf);
   	 fclose(messagef);
   	 fclose(outputf);

    }
    // if more or less than 3 files have been passed, the program will not execute fully and will throw the below exception
    else{printf("Insifficient number of files entered.\nEncryption cannot be performed.\n");}

    // return integer to satisty main method and terminate program
    return 0;

} // end of main
