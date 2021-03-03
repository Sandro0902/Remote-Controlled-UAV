#include "dji_linux_helpers.hpp"
#include <fstream>
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include "dji_control.hpp"
#include <dji_virtual_rc.hpp>
#include <fcntl.h>
#include <iostream>
#include <limits.h>
#include <linux/joystick.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
//protobuf
#include "controller_message.pb.h"
#include "UAV_Message.pb.h"

using namespace DJI;
using namespace DJI::OSDK;

#define SERVICE_PORT 21234
#define BUFSIZE 4096

float32_t convert_short_to_range(short upper_bound, short lower_bound,
                                 short value, double precision_constant) {
  double new_upper_bound = precision_constant * upper_bound;
  double new_lower_bound = precision_constant * lower_bound;
  double old_range = (SHRT_MAX - SHRT_MIN);
  double new_range = (new_upper_bound - new_lower_bound);
  double new_value =
      ((((value - SHRT_MIN) * new_range) / old_range) + new_lower_bound);
  return (float32_t)new_value;
}

int main(int argc, char **argv) {
  int functionTimeout = 1;

  // initialize the networking variables
  struct sockaddr_in myaddr;           /* our address */
  struct sockaddr_in remaddr;          /* remote address */
  socklen_t addrlen = sizeof(remaddr); /* length of addresses */
  int fd;                              /* our socket */
  unsigned char recvbuf[BUFSIZE];          /* receive buffer */
  unsigned char sendbuf[BUFSIZE];
  int msgint[5];


  std::fstream fout;
  fout.open("controls_log1.csv", std::ios::out | std::ios::app);

  /* create a UDP socket */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket\n");
    return 0;
  }
  //  Set the socket to non-blocking mode
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 10;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

  /* bind the socket to any valid IP address and a specific port */
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(SERVICE_PORT);

  if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed");
    return 0;
  }

  LinuxSetup linuxEnvironment(argc, argv);
  Vehicle *vehicle = linuxEnvironment.getVehicle();
  if (vehicle == NULL) {
    std::cout << "Vehicle not initialized, exiting.\n";
    return -1;
  }

  // Obtain Control Authority
  vehicle->obtainCtrlAuthority(functionTimeout);

  // Get acceleration, velocity, angular rate from UAV
  Telemetry::Vector3f velocity;
  Vmsg::UAVmsg::velocity* velocityMsg;

  Telemetry::Vector3f acceleration; 
  Vmsg::UAVmsg::acceleration* accelerationMsg;

  Telemetry::Vector3f angular_Rate;
  Vmsg::UAVmsg::angle* angleMsg;

  // Display interactive prompt
  std::cout
      << "| Available commands:                                            |"
      << std::endl;
  std::cout << "\n | [c] for custom controls                                   "
               "       |"
            << std::endl;
  char inputChar;
  std::cin >> inputChar;

  switch (inputChar) {
  case '.': {
    uint8_t ctrl_flag = (Control::HORIZONTAL_VELOCITY |
                         Control::VERTICAL_VELOCITY | Control::YAW_RATE |
                         Control::HORIZONTAL_BODY | Control::STABLE_DISABLE);

    DJI::OSDK::Control::CtrlData data(ctrl_flag, (float32_t)0, (float32_t)0,
                                      (float32_t)0, (float32_t)0);

    // Start with normal mode.
    struct timespec gettime_now;
    double mode = 1.0;
    auto start = std::chrono::system_clock::now();

    Controller::ControlMsg* controlMessage;

    while (1) {
      auto now = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = now - start;
      std::cout << "now: " << elapsed_seconds.count() << std::endl;
      printf("waiting on port %d\n", SERVICE_PORT);

      velocity = vehicle->broadcast->getVelocity();
      velocityMsg->set_velocity_x(velocity.x);
      velocityMsg->set_velocity_y(velocity.y);
      velocityMsg->set_velocity_z(velocity.z);
      acceleration = vehicle->broadcast->getAcceleration();
      accelerationMsg->set_acc_pitch(acceleration.x);
      accelerationMsg->set_acc_roll(acceleration.y);
      accelerationMsg->set_acc_thrust(acceleration.z);
      angular_Rate = vehicle->broadcast->getAngularRate();
      angleMsg->set_ang_pitch(angular_Rate.x);
      angleMsg->set_ang_roll(angular_Rate.y);
      angleMsg->set_ang_thrust(angular_Rate.z);

      if(velocityMsg->SerializeToArray(sendbuf, BUFSIZE)){
            std::cout<<"Serialize to Array success"<<std::endl;
            }

      if (recvfrom(fd, recvbuf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen) >
          0) {
        // Set the mode variable between 0.01 to 0.99, precision mode.

        controlMessage->ParseFromArray(recvbuf, BUFSIZE);
        msgint[0] = controlMessage->pitch();
        msgint[1] = controlMessage->roll();
        msgint[2] = controlMessage->thrust();
        msgint[3] = controlMessage->yaw();
        msgint[4] = controlMessage->mode();

        mode = 0.2;
        data = Control::CtrlData(
            ctrl_flag, convert_short_to_range(29, -29, msgint[1], mode),  //x
            convert_short_to_range(29, -29, msgint[0], mode),             //y
            convert_short_to_range(4, -4, msgint[2] * (-1), mode),        //z
            convert_short_to_range(149, -149, msgint[3], mode));          //yaw

      } else {
        data = Control::CtrlData(ctrl_flag, (float32_t)0, (float32_t)0,
                                 (float32_t)0, (float32_t)0);
      }
      std::cout << "PRECISION: " << mode << std::endl;
      std::cout << "\nroll: " << data.x << "\npitch: " << data.y
                << "\nthrust: " << data.z << "\nyaw: " << data.yaw << std::endl;
  

      if(sendto(fd, sendbuf, BUFSIZE, 0, (struct sockaddr *)&remaddr, addrlen)
       == -1){
        perror("Error on sending the data through socket. ");
        exit(1);
      }
      
      vehicle->control->flightCtrl(data);
      usleep(20000);
    }
  }
  default:
    break;
  }

  return 0;
}
