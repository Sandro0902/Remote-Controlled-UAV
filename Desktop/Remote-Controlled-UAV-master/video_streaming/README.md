# Streaming video to local PC 

### Requirements:
 - DJI GO app on your android or IOS 
 - VLC Media Player 
 - NGINX-RTMP Server
 
Download the first two positions on your phone and PC. 

Next, follow the tutorial to set up a local Nginx-RTMP server. 
[Piece of cake tutorial here](https://hub.docker.com/r/tiangolo/nginx-rtmp/)

Once everything is up and running we can start streaming the video. 
Please check your ip address using:
```bash
ifconfig
```
We are gonna need it. 

### Starting the stream

1. Launch the DJI GO app. Open the live streaming option and add custom url. Set it to: rtmp://<your_ip>/live/dji
2. Next open the VLC app and open the network stream option: 
- There add rtmp url as rtmp://<your_ip>/live/dji.
- Make sure you select the stream output option and select strea locally option. Use WMV1 codec.

You should be getting the stream already. If not I highly recommend checking out the server logs using: 
```bash
docker logs nginx-rtmp -f 
```
