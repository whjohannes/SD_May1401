
/*
 Define global variables here to be used in all other files
 
*/

#ifndef Globals_h
#define Globals_h

#include <Wstring.h>

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
String Time4; 
int  duration4;
} zone_properties;

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
