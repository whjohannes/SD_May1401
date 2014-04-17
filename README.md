This repository is to be used by SD May14-01 for the Greenhouse watering system at ISU

Current State:
 - Website builds in Arduino using the Wiznet Ethernet shield.
 - Time is displaying correctly on the config page
 - Navigation between pages
 - Parsing of config file to load previous settings
 - Structure between IOs for XML requests
 
TODO:
 - Use pinout from excel spreadsheet to define zones
 - Write config file after "Update zones" button pressed
 - Handle GET requests from html for changing zone states
 - Implement and test LCD display functionality
 - Share variables and arrays between LCD library and website control 
 
HOW TO USE
 - Load website files onto SD card
 - Open webserver code in Arduino 
 -   Modify IP address to suit network
 - Upload to Arduino with ethernet shield installed
 - Visit assigned IP address (~10 second load)
 - Navigate to "Config > Setup Zones" to choose which zones to make visible and name specific zones
 - Assign start times and durations to each zone
 - Verify IO pinout with relays
