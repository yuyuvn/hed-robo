/*
* Hello World and Crash for Turtlebot
* Moves Turtlebot forward until you ctrl + c
* Tested using TurtleBot 2, ROS Indigo, Ubuntu 14.04
*/
/* 動かし方　
roslaunch turtlebot_bringup minimal_nomovebase.launch
~/myprog/devel/lib/move/move
*/

#include <stdio.h>    /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>   /* for atoi() and exit() */
#include <string.h>   /* for memset() */
#include <unistd.h>   /* for close() */
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <errno.h>
#include <ifaddrs.h>
#include<pthread.h>
#include<fcntl.h>
#include <assert.h>
#define MAXRECVSTRING 255  /* Longest string to receive */
#include <iostream>
#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <signal.h>
#include <fstream>
#include <sys/wait.h>

;/* External error handling function */
void DieWithError(char const *errorMessage) {
  ROS_INFO_STREAM(errorMessage);
  _Exit(3); // change to _Exit(3); if receivebroadcast in child process
}

typedef struct{
  char IP[16];
  char capcom_port[6];
  char stream_port[6];
} ConnectionData;

char** createArray(int m, int n)
{
  char* values = (char*)calloc(m*n, sizeof(char));
  char** rows = (char**)malloc(n*sizeof(char*));
  int i = 0;
  for (i = 0; i<n; ++i)
  {
    rows[i] = values + i*m;
  }
  return rows;
}
void destroyArray(char** arr)
{
  free(*arr);
  free(arr);
}
char** str_split(char* a_str, const char a_delim)
{
  char** result = 0;
  size_t count = 0;
  char* tmp = a_str;
  char* last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;
  
  /* Count how many elements will be extracted. */
  while (*tmp)
  {
    if (a_delim == *tmp)
    {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }
  
  /* Add space for trailing token. */
  count += last_comma < (a_str + strlen(a_str) - 1);
  
  /* Add space for terminating null string so caller
  knows where the list of returned strings ends. */
  count++;
  
  result = (char**)malloc(sizeof(char*) * count);
  
  if (result)
  {
    size_t idx = 0;
    char* token = strtok(a_str, delim);
    
    while (token)
    {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }
  
  return result;
}
void parse(char args[], char **infor)
{
  char data[100];
  strcpy(data, args);
  char** tokens;
  char **token2s;
  //char infor[3][20];
  //printf("datas=[%s]\n\n", data);
  
  tokens = str_split(data, '\n');
  
  if (tokens)
  {
    int i;
    int j = 0;
    for (i = 0; *(tokens + i); i++)
    {
      token2s = str_split(*(tokens + i), ':');
      if ((*(token2s + 1)) != NULL)
      {
        strcpy(*(infor + j), *(token2s + 1));
        j++;
      }
    }
  }
}
void boardcastReceive(char* IPAdd)
{
  char* IP=IPAdd;
  char const *argv="11000";
  int sock;             /* Socket */
  struct sockaddr_in broadcastAddr; /* Broadcast Address */
  unsigned short broadcastPort;   /* Port */
  char recvString[MAXRECVSTRING+1]; /* Buffer for received string */
  int recvStringLen;        /* Length of received string */
  
  //if (argc != 2)  /* Test for correct number of arguments */
  //{
    //fprintf(stderr,"Usage: %s <Broadcast Port>\n", argv[0]);
    //exit(1);
  //}
  
  broadcastPort = atoi(argv);   /* First arg: broadcast port */
  
  /* Create a best-effort datagram socket using UDP */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  DieWithError("socket() failed");
  
  /* Construct bind structure */
  memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
  broadcastAddr.sin_family = AF_INET;         /* Internet address family */
  broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
  broadcastAddr.sin_port = htons(broadcastPort);    /* Broadcast port */
  
  /* Bind to the broadcast port */
  if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
  DieWithError("bind() failed");
  
  //fcntl(sock,F_SETFL,O_NONBLOCK);
  
  /* Receive a single datagram from the server */
  if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
  DieWithError("recvfrom() failed");
  
  recvString[recvStringLen] = '\0';
  //printf("Received: %s\n", recvString);  /* Print the received string */
  strcpy(IP,recvString);
  close(sock);
}
void receiveCommand(char* ip, char* port, ros::Publisher cmd_vel_pub_)
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  
  char buffer[256];
  
  portno = atoi(port);
  
  /* Create a socket point */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (sockfd < 0) {
    ROS_INFO_STREAM("ERROR opening socket");
    return;
  }
  
  server = gethostbyname(ip);
  
  if (server == NULL) {
    ROS_INFO_STREAM("ERROR, no such host");
    return;
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  
  /* Now connect to the server */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    ROS_INFO_STREAM("ERROR connecting");
    return;
  }
  
  bzero(buffer,256);
  strcpy(buffer,"HED-Robo v1.0");
  
  /* Send message to the server */
  n = write(sockfd, buffer, strlen(buffer));
  
  if (n < 0) {
    ROS_INFO_STREAM("ERROR writing to socket");
    return;
  }
  
  float left_X,left_Y, right_trigger, left_trigger;
  //init direction that turtlebot should go
  geometry_msgs::Twist base_cmd;
  
  base_cmd.linear.x = 0;
  base_cmd.linear.y = 0;
  base_cmd.angular.z = 0;
  while(1) {
    /* Now read server response */
    bzero(buffer,256);
    n = read(sockfd, buffer, 16);
    
    if (n < 0) {
      ROS_INFO_STREAM("ERROR reading from socket");
      return;
      } else if (n != 16) {
      break;
    }
    
    memcpy(&left_X, buffer, 4);
    memcpy(&left_Y, buffer+4, 4);
    memcpy(&right_trigger, buffer+8, 4);
    memcpy(&left_trigger, buffer+12, 4);
    
    base_cmd.linear.x = right_trigger - left_trigger;
    base_cmd.angular.z = -left_X;
    
    cmd_vel_pub_.publish(base_cmd);
  }
}
void sstream(char* ip, char* port)
{
  char buffer[256];
  
  //sprintf(buffer, "rtsp://%s:%s/live.sdp", ip, port);
  //execl("/usr/bin/ffmpeg", "ffmpeg", "-f", "v4l2", "-i", "/dev/video0", "-r", "50", "-vcodec", "mpeg2video", "-b:v", "1000k", "-f", "rtsp", "-rtsp_transport", "tcp", buffer, NULL);
  
  //sprintf(buffer, "ffmpeg -f v4l2 -i "/dev/video0" -r 50 -vcodec mpeg2video -b:v 1000k -f rtsp -rtsp_transport tcp rtsp://%s:%s/live.sdp", ip, port);
  //sprintf(buffer, "ffmpeg -f v4l2 -i /dev/video0 -r 50 -vcodec mpeg2video -b:v 500k -f rtsp -rtsp_transport tcp rtsp://%s:%s/live.sdp", ip, port);
  sprintf(buffer, "nice -10 ffmpeg -f v4l2 -i /dev/video0 -r 50 -vcodec mpeg2video -deinterlace -b:v 8000 -f rtsp -rtsp_transport tcp rtsp://%s:%s/live.sdp", ip, port);
  system(buffer);
}

void write(char* const message)
{
  std::ofstream out("capcominfo.txt", std::ofstream::trunc);
  out << message;
  out.close();
}

void reset_robo(ros::Publisher cmd_vel_pub_)
{
  geometry_msgs::Twist base_cmd;
  base_cmd.linear.x = 0;
  base_cmd.linear.y = 0;
  base_cmd.angular.z = 0;
  cmd_vel_pub_.publish(base_cmd);
}

int main(int argc, char** argv)
{
  //init the ROS node
  ROS_INFO_STREAM("Hedspi Robo v1.0");
  ros::init(argc, argv, "robot_driver");
  ros::NodeHandle nh;
  
  //init publisher
  ros::Publisher cmd_vel_pub_;
  cmd_vel_pub_ = nh.advertise<geometry_msgs::Twist>("cmd_vel_mux/input/teleop", 1);
  
  
  ros::Rate rate(60); // 60fps
  
  
  char data[MAXRECVSTRING+1];
  int pid;
  char** output = NULL;
  
Start:
  pid = fork();
  if(pid<0){
    ROS_INFO_STREAM("Cannot create child process.");
    exit(1);
  } else if (pid==0) {
    boardcastReceive(data);
    write(data);
    _Exit(0);
  } else {
    int c_rvalue, c_exited = 0;
    while(nh.ok()) {
      c_exited = waitpid(pid, &c_rvalue, WNOHANG);
      
      if (c_exited>0&&c_rvalue==0) {
        ROS_INFO_STREAM("Capcom connected.");
        std::ifstream t("capcominfo.txt");
        std::stringstream buffer;
        buffer << t.rdbuf();
        strcpy(data, buffer.str().c_str());
        t.close();
        goto Connected;
      }
    }
    kill(pid, SIGTERM);
    goto Clean;
  }
  
Connected:
  ConnectionData cdata;
  output = createArray(20, 3);
  
  parse(data, output);
  strcpy(cdata.IP, output[0]);
  strcpy(cdata.capcom_port, output[1]);
  strcpy(cdata.stream_port, output[2]);
  destroyArray(output);
  
  pid = fork();
  if(pid<0){
    ROS_INFO_STREAM("Cannot create child process.");
    exit(1);
  } else if(pid==0){
    sstream(cdata.IP,cdata.stream_port);
    _Exit(0);
  } else {
    receiveCommand(cdata.IP,cdata.capcom_port,cmd_vel_pub_);
    kill(pid, SIGTERM);
    
    reset_robo(cmd_vel_pub_);
    
    if(nh.ok()) {
      goto Start;
    }
  }
  
Clean:
  reset_robo(cmd_vel_pub_);
  remove( "capcominfo.txt" );
  ROS_INFO_STREAM("Finished");
  return 0;
}