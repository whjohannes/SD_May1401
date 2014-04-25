#include <Globals.h>
#include <stdio.h>
#include <SD.h>
#include <string.h>

String config_file_string;
//String config_array[NUM_ZONES][NUM_PROPS]; //Name, Visible, Time1, Dur1, Time2, Dur2, Time3, Dur3, Time4, Dur4
File config_file;
int comma_position;
char temp_char;

int split_config(String config_array[NUM_ZONES][NUM_PROPS])
{
	config_file = SD.open("config.h");        // open web page file
    if (config_file) {
    	while(config_file.available()) 
    	{
    		config_file_string.concat( (char) config_file.read());
    	}
    //	Serial.println(config_file_string);
    //	Serial.println("TP1");
		for(int i = 0; i < NUM_ZONES; i++)
		{
			for(int j = 0; j < NUM_PROPS; j++)
			{
				comma_position = config_file_string.indexOf(',');
			//	Serial.print("comma pos ");
			//	Serial.println(comma_position);

				if(comma_position != -1)
				{
      				//Serial.println( config_file_string.substring(0,comma_position));
      				config_array[i][j] = config_file_string.substring(0,comma_position);
      				config_file_string = config_file_string.substring(comma_position+1, config_file_string.length());
				}
		
			}
		}
	config_file.close();
	 return 1;
	}
	else
	{
		return 0;
	}
}

 