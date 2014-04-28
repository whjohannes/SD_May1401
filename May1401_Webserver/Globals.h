
/*
 Define global variables here to be used in all other files
 
*/

#ifndef Globals_h
#define Globals_h
#endif
 
typedef struct
{
String Name; 
int Visible; 
String Time1; 
int  duration1;
String Time2; 
int  duration2;
String Time3; 
int  duration3;
} zone_properties;

typedef enum zone_list 
{ 
	ZONE1, 
	ZONE2,
	ZONE3,
	ZONE4,
	ZONE5,
	ZONE6,
	ZONE7,
	ZONE8,
	ZONE9,
	ZONE10,
	ZONE11,
	ZONE12,
	ZONE13,
	ZONE14,
	ZONE15,
	ZONE16
};


#ifndef OPEN
#define OPEN LOW
#endif

#ifndef CLOSE
#define CLOSE HIGH
#endif

#ifndef LOG_SIZE
#define LOG_SIZE 25
#endif

#ifndef LOG_MESSAGE
#define LOG_MESSAGE 20
#endif

#ifndef NUM_USERS
#define NUM_USERS 4
#endif

//Define output pins/ input (flow sensor) for relay board
#ifndef ZONE1_PIN_OUT
#define ZONE1_PIN_OUT 25
#endif

#ifndef ZONE2_PIN_OUT
#define ZONE2_PIN_OUT 23
#endif

#ifndef ZONE3_PIN_OUT
#define ZONE3_PIN_OUT 29
#endif

#ifndef ZONE4_PIN_OUT
#define ZONE4_PIN_OUT 27
#endif

#ifndef ZONE5_PIN_OUT
#define ZONE5_PIN_OUT 33
#endif

#ifndef ZONE6_PIN_OUT
#define ZONE6_PIN_OUT 31
#endif

#ifndef ZONE7_PIN_OUT
#define ZONE7_PIN_OUT 37
#endif

#ifndef ZONE8_PIN_OUT
#define ZONE8_PIN_OUT 35
#endif

#ifndef ZONE9_PIN_OUT
#define ZONE9_PIN_OUT 41
#endif

#ifndef ZONE10_PIN_OUT
#define ZONE10_PIN_OUT 39
#endif

#ifndef ZONE11_PIN_OUT
#define ZONE11_PIN_OUT 45
#endif

#ifndef ZONE12_PIN_OUT
#define ZONE12_PIN_OUT 43
#endif

#ifndef ZONE13_PIN_OUT
#define ZONE13_PIN_OUT 49
#endif

#ifndef ZONE14_PIN_OUT
#define ZONE14_PIN_OUT 47
#endif

#ifndef ZONE15_PIN_OUT
#define ZONE15_PIN_OUT 53
#endif

#ifndef ZONE16_PIN_OUT
#define ZONE16_PIN_OUT 51
#endif

//INPUTS
#ifndef ZONE1_PIN_IN
#define ZONE1_PIN_IN 36
#endif

#ifndef ZONE2_PIN_IN
#define ZONE2_PIN_IN 34
#endif

#ifndef ZONE3_PIN_IN
#define ZONE3_PIN_IN 32
#endif

#ifndef ZONE4_PIN_IN
#define ZONE4_PIN_IN 30
#endif

#ifndef ZONE5_PIN_IN
#define ZONE5_PIN_IN 28
#endif

#ifndef ZONE6_PIN_IN
#define ZONE6_PIN_IN 26
#endif

#ifndef ZONE7_PIN_IN
#define ZONE7_PIN_IN 24
#endif

#ifndef ZONE8_PIN_IN
#define ZONE8_PIN_IN 22
#endif

#ifndef ZONE9_PIN_IN
#define ZONE9_PIN_IN 52
#endif

#ifndef ZONE10_PIN_IN
#define ZONE10_PIN_IN 50
#endif

#ifndef ZONE11_PIN_IN
#define ZONE11_PIN_IN 48
#endif

#ifndef ZONE12_PIN_IN
#define ZONE12_PIN_IN 46
#endif

#ifndef ZONE13_PIN_IN
#define ZONE13_PIN_IN 44
#endif

#ifndef ZONE14_PIN_IN
#define ZONE14_PIN_IN 42
#endif

#ifndef ZONE15_PIN_IN
#define ZONE15_PIN_IN 40
#endif

#ifndef ZONE16_PIN_IN
#define ZONE16_PIN_IN 38
#endif
