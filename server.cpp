#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<time.h>
#include<string>
#include<vector>
#include<sstream>
#include<iostream>
#include "helpers.h"
#include<queue>
#define MAX_CLIENTS 5

bool finished ;
vector<client> clients;
//multimea de citire folosita in select()
fd_set read_fds;
//multime folosita temporar 	
fd_set tmp_fds;	
int fdmax;		
//coada de mesaje
queue<int> qmsg;

void error(char *msg)
{
    perror(msg);
    exit(1);
}


/*
  mesaj primit de la un clienti in care cere 
  lista clientilor conectati
*/
void listclients(int sock)
{
  vector<client>::iterator iter;
  message response;  
    response.type = LISTCLIENTS;
    char str[1000];
    strcpy(str, "\0");
  for(iter=clients.begin();iter<clients.end();iter++)
  {
      strcat(str,(*iter).name.c_str());
      strcat(str, " ");
  }
  strcpy(response.payload,str);
  int n =  send(sock,&response,sizeof(response),0);
  if(n<0)
  {
    error((char *)"ERROR: In server.Writing to socket. Exiting ...\n");
  }
  
}

/*
  mesaj primit de la client in acre anunta partajarea unui fisier
*/
void sharefile(int sock, message msg)
{
  char filename[100];
  strcpy(filename,msg.payload);
  vector<client>::iterator iter;  
  for(iter=clients.begin();iter<clients.end();iter++)
  {
      if((*iter).sockfd == sock)
      {
            vector<string>::iterator iter2;  
            for(iter2=(*iter).files.begin();iter2<(*iter).files.end();iter2++)
            {
                if((*iter2).compare(string(filename)) == 0 ) 
                {
                return;
                }
              
            }
            clients.at(iter-clients.begin() ).files.push_back(string(filename));
            cout<<"The file "<<filename<<"has been succesfully shared by "<<clients.at(iter-clients.begin()).name.c_str()<<endl;
            
            
      }
  }

}
/*
  mesaj primit de la client in acre anunta stergerea unui fisier dinn lista 
  fisierelor partajate
  */
void unsharefile(int sock, message msg)
{
    char filename[100];
  strcpy(filename,msg.payload);
  vector<client>::iterator iter;  
  for(iter=clients.begin();iter<clients.end();iter++)
  {
      if((*iter).sockfd == sock)
      {
            vector<string>::iterator iter2;  
            for(iter2=(*iter).files.begin();iter2<(*iter).files.end();iter2++)
            {
                if((*iter2).compare(string(filename)) == 0 ) 
                {
                   clients.at(iter-clients.begin()).files.erase(iter2);
                   cout<<"The file "<<filename<<"has been successfully unshared by "<<clients.at(iter-clients.begin()).name.c_str()<<endl;
                   return ; 
                }
              
            }
            cout<<"The file has not been shared by the client yet.\n";
            return ;

            
            
      }
}}

/*
serverul trimite ACCEPT sau REJECT unui client nou conectat
*/
void nice2meetU(string name, int sock)
{
  vector<client>::iterator iter;
  message msg;  
  for(iter=clients.begin();iter<clients.end();iter++)
  {
    if(clients.at((int)(iter - clients.begin())).name.compare(name) == 0)
    {
      msg.type = REJECT;
      send(sock,&msg,sizeof(msg),0);
      clients.erase(iter);
      FD_CLR(clients.at((int)(iter - clients.begin())).sockfd,&read_fds);
      close(clients.at((int)(iter - clients.begin())).sockfd);
      return ;
      
    }
    else     
      if(clients.at((int)(iter - clients.begin())).sockfd == sock)
      {
         clients.at((int)(iter - clients.begin())).name = name; 
         msg.type = ACCEPT;
         int n = send(sock,&msg,sizeof(msg),0);
         if(n<0) error((char *)"ERROR: In server. Writing to socket.\nExiting ... \n");
         printf("SUCCESS: Connection with client %s has been established.\n",name.c_str());
         cout<<clients.size()<<endl;
        return ;
      }
   }

}

/*
un client a cerut informatii despre alt client
*/
void infoclient(message msg , int sock)
{ 

  char name[100];strcpy(name,msg.payload);
  cout<<"Am primit cerere infoclient cu date pt clientul "<<name<<"\n";
  int i;


  vector<client>::iterator iter;
  for(iter=clients.begin();iter<clients.end();iter++)
  { 
      if(strcmp(name, clients.at((int)(iter - clients.begin())).name.c_str()) == 0)
      {
        message response ;
        char data[300];
        response.type = INFOCLIENT;
        strcpy(data, "Name: ");
        strcat(data,clients.at((int)(iter - clients.begin())).name.c_str());
        strcat(data, "\nPort: ");
       char str[100];
       sprintf(str, "%d",clients.at((int)(iter - clients.begin())).port);
        strcat(data,str);
       strcat(data, "\nConnected for: ");
        sprintf(str,"%ld",(time(NULL)-clients.at((int)(iter - clients.begin()))._time));
        strcat(data,str);
        strcat(data,"\r\n");
        strcpy(response.payload, data);      
        
        int n = send(sock,&response,sizeof(response),0);
        if(n<0)
        {
          error((char *)"ERROR: In Server. Writing to socket.\n");
        }
        break;
      } 
      
  }
}

