
/*
 Define global variables here to be used in all other files
*/

#ifndef NUM_ZONES
#define NUM_ZONES 15
#endif

#ifndef NUM_PROPS
#define NUM_PROPS 9
#endif

typedef struct 
{
char Name[9]; 
char Visible; 
char Time1[6]; 
int  Dur1;
char Time2[6]; 
int  Dur2;
char Time3[6]; 
int  Dur3;
char Time4[6]; 
int  Dur4;
} zone_properties;