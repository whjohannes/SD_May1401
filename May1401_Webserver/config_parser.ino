#include <globals.h>
#include <stdio.h>
#include <SD.h>
#include <string.h>

String config_array[NUM_ZONES][NUM_PROPS]; //Name, Visible, Time1, Dur1, Time2, Dur2, Time3, Dur3, Time4, Dur4
File config_file;
char temp_str[100];


int split_config()
{

	config_file = SD.open("config.h");        // open web page file
    if (config_file) {
    	while(config_file.available()) {
			//Serial.println(config_file.readStringUntil(','));
    	
			for(int i = 0; i < NUM_ZONES; i++)
			{
				for(int j = 0; j < NUM_PROPS; j++)
				{
					//TODO: Fill config_array
					config_array[i][j] = config_file.readStringUntil(',');
				}
			}
		}
	}
}