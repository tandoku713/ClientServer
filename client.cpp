#include <stdio.h>
#include<sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include<string>
#include<time.h>
#include<vector>
#include "helpers.h"
#include<iostream>
#include<queue>
using namespace std;
struct client me;
bool finished = false;
int fdmax;
fd_set read_fds, tmp_fds;

//coada de mesaje
queue<string> qmsg;
//coada pt numele fisierelor
queue<string> qfiles;
//coada pt fisiere
queue<FILE *> qFILE;
void error(char *msg)
{
    perror(msg);
    exit(1);
}


/*
  trimite un mesaj din coada de asteptare catre 
  socketul cu IP-ul si portul din response.payload
*/
void messageback(message response)
{
  pair<string,string> p;
  cout<<"hello\n";
  char buffer[LEN];
  memset(buffer,'\0',LEN);
   memcpy(buffer,response.payload,sizeof(buffer ));
  cout<<"here\n"<<"\n";
   int sock ;
  sock = socket(AF_INET,SOCK_STREAM,0);
  struct sockaddr_in serv_addr;
  stringstream ss(buffer);
  					string str;
  					vector<string> elem;
  					while(getline(ss, str, ' '))elem.push_back(str);	
          
  serv_addr.sin_family = AF_INET;
  //      cout<<"aici\n"<<"\n"<<buffer<<"\n";  
  cout<<elem.at(1).c_str()<<""<<elem.at(0).c_str()<<"\n"; 
  serv_addr.sin_port = htons(atoi(elem.at(1).c_str()));

  inet_aton(elem.at(0).c_str(),&serv_addr.sin_addr);
        cout<<"aici\n";  
    if (connect(sock,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        {
        error((char *)"ERROR: connecting");    
        exit(-1);
        }

   string mesaj=qmsg.front();
   qmsg.pop();
   message msg;
   msg.type = MESSAGE;
   strcpy(msg.payload,mesaj.c_str());
   strcat(msg.payload," de la ");
   strcat(msg.payload, me.name.c_str());
   int n = send(sock,&msg,sizeof(msg),0);
  if(n<0) 
  {
    error((char *)"ERROR: Writing to socket\n");
  }
        
}


/*
  trimite un fisier din coada de asteptare catre 
  socketul cu IP-ul si portul din response.payload
*/
void sendfile(message response)
{
  pair<string,string> p;
 memcpy(&p,response.payload,sizeof(p));
  cout<<p.first<<p.second<<" ";
  int sock ;
  sock = socket(AF_INET,SOCK_STREAM,0);
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(p.second.c_str()));
  inet_aton(p.first.c_str(),&serv_addr.sin_addr);
  
    if (connect(sock,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        {
        
        error((char *)"ERROR connecting");    
        exit(-1);
        }
   string file=qfiles.front();
   qmsg.pop();
   FILE * f = fopen(file.c_str(),"r");
   
   char buffer[LEN];
   int count;
   message msg;
    msg.type = BEGIN;
    strcpy(msg.payload, file.c_str());
   int n = send(sock,&msg,sizeof(msg),0);
  if(n<0) 
  {
    error((char *)"ERROR: Writing to socket\n");
  }
   while((count=fread (buffer,1,LEN,f)>0))
   {
   
   message msg;
   msg.type = GETFILE;
   strcpy(msg.payload,buffer);
   int n = send(sock,&msg,sizeof(msg),0);
  if(n<0) 
  {
    error((char *)"ERROR: Writing to socket\n");
  }
        }
        fclose(f);
         message msg2;
         
   msg2.type = END;
   n = send(sock,&msg2,sizeof(msg2),0);
  if(n<0) 
  {
    error((char *)"ERROR: Writing to socket\n");
  }
  cout<<"messageback";
}


/*
    clientul trimite un mesaj catre server in care ii trimite acestuia numele sau  
*/
void greetings(char *name)
{
  message greetings;
  greetings.type = HELLO;
  memset(greetings.payload,'\0',LEN);
  sprintf(greetings.payload,"%s",name);

  int n = send(me.sockfd,&greetings,sizeof(greetings),0);
  if(n<0) 
  {
    error((char *)"ERROR: Writing to socket\n");
  }
  message greetingsBack;
  n = recv(me.sockfd,&greetingsBack,sizeof(message),0);
  
  if(n<0 || greetingsBack.type == REJECT)
  {
    error((char *) "ERROR: Connection could not be established. A client with the same name already exists.\nExiting...\n");
  }
  

}

/*
  mesaj de tip listclients
*/
void listclients()
{
message msg;
msg.type = LISTCLIENTS;
int n = send(me.sockfd,&msg,sizeof(msg),0);
if(n<0)
{
  error((char *)"ERROR: In client. Writing to socket. \nExiting ..\n");
}

}

/*
  clientul anunta serverul ca intrerupe conexiunea
*/
void quit()
{
message msg ;
msg.type = QUIT;
int n = send(me.sockfd,&msg,sizeof(message),0);
if(n<0)
{
    error((char *) "ERROR:In client, Writing to socket.\nExiting ...\n");
}
}

/*
  clientul trimite serverului mesaj in care cere informatii leagte de un client 
  cu numele client_name
  
*/
void infoclient(char client_name[100])
{
 message msg ;
 msg.type = 2;
 strcpy(msg.payload, "\0" );
 memcpy(msg.payload,client_name, strlen(client_name));
 int n = send(me.sockfd,&msg,sizeof(msg),0);
 if(n<0)
 {
  error((char *) "ERROR:In client. Writing to socket.\n Exiting ...\n");
 }   
    
}
/*
  clientul trimite un mesaj in acre cere serverului date de conectare catre 
  un alt client
*/
void send_message(char client_name[100],vector<string>elem)
{
  string str="";
  for(int i=2;i<elem.size();i++) str+=elem.at(i);
  qmsg.push(str);
  message msg;
  msg.type = MESSAGE;
  strcpy(msg.payload, "\0" );
  memcpy(msg.payload,client_name, strlen(client_name));
  int n = send(me.sockfd,&msg,sizeof(msg),0);
  if(n<0)
 {
  error((char *) "ERROR:In client. Writing to socket.\n Exiting ...\n");
 }   
  

}
/*
  clientul trimite un emsaj prin care anunta ca partajeaza un nou fisier
*/
void sharefile(char * name)
{
  message request;
  request.type = SHAREFILE;
  strcpy(request.payload,name);

  int n = send(me.sockfd,&request,sizeof(request),0);
  
  if(n<0)
  {
    error((char *) "ERROR:In client. Writing to socket.\n Exiting ...\n");
  }

}

/*
  clientul trimite un mesaj prin care anunta ca un fisier
   trebuie sters din lista fisierelor partajate
*/
void unsharefile(char * filename)
{   
    message request;
    request.type = UNSHAREFILE; 
    strcpy(request.payload,filename);
    int n = send(me.sockfd,&request,sizeof(request),0);
    if(n<0)
    {
      error((char *) "ERROR:In client. Writing to socket.\n Exiting ...\n");
    }
      
    
}

void getshare(char str_client[100])
{
  message request ;
  request.type = GETSHARE;
  strcpy(request.payload,str_client);
  int n = send(me.sockfd,&request,sizeof(request),0);
  if(n<0)
    {
      error((char *) "ERROR:In client. Writing to socket.\n Exiting ...\n");
    }
     
}

/*
  clientul trimite un mesaj catre server prin care anunta ca 
  vrea sa transfere un fisier de la alt client
*/
void getfile(char clientname[100], char filename[100])
{

qfiles.push(string(filename));
  message msg;
  msg.type = GETFILE;
  strcpy(msg.payload, "\0" );
  memcpy(msg.payload,clientname, strlen(clientname));
  int n = send(me.sockfd,&msg,sizeof(msg),0);
  if(n<0)
 {
  error((char *) "ERROR:In client. Writing to socket.\n Exiting ...\n");
 }   
  

}

/*  clientul a primit un pachet cu continut de date 
=> scrie in fisier
*/
void receivedfile(message response, FILE *f)
{
// FILE * f = fopen(file.c_str(),"w");
   char buffer[LEN];
   int count;
  fwrite (response.payload,1,LEN,f);
  
}

/*
Main
*/
int main(int argc, char *argv[])
{

    struct sockaddr_in serv_addr,serv_addr2;
    struct hostent *server;

    char buffer[BUFLEN];
    if (argc < 4) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }  
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    me.name=string(argv[1]);
    //golire multime descriptori de fisiere
   FD_ZERO(&read_fds);
   FD_ZERO(&tmp_fds);
   //socket conectare server
	me.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (me.sockfd < 0) 
        error((char *)"ERROR opening socket");
    //adauga desciptorul in multimea read_fds
    FD_SET(me.sockfd,&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    inet_aton(argv[2], &serv_addr.sin_addr);
    
    //socket pt ascultatre
        me.listensockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset((char *) &serv_addr2, 0, sizeof(serv_addr2));
     serv_addr2.sin_family = AF_INET;
     serv_addr2.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
     serv_addr2.sin_port = 0;
     
    
    if (me.listensockfd < 0) 
        error((char *)"ERROR opening socket");
    if(bind(me.listensockfd,(struct sockaddr *)&serv_addr2,sizeof(struct sockaddr))<0)cout<<"Eroare in bind.";
     listen(me.listensockfd, 2);
    FD_SET(me.listensockfd,&read_fds);
    fdmax = me.listensockfd;
    if (connect(me.sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        {
        
        error((char *)"ERROR connecting");    
        exit(-1);
        }
    
    //trimitere mesaj cu numele clientului     
    greetings(argv[1]);   
    if(fdmax < me.sockfd)fdmax = me.sockfd;
    while(!finished){
    	tmp_fds = read_fds;
    	if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
    		error((char *)"ERROR in select - client");
    	int i;
    	for (i = 0; i <= fdmax; i++){
    		if (FD_ISSET(i, &tmp_fds)){
    		// am primit un mesaj de la stdin
    			if( i == STDIN_FILENO ) {
  					//citesc de la tastatura
            int n =read(STDIN_FILENO,buffer,LEN);
            buffer[n-1] = '\0';
    			  //comanda de tip listclients
			      if(strcmp (buffer,"listclients") == 0)
			      {
			        listclients();   
			        
            }
            //comanda de tip quit
            if(strcmp(buffer, "quit") == 0)
            {
                cout<<"Quiting ...\n";
                quit();
                
                finished = true;
            }
            stringstream ss(buffer);
  					string str;
  					vector<string> elem;
  					while(getline(ss, str, ' '))elem.push_back(str);	
  					
  					//comanda de tip infoclient
            if(elem.at(0)=="infoclient")
            {
              infoclient((char *)elem.at(1).c_str());
              cout<<"infoclient"<<endl;
            }
            
            //comanda de tip message
            if(elem.at(0)=="message")
            {
               send_message((char *)elem.at(1).c_str(),elem);
            }
            
            //comanda de tip sharefile
            if(elem.at(0)=="sharefile")
            { 
                
                sharefile((char *)elem.at(1).c_str());
            }
            
             //comanda de tp unsharefile 
            if(elem.at(0)=="unsharefile")
            { 
                cout<<"hello";
                unsharefile((char *)elem.at(1).c_str());
            
            }
            
            //comanda de tip getsharefile
            if(elem.at(0)=="getsharefile")
            {
                getshare((char *)elem.at(1).c_str());
            }
            
            //comanda de tip getfile
            if(elem.at(0)=="getfile")
            {
                getfile((char *)elem.at(1).c_str(),(char *)elem.at(2).c_str());
            }
            
            //comanda de tip quit
            if(elem.at(0) == "quit")
            {
              quit();           
              
            }
                      			  	
				}
				else {
				// am primit mesaj pe socketul de ascultare
				if(i == me.listensockfd)
				{
				    struct sockaddr_in con_addr;
				    int newsock;
				    socklen_t len=sizeof(con_addr);
				    if((newsock = accept(i,(struct sockaddr *)&con_addr,&len))==-1)
                {
                  perror("ERROR: In client. Accept new connection...\n");
                  exit(1);
                }
             else
             {
                  FD_SET(newsock,&read_fds);
                  fdmax = (fdmax>newsock)?fdmax:newsock;
                  FD_CLR(i,&read_fds);
                  close(i);
                
              }   
  				
        }
        else
        {

				message response;
					memset(buffer, 0, BUFLEN);
					int n;
					if ((n = recv(i, &response, sizeof(response), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("server %d hung up\n", i);
						} 
						else 
							error((char *)"ERROR in recv");
						
						close(i); 
						FD_CLR(i, &read_fds);
					}
					else
					{
					// am primit masaj de la un client
					
					if(i!=me.sockfd)
					{
					// de tip message =>afisez mesaj
					if(response.type == MESSAGE)
					{
					  cout<<"Am primit mesajul "<<response.payload<<endl;
          }
          //mesaj care anunta inceputul transferului unui fisier
                if(response.type == BEGIN)
            {
              FILE * f = fopen(strcat(response.payload,"_primit"),"r");
              qFILE.push(f);           
              
             }
             //mesaj care anunta sfarsitul transferului unui fisier
               if(response.type == END)
            {
              FILE *f = qFILE.front();
              fclose(f);qFILE.pop();
                 finished = true;           
              
             }
            // un alt client a cerut un fisier de la client
            if(response.type ==GETFILE)
            {
                sendfile(response);
             }
          
          }
         //mesaje primite de la server 
         if(i==me.sockfd)
          {
            //de tip infoclient=>afisez detaliile clientului
            if(response.type == INFOCLIENT)
            {
           
              cout<<"\nClient name: "<<response.payload;
              
             } 
             // am primit un mesaj de tip quit de la server=>ies din while
             if(response.type == REJECT)
            {
                 finished = true;           
              
             }
              // am primit mesaj de listaer clienti
             if(response.type == LISTCLIENTS)
            {
                
                cout<<"The list of all clients is: "<<response.payload<<endl;    
                      
             }
             //lista fisierelor partajate de un client
             if(response.type ==GETSHARE)
            {
                
                cout<<"The list of shared files is: "<<response.payload<<endl;    
                      
             }
             //mesaj de la server cu date de conectare catre alt client
             if(response.type ==MESSAGE)
            {
                cout<<response.type<<endl;
                messageback(response);
                cout<<"hello";
             }
          
					}
					}
				}
				}
			}
   		 }
    }
    return 0;
}
