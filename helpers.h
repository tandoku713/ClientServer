
#include<time.h>
#include<vector>
#include<string>
using namespace std;
#define BUFLEN 1024
#define LEN 1024
#define REJECT -1
#define HELLO -3
#define ACCEPT 0
#define LISTCLIENTS 1
#define INFOCLIENT 2
#define MESSAGE 3
#define  SHAREFILE 4
#define UNSHAREFILE 5
#define GETSHARE 6
#define GETFILE 7
#define QUIT 8
#define STATUS 9
#define END 10
#define CONNECT 11
#define BEGIN 12
struct message
{
  int type ;
  int req;
  char payload[LEN];  
};

struct client
{
string name;
int port; 
int listport;
string IP;
int sockfd;
int listensockfd;
time_t _time;
vector <string> files;
};

