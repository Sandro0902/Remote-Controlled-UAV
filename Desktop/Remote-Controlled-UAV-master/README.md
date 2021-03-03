# Remote-VR-Control-System

Remote VR Control System for Cellular Connected UAV

## How to use the repo

- Create separate directories for your separate, self-contained parts of the project 
- Follow git flow guidelines to colllaborate with others. 

## What's where?

From the top level directory there are 4 main directories: 
* latency_analysis - direcotory with python scripts used to analyse the results from latency experiments 
* controller_sender - includes script running on the Ground Station to send control data to the UAV 
* dji_osdk - is the OSDK running on the Onboard Computer used to control the UAV 
* video_streaming - extra directory describbing how to stream data from DJI X3 camera using RTMP 

Each of these directories has a separate README.md file, that is describing how to compile and run the code.

## NOTE: 

Majority if not all of the developed code is designated to work on Linux OS. We have used Ubuntu and highly recommend doing the same. 

## Important notes for measurement system

In order to run the measurement system, you have to checkout to a different branch. Do so by running:
```bash
git checkout latency_measurement
```

Then you should have all the log files, notebooks with data analysis and more. 

