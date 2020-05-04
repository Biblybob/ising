#include <stdio.h>
// globals 
#define BUF_SIZE 80000 // about 10 seconds of buffer (@ 8K samples/sec)
#define BUF_THRESHOLD 96 // 75% of 128 word buffer
	
// function prototypes 
void check_KEYs(int * Record, int * Play, int * Buffer);
void iniTimer(int desiredFreq);
int scalar();


//pointers
volatile int * KEY_ptr = (int *) 0xFF200050;
volatile int * audio_ptr = (int *) 0xFF203040;
volatile int * LED_ptr = (int *) 0xFF200000;
volatile int * timer_ptr = (int *) 0xFF202000;  
volatile int * POT_ptr = (int *) 0xFF204000; // potentiometer pointer (adc) channel 0; 


/*******************************************************************************
* This program performs the following:
* 1. records audio for 10 seconds when KEY[0] is pressed. LEDR[0] is lit
* while recording.
* 2. plays the recorded audio when KEY[1] is pressed. LEDR[1] is lit while
* playing.
******************************************************************************/
int main(void) {

// used for audio record/playback 
int fifospace = * timer_ptr ;
int record = 0;
int play = 0;
int buffer_index = 0;

// array for the left and right audio data points 
int left_buffer[BUF_SIZE];
int right_buffer[BUF_SIZE];
  
	
// read and echo audio data 
record = 0;
play = 0;

while (1) {
	
	// updates record and play variables based on push button status and sets buffer index to zero
	check_KEYs(&record, &play, &buffer_index);
	    
		if (record) {
		 
		  *(LED_ptr) = 0x1; // turn on LEDR[0]
		  fifospace = *(audio_ptr + 1); // read the audio port fifospace register
		 
		  // check RARC (the first byte) of Fifospace
		  if ((fifospace & 0x000000FF) > BUF_THRESHOLD) {
		 
		  // store data until the the audio-in FIFO is empty or the buffer is full
		 
		 while  ((fifospace & 0x000000FF) && (buffer_index < BUF_SIZE)) {
			 
		     left_buffer[buffer_index] = *(audio_ptr + 2);
			 right_buffer[buffer_index] = *(audio_ptr + 3);
			
			 buffer_index++;
			
			 // done recording
			 if (buffer_index == BUF_SIZE) {
			  record = 0;
			 *(LED_ptr) = 0x0; // turn off LEDR
				 
				 // sets up timer requency based on adc pot value 
                 *(timer_ptr + 1) = 0b1011; //stops the timer
                 iniTimer( scalar());
                 *(timer_ptr + 1) = 0b0111;  //starts the timer 
			 }
			 
			 fifospace = *(audio_ptr + 1); // read the audio port fifospace register
		    }
		} 
	}
		
		// play by putting data points in the left right data out buffer at frequency determined by pot
		else if (play) {
	      // turn on LEDR_1
		  *(LED_ptr) = 0x2; 
          fifospace = *(audio_ptr + 1); 
		  
		  // check WSRC 3rd byte fifospace register
		  if ((fifospace & 0x00FF0000) > BUF_THRESHOLD)  {
          
		  // output data until the buffer is empty 
          while  (buffer_index < BUF_SIZE) {
		    
			 // puts next data point onto the out buffers at next clock pulse	
	         if(((*(timer_ptr) & 0x01) == 0x01) && (fifospace & 0x00FF0000)) {
			     *(audio_ptr + 2) = left_buffer[buffer_index];
			     *(audio_ptr + 3) = right_buffer[buffer_index];
			      buffer_index++;
		     }
		     
			  // done playback
			 if (buffer_index == BUF_SIZE) {
			   play = 0;
			   *(LED_ptr) = 0x0; // turn off LEDR
			 }
			 
			 fifospace = *(audio_ptr +1); // read the audio port fifospace register
			}
		}
		}
   }
}


// reads push buttons and updates record, play and buffer variables

void check_KEYs(int * Record, int * Play, int * Buffer) {
    int KEY_value;
	KEY_value = *(KEY_ptr); // read the pushbutton KEY values
	
	// wait for pushbutton KEY release
	while (*KEY_ptr); 
	
	// check if record button is pressed
	if (KEY_value == 0x1) {
	    
		*Buffer = 0; // reset counter to start recording
		
		// clear audio-in FIFO
		*(audio_ptr) = 0x4;
		*(audio_ptr) = 0x0;
		*Record = 1;
	} 
	
	// check if play button is pressed
	else if (KEY_value == 0x2) {
	   
     	*Buffer = 0;  // reset counter to start playback
		
		// clear audio-out FIFO
		*(audio_ptr) = 0x8;
		*(audio_ptr) = 0x0;
		*Play = 1;
}
}

// calculates the scaled clock frequency for the audio output between 800hz to 80000hz based on pot adc input; 
int scalar(){

	// 12bit adc pot value so adc value is between 0 and 4095;
	
	int audioOutFreq; 
	int potValue;
	double scale; 
	
	// updates adc
	*(POT_ptr) = 0x80; 
	
	// reads adc
	*(POT_ptr) = 0x00; 
	potValue =  *(POT_ptr) & 0x3FF; // pot value is only first 12 bits (0011 1111 1111)
	
	audioOutFreq = 8000; // sample frequency of input signal using this as a base
	scale = 1;
	
	// pot value between 0 and 2047 should  scale 8000hz frequency between 0.1 and 1 
	if(potValue <= 2047){
	    scale = (potValue/2274.4) + 0.1; 
	}
	
	// pot value between 2048 and 4095 should scale 8000hz frequency between 1 and 10	
	else if( potValue >= 2048 && potValue <= 4095){
		scale = ((potValue - 2048)/227.44) + 1;
	}
	
	printf("%f \n", audioOutFreq * scale);
	return (int) (audioOutFreq * scale); 
}


// initialises timer with desired frequency parameter
void iniTimer(int desiredFreq){ 
	int counter = 100000000/desiredFreq; // count down value
	*(timer_ptr + 2) = counter; 
	*(timer_ptr + 1) = 0b0011; // config to loop count down and  
}