/*
  trimit mesaj cu datele de conectare catre alt client
  pt mesaje
*/
void send_message(int sock, message msg)
{
    message response;
//    for(int i=0;i<)
cout<<"response\n";
   pair<string,string>p;
    char buffer[LEN];
    memset(buffer,'\0',LEN);
    response.type = MESSAGE;
  vector<client>::iterator iter;
  for(iter=clients.begin();iter<clients.end();iter++)
  { 
    //  cout<<clients.at((int)(iter - clients.begin())).name.c_str();
       if(strcmp(msg.payload, clients.at((int)(iter - clients.begin())).name.c_str()) == 0)
      {
            stringstream ss;
            ss<<clients.at((int)(iter - clients.begin())).port;
            p.first = clients.at((int)(iter - clients.begin())).IP;
            p.second = ss.str();
            cout<<p.first.c_str()<<"\n";
            cout<<p.second.c_str()<<"\n";
            strcpy(buffer,clients.at((int)(iter - clients.begin())).IP.c_str());
            strcat(buffer," ");
            strcat(buffer,ss.str().c_str());
            memset(response.payload,'\0',LEN);
            memcpy(response.payload,buffer,LEN);
            cout<<sizeof(response.payload)<<endl;
          int n = send(sock,&response,sizeof(response),0);
           if(n<0)
        {
          error((char *)"ERROR: In Server. Writing to socket.\n");
        }

        break;
      }
  
  }
}
/*
  trimit mesaj cu datele de conectare catre alt client
  pt transfer de  fisiere
*/

void send_file(int sock, message msg)
{
    message response;
//    for(int i=0;i<)
cout<<"response";
    pair<string,string>p;
    response.type = GETFILE;
  vector<client>::iterator iter;
  for(iter=clients.begin();iter<clients.end();iter++)
  { 
    //  cout<<clients.at((int)(iter - clients.begin())).name.c_str();
       if(strcmp(msg.payload, clients.at((int)(iter - clients.begin())).name.c_str()) == 0)
      {
            stringstream ss;
            ss<<clients.at((int)(iter - clients.begin())).port;
            p.first = clients.at((int)(iter - clients.begin())).IP;
            p.second = ss.str();
            memset(response.payload,'\0',LEN);
            memcpy(response.payload,&p,sizeof(p));
          int n = send(clients.at((int)(iter - clients.begin())).sockfd,&response,sizeof(response),0);
           if(n<0)
        {
          error((char *)"ERROR: In Server. Writing to socket.\n");
        }

        break;
      }
  
  }
}

/*
  mesaj de tip status primit la stdin
*/
void status()
{
  cout<<"STATUS\n";
  if(clients.size() == 0) cout<<"No clients\n";
  else
  {
   vector<client>::iterator iter;
  for(iter=clients.begin();iter<clients.end();iter++)
  {
    cout<<"\nName:\n"<<(*iter).name<<"\nIP:\n"<<(*iter).IP<<"\nPort:\n"<<(*iter).port<<"\nFiles:\n";
    if((*iter).files.size() == 0) cout<<"No files.\n";
    else
    {
       vector<string>::iterator iter2;
       for(iter2=(*iter).files.begin();iter2<(*iter).files.end();iter2++)
        {
            cout<<"\t"<< (*iter2).c_str()<<endl;
        } 
  }
  }
}
}

/*
mesaj de tip quit primit la stdin
*/
void quit()
{
finished = true;
int sock;
message response;
  vector<client>::iterator iter;
  for(iter=clients.begin();iter<clients.end();iter++)
  {
     response.type = REJECT; 
    send((*iter).sockfd,&response,sizeof(response),0);
    close((*iter).sockfd);
  }
}

