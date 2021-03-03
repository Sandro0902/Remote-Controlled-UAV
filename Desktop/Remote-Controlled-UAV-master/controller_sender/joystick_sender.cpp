/**
 * The part of the code to read events from joystick events is based on
 * a project by Jason White that can be found under this link:
 * https://www.kernel.org/doc/Documentation/input/joystick-api.txt
 *
 */
#include <arpa/inet.h>
#include <bitset>
#include <fcntl.h>
#include <iostream>
#include <linux/joystick.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <thread>
//for protobuf
#include "controller_message.pb.h"
#include "UAV_Message.pb.h"

#define BUFLEN 2048
#define MSGS 5 /* number of messages to send */
#define SERVICE_PORT 21234

typedef float  float32_t;
/**
 * Reads a joystick event from the joystick device.
 *
 * Returns 0 on success. Otherwise -1 is returned.
 */
int read_event(int fd, struct js_event *event) {
    ssize_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event)) {
        return 0;
    }
    /* Error, could not read full event. */
    return -1;
}

/**
 * Current state of an axis.
 */
struct axis_state {
    short x, y;
};

struct send_state {
    float x, y;
};

/**
 * Keeps track of the current axis state.
 *
 * NOTE: This function assumes that axes are numbered starting from 0, and that
 * the X axis is an even number, and the Y axis is an odd number. However, this
 * is usually a safe assumption.
 *
 * Returns the axis that the event indicated.
 */
size_t get_axis_state(struct js_event *event, struct axis_state axes[3]) {
    size_t axis = event->number / 2;

    if (axis < 3) {
        if (event->number % 2 == 0) {
            axes[axis].x = event->value;
        } else {
            axes[axis].y = event->value;
        }
    }
    return axis;
}

//Avoid small movement or imprecision of the axis to be read
short normalize(short value){
  if (value < 1200 && value > -1200){
    return 0;
  }
  return value;
}

int main(int argc, char *argv[]) {

    struct sockaddr_in myaddr, remaddr;             //Soket Address Input: myAddress, remoteAddress
    int fd, i;
    socklen_t slen = sizeof(remaddr);               //Soket length
    char sendbuf[BUFLEN];
    char recvbuf[BUFLEN];
    int recvlen;                                    //Length of the receiver
    char const*server = "10.101.160.200";              
    int msgint[5] = {0, 0, 0, 0, 0};                //Sends 5 messages per time


    float32_t velocityInfo[3];

    // controller declarations
    const char *device;
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};                
    size_t axis;

    if (argc > 1) {
        device = argv[1];
    } else {
        device = "/dev/input/js2";                  //Please check for your input port, for my case it is js2
    }

    js = open(device, O_NONBLOCK);

    if (js == -1) {
        perror("Could not open joystick");
    }

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("socket created\n");
    }

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);

    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }

    memset((char *)&remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(SERVICE_PORT);
    if (inet_aton(server, &remaddr.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    Controller::ControlMsg* controlMessage;
    Vmsg::UAVmsg::velocity* velocityMsg;

    bool is_precision_mode = true;
    while (true)  {

        if (recvfrom(fd, recvbuf, BUFLEN, 0, (struct sockaddr *)&remaddr, &slen) >
          0){
              velocityMsg->ParseFromArray(recvbuf, BUFLEN);
              std::cout << "\n Velocity: " << velocityMsg -> velocity_x()
                        << "\n"            << velocityMsg -> velocity_y()
                        << "\n"            << velocityMsg -> velocity_z()
                                           <<std::endl;
          }

      if (read_event(js, &event) != -1){
        if (event.type == JS_EVENT_BUTTON) {
          is_precision_mode = !is_precision_mode;
        }
        axis = get_axis_state(&event, axes);
        //
        msgint[0] = normalize(axes[1].x); // pitch
        msgint[1] = normalize(axes[1].y); // roll
        msgint[2] = normalize(axes[0].y); // thrust/vertical control
        msgint[3] = normalize(axes[0].x); // yaw
        msgint[4] = is_precision_mode ? 100 : 0;
      }

    controlMessage->set_pitch(msgint[0]);
    controlMessage->set_roll(msgint[1]);
    controlMessage->set_thrust(msgint[2]);
    controlMessage->set_yaw(msgint[3]);
    controlMessage->set_mode(msgint[4]);
    
    if(controlMessage->SerializeToArray(sendbuf, BUFLEN)){
        std::cout<<"Serialize to Array success"<<std::endl;
    }

      if(msgint[0] != 0 || msgint[1] != 0 || msgint[2] != 0 || msgint[3] != 0){
      std::cout << "\n roll: " << msgint[0]
                << "\n pitch: " <<  msgint[1]
                << "\n thrust/v: " << msgint[2]
                << "\n yaw: " << msgint[3]
                << "\n mode: " << msgint[4]
                << std::endl;

    if (sendto(fd, sendbuf, BUFLEN, 0, (struct sockaddr *)&remaddr,
          slen) == -1) {
        perror("Error on sending the data through socket. ");
        exit(1);
      }      
      usleep(20000);
    }
    }
    close(fd);
    close(js);
    return 0;
}
