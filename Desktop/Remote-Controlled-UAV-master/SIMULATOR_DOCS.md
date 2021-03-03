# Documentation on running the simulation

## Requirements:
- Install DJI Assistant 2. Available on Mac and Windows.
 
##### Note:
 If using Mac OS Catalina, install the driver separately from this discussion. https://github.com/jwise/HoRNDIS/issues/102
 
## Steps for running the testbed:
1. Connect Raspberry Pi USB - drone CAN port. 
2. Turn on the Raspberry Pi. 
3. Configure internet access. 
4. Turn on the DJI drone and controller. 
5. Turn on the propellers. 
6. Plug in laptop with DJI Assistant to drone and start the simulation.
7. Start ./dji-flight-controller UserConfig.txt script on Pi. When prompted type: . and then enter.
8. Run ./joystick_sender_2.cpp to stream the signal to Pi. 

Then you should see how the drone control is simulated in the DJI Assistant app. 

