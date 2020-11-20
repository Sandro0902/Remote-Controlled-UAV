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
#include <fstream>
#include <string>
#include <iostream>

using namespace std::chrono;

#define BUFLEN 4096
#define SERVICE_PORT 21234

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

short normalize(short value){
  if (value < 1200 && value > -1200){
    return 0;
  }
  return value;
}

int main(int argc, char *argv[]) {
  struct sockaddr_in myaddr, remaddr;
  int fd, i;
  socklen_t slen = sizeof(remaddr);
  char buf[BUFLEN];
  int recvlen;
  char *server = "172.16.0.4";
  int msgint[5] = {0, 0, 0, 0, 0};



  const char *device;
  int js;
  struct js_event event;
  struct axis_state axes[3] = {0};
  size_t axis;

  if (argc > 1) {
    device = argv[1];
  } else {
    device = "/dev/input/js0";
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

  std::fstream fout;
  int id = 1;
  while (true)  {
    if (read_event(js, &event) != -1){
      axis = get_axis_state(&event, axes);
      msgint[0] = normalize(axes[2].x); // pitch
      msgint[1] = normalize(axes[1].y); // roll
      msgint[2] = normalize(axes[0].y); // thrust/vertical control
      msgint[3] = normalize(axes[0].x); // yaw
    }
    msgint[4] = id;
    std::cout << "`\n roll: " << msgint[1]
              << "\n pitch: " <<  msgint[0]
              << "\n thrust/v: " << msgint[2]
              << "\n yaw: " << msgint[3]
              << "\n id:  " << msgint[4] << std::endl;

    if (sendto(fd, msgint, (sizeof(msgint)), 0, (struct sockaddr *)&remaddr,
               slen) == -1) {
      perror("Error on sending the data through socket. ");
      exit(1);
    } else {
      long value_ms = duration_cast<milliseconds>(time_point_cast<::milliseconds>(high_resolution_clock::now()).time_since_epoch()).count();
      fout.open("controls_log1.csv", std::ios::out | std::ios::app);
      fout << id << ", " << value_ms << "\n";
      std::cout << value_ms << " 1 " << id << std::endl;
      fout.close();
    }
    id = id + 1;
    usleep(20000);
  }
  close(fd);
  close(js);
  return 0;
}
