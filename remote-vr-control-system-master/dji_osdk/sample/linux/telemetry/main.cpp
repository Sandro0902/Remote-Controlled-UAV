/*
asish mehta
 */

#include "telemetry_sample.hpp"

using namespace DJI::OSDK;
using namespace DJI::OSDK::Telemetry;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFLEN 2048
#define MSGS 5  /* number of messages to send */
#define SERVICE_PORT  21234

int
main(int argc, char** argv)
{

  // Setup OSDK.
  LinuxSetup linuxEnvironment(argc, argv);
  Vehicle*   vehicle = linuxEnvironment.getVehicle();
  if (vehicle == NULL)
  {
    std::cout << "Vehicle not initialized, exiting.\n";
    return -1;
  }

  //declare variables for sending udp packets
  struct sockaddr_in myaddr, remaddr;
  int fd, i;
  socklen_t slen = sizeof(remaddr);
  char buf[BUFLEN]; /* message buffer */
  int recvlen;    /* # bytes in acknowledgement message */
  char *server = "10.40.254.207";  /* change this to use a different server --> change to macbook address */
  std::string data;


  // Display interactive prompt
  std::cout
    << "| Available commands:                                            |"
    << std::endl;
  std::cout
    << "| [a] Get telemetry data and print                               |\n"
    << "| [b] Select some subscription topics to print                   |\n"
    << "| [c] Get telemetry data and save to file                        |\n"
    << "| [d] Start broadcasting telemetry data to macbook                        |"

    << std::endl;
  char inputChar;
  std::cin >> inputChar;

  switch (inputChar)
  {
    case 'a':
      if (vehicle->getFwVersion() == Version::M100_31)
      {
        getBroadcastData(vehicle);
      }
      else
      {
        subscribeToData(vehicle);
      }
      break;
    case 'b':
      subscribeToDataForInteractivePrint(vehicle);
      break;
    case 'c':
      subscribeToDataAndSaveLogToFile(vehicle);
      break;
    case 'd':

      

      /* create a socket */

      if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
        printf("socket created\n");

      /* bind it to all local addresses and pick any port number */

      memset((char *)&myaddr, 0, sizeof(myaddr));
      myaddr.sin_family = AF_INET;
      myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      myaddr.sin_port = htons(0);

      if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
      }       

      /* now define remaddr, the address to whom we want to send messages */
      /* For convenience, the host address is expressed as a numeric IP address */
      /* that we will convert to a binary format via inet_aton */

      memset((char *) &remaddr, 0, sizeof(remaddr));
      remaddr.sin_family = AF_INET;
      remaddr.sin_port = htons(SERVICE_PORT);
      if (inet_aton(server, &remaddr.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
      }

      std::cout << "now we are going to start sending the telemetry data!\n" << std::endl;


      while(1) 
      {
        printf("Sending packet %d to %s port %d\n", i, server, SERVICE_PORT);
        sprintf(buf, "This is packet %d", i);

        //we need to get the telemetry data here and then pass it along
        data = getBroadcastData_tcp(vehicle);


        if (sendto(fd, data.c_str(), data.length(), 0, (struct sockaddr *)&remaddr, slen)==-1) 
        {
          perror("sendto");
          exit(1);
        }
        /* now receive an acknowledgement from the server */
        recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr *)&remaddr, &slen);
                    if (recvlen >= 0) 
                    {
                            buf[recvlen] = 0; /* expect a printable string - terminate it */
                            printf("received message: \"%s\"\n", buf);
                    }
        usleep(500000);
     }
     close(fd);


      break;


    default:
      break;
  }

  return 0;
}
