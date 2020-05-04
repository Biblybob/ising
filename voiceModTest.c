#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 80000 // about 10 seconds of buffer (@ 8K samples/sec)
#define SAMPFREQ 8000 // sampling frequency

/*
program simulates and tests the changing of 'pitch' of a signal by changing the frequency
datapoints is placed onto an output
*/

// function prototype
void record(double buffer[]);
void play (double buffer []);
double outFeq();

// global buffer pointer
int bufferIndex;


int main (){
	double BUFF[BUFSIZE];
	record(BUFF);
	play(BUFF);
}

// samples the input at defined sampled rate
void record (double buffer[]){ 
	int T =  0;
	bufferIndex = 0;
	printf("Input waveform sample frequency: 8000 Hz\n");
	
	FILE *fptr;
    fptr = fopen("input.txt","w");
    fprintf(fptr,"#T            I\n");
	
	double period =  (double) 1/SAMPFREQ;
	
	while(bufferIndex < BUFSIZE){
	    
		fprintf(fptr,"%f            %f \n", (T*period), 1.5*sin((double)500/(T*period))+ 3*cos((double)T*period));
		buffer[T] = 1.5*sin((double)500/(T*period))+ 3*cos((double)T*period);
		bufferIndex++;
	    T++;
	
   }
   
   fclose(fptr);
}

// puts data points onto output (this case a file) at user defined frequency
void play (double buffer[]){
	
	int T =  0;
	bufferIndex = 0;
	
	// sets the dac output frequency based on user input
	double dacFreq = outFeq();
    
	// sets up file to write to	
	FILE *fptr;
    fptr = fopen("output1.txt","w");
    fprintf(fptr,"#T            O\n");
    
    double period =  1/dacFreq;
	
	// writes to file at defined defined period
	while(bufferIndex < BUFSIZE){
	    
		fprintf(fptr,"%f            %f \n", (T*period), 	buffer[T] );
		bufferIndex++;
	    T++;
	
   }
   
   fclose(fptr);
}

// returns frequency set by the user via adc pot value
double outFeq(){
	// 12bit pot adc value so adc value is between 0 and 4095;
	
	double audioOutFreq; 
	int potValue;
	double scale;
	
	audioOutFreq = 8000; // sample frequency of input signal using this as a base
	scale = 1;
	
	printf("\n Enter the ADC value between 0 and 4095 \n***Values between 0 and 2047 decreases frequency***\n***Values between 2048 and 4095 increases frequency***\n");
    scanf("%d", &potValue);
	
	// pot value between 0 and 2047 should  scale 8000hz frequency between 0.1 and 1 
	if(potValue <= 2047){
	    scale = (potValue/2274.4) + 0.1; 
	}
	
	// pot value between 2048 and 4095 should scale 8000hz frequency between 1 and 10	
	else if( potValue >= 2048 && potValue <= 4095){
		scale = ((potValue - 2048)/227.44) + 1;
	}
	
	// returns the new scaled freqency
	printf("Output waveform frequency: %f Hz\n", audioOutFreq * scale);
	return (audioOutFreq * scale); 
}
