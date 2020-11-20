# Intro 

This directory includes the full DJI OSDK, so we can leverage on its API to develop our custom scripts. It runs on Linux Operating Systems, Raspbian in our case. Most of the developed code is in the `sample/linux/flight-control/` directory. 

## Requirements: 

To successfuly run this code and connect with Matrice 100 do: 
* configure the UserConfig.txt file. Check DJI docs to configure 
* use a Linux distribution OS to run this code 
* make sure that USB/UART connection with the drone is done correctly. If in doubt check DJI docs. 
   
## Instructions 

To compile the code, run sequence of this commands in /dji_osdk directory:
```bash 
$ mkdir build  
$ cd build  
$ cmake ..  
$ make
```

Then, once the files are compiled you can run our particular script for controlling the flight by doing: 
```bash
$ cd build/bin  
$ ./djiosdk-flightcontrol-sample UserConfig.txt   
```


## Extra notes 

Extra explanation of the control flags is in the DJI API docs, but here is a short explanation of what setting we're using. 

After numerous tries, this mode of control works best for our purpose
```
uint8_t ctrl_flag = (
        Control::HORIZONTAL_ANGLE |
        Control::VERTICAL_THRUST |
        Control::YAW_RATE |
        Control::HORIZONTAL_BODY |
        Control::STABLE_DISABLE
    );
```
Input data ranges (axes):
 - x : -35 to 35 degrees, left to right 
 - y : -35 to 35 degrees, negative is forward
 - z : -5 to 5 m/s, positive is upwards
 - yaw : -150 to 150 deg/s, negative = rotate left, right if positive