/*
  mesaj prin care un client anunta ca inchide conexiunea
*/
void byebye(int sock)
{
  vector<client>::iterator iter;
  for(iter=clients.begin();iter<clients.end();iter++)
  {
    if((*iter).sockfd ==  sock)
    {
      cout<<"The client "<<(*iter).name<<" has quited..."<<endl;
      FD_CLR(sock,&read_fds);
      close(sock);
      clients.erase(iter);
    }
  }

}
/*
  serverul a primit un mesaj de request pt lista de fisiere partajate de un anumit client
*/
void getshare(int sock, message msg)
{
  char str_client[100];
  strcpy(str_client,msg.payload);
  vector<client>::iterator iter; 
  char mesaj[1000];
  strcpy(mesaj, "\0"); 
  for(iter=clients.begin();iter<clients.end();iter++)
  {
      if((*iter).name == string(str_client))
      {
            vector<string>::iterator iter2;  
            for(iter2=(*iter).files.begin();iter2<(*iter).files.end();iter2++)
            {
              strcat(mesaj,(*iter2).c_str());
              strcat(mesaj, " , ");
              
            }
            break;
            
            
      }
  }
  message response;
  response.type = GETSHARE;
  strcpy( response.payload, mesaj);
  int n = send(sock,&response,sizeof(response),0);
  if(n<0)
  {
    error((char *)"ERROR: In Server. Writing to socket.\n");
  }
  
  
}
/*
  serverul trimite clientlui un mesaj inapoi cu datele d conectare catre un anumit client
*/
void messageback(int sock,message msg)
{
  vector<client>::iterator iter; 
    client cli;
    memcpy(&cli.port,msg.payload,sizeof(cli));
    for(iter=clients.begin();iter<clients.end();iter++)
    {
        if((*iter).sockfd == sock)
        memcpy(&cli.IP,(*iter).IP.c_str(),strlen((*iter).IP.c_str()));
        break;
     }
     int _socket=qmsg.front();
     qmsg.pop();
     message fwd;
     
     memcpy(fwd.payload,&cli,sizeof(cli));
     int n = send(_socket,&fwd,sizeof(fwd),0);
  if(n<0)
  {
    error((char *)"ERROR: In Server. Writing to socket.\n");
  }
   
}

/*
  main
*/
int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n, i, j;
     int vsocket[100];
     int ind=0;

   
     if (argc < 2) {
         fprintf(stderr,"Usage : %s port\n", argv[0]);
         exit(1);
     }

     //golim multimea de descriptori de citire 
     FD_ZERO(&read_fds);
     FD_ZERO(&tmp_fds);
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error((char *)"ERROR opening socket");
     
     portno = atoi(argv[1]);

     memset((char *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error((char *)"ERROR on binding");
     
     listen(sockfd, MAX_CLIENTS);

     //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
     FD_SET(sockfd, &read_fds);
     fdmax = sockfd;
     FD_SET(STDIN_FILENO, &read_fds);
     finished = false;
	while (!finished) {
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error((char *)"ERROR in select");
	
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
			
				if (i == sockfd) {
					// a venit ceva pe socketul de ascultare = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error((char *)"ERROR in accept");
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
            client newclient;
            newclient.sockfd = newsockfd;
            newclient.port = cli_addr.sin_port;
            newclient.IP = string(inet_ntoa(cli_addr.sin_addr));
            newclient._time = time(NULL);
						clients.push_back(newclient);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					

				}
					
				else {
				// am primit o comanda de la stdin
			    if(i == STDIN_FILENO)
			    {
			      char buffer[100];
			      strcpy(buffer,"\0");
			      int n =read(STDIN_FILENO,buffer,LEN);
			      buffer[n-1]='\0';
			      cout<<buffer;
			      // de tip quit
			      if(strcmp(buffer,"quit") == 0)
			      {
			          quit();
			          
            }
            // de tip status
           if(strcmp(buffer,"status") == 0)
			      {   
			          status();
            }
          }
          else
          {  
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()

					message newmsg;
					if ((n = recv(i, &newmsg, sizeof(message), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							
						} else {
							error((char *)"ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
					} 
					
					else { //recv intoarce >0
						 cout<<newmsg.type;
						printf ("Am primit de la clientul de pe socketul %d, mesajul: %s\n", i, newmsg.payload);
					  
					    // am primit un mesaj in care un nou client anunta ca vrea sa se conecteze
						if(newmsg.type == HELLO)
						{
						    nice2meetU(newmsg.payload,i);
            }
            // am primit un mesaj in care un client cere detalii despre alt client
            if(newmsg.type == INFOCLIENT)
            {
              infoclient(newmsg,i);

            }
            //un client vrea sa se deconecteze
            if(newmsg.type == QUIT)
            {
              
                byebye(i);
            }
            //un client vrea listad e clienti disponibili
            if(newmsg.type == LISTCLIENTS)
            {
                listclients(i);    
            }
            //un client vrea sa partajeze un fisier
             if(newmsg.type == SHAREFILE)
            {
                sharefile(i,newmsg);    
            }
              //un client vrea sa stearga unn fisier din lista de fisiere partajate
              if(newmsg.type == UNSHAREFILE)
            {
                unsharefile(i,newmsg);    
            }
            // un client vreas lista de fisiere partajate de altul
            if(newmsg.type == GETSHARE)
            {
                getshare(i,newmsg);    
            }
            //mesaj in care un client cere datele de conecatre cu un alt client
            if(newmsg.type == MESSAGE)
            {
                cout<<"primit\n";
              send_message(i,newmsg); 
            }
             //  un client vrea sa transfere un fisier catre alt client 
             if(newmsg.type == GETFILE)
            {
                send_file(i,newmsg); 
            }
					}
					}
				} 
			}
		}
     }


     close(sockfd);
   
     return 0; 
}


