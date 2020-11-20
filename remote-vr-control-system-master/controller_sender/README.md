# What is this component? 

This comoponent is a simple script to send controls data from an xbox joystick to the Onboard Computer/UAV. 

## How to use this component? 
1. Download the repo and make sure that your xbox controller is connected. 
2. Find the ip address of the Onboard Computer by running  `ifconfig` on the Pi and put it in the code:
    ```bash
    char *server = "<your_ip_address>";
    ```

3. Compile the code by running: 
    ```bash
    g++ -std=c++11 joystick_sender.cpp -o <some_output_file>;
    ```
    To get the regular UAV control with regular and precision control
    OR 
    ```bash
    g++ -std=c++11 measure_latency.cpp -o <some_output_file>;
    ```
    To measure the latency of sent commands to a logfile.

4. And later you can run the binary saved to the output file you specified: 
    ```bash
    ./<some_output_file>;
    ```


## Requirements: 
- XBOX joystick connected over usb to the machine. 
- needs to run on Ubuntu/Linux 
- Specify the IP address of the pi in code before compiling. 
