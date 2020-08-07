#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <map> 
#include <iterator> 
#include <pthread.h>

using namespace std; 


void *Worker(void *threadid);
int isFull();
void enQueue(int element);
int isEmpty();
int deQueue();
char **tokenize(char *line);



pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

#define SIZE 10000
#define jobs 10

#define MAX_INPUT_SIZE 5000
#define MAX_TOKEN_SIZE 5000
#define MAX_NUM_TOKENS 5000
#define THREAD 1000

int items[SIZE];
int front = -1, rear =-1;
int fill_flag=1;
int count=0;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
map <int,string>KVpair;
int server_fd, new_socket, valread; 
struct sockaddr_in address; 
int opt = 1; 
int addrlen = sizeof(address); 

//add request to queue
void *generate_requests_loop(void *data)
{
  int ret;
  int thread_id = *((int *)data);
  pthread_t worker[THREAD];
    int rc;
      for(int i=0;i<THREAD;i++){
      rc = pthread_create(&worker[i], NULL, Worker, &i);
      if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }
      }


  while(1)
    {   int val;
        // printf("server is listening\n");
        val = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
      pthread_mutex_lock(&lock);
      if(!isFull()){
         enQueue(val);
         pthread_cond_signal(&empty);
        //  printf("connected %d\n",val);
      }
      else{
          pthread_cond_wait(&full,&lock);
      }
      pthread_mutex_unlock(&lock);
    }
}

void erase(int itr)
{
    //printf("delete the value");
}


void *Worker(void *threadid)
{ 
  while(1){
      map <int, string> :: iterator itr;
      int new_socket;
      int entry=0;
    char  line[MAX_INPUT_SIZE];            
      char  **tokens;              
      pthread_mutex_lock(&lock);
      if(!isEmpty()){
          new_socket = deQueue();
          if(new_socket>=4){
         //printf("processed request %d\n",new_socket);
          entry=1;
          pthread_cond_signal(&full);    
          }
        }
        else{
            pthread_cond_wait(&empty, &lock);
        }
     pthread_mutex_unlock(&lock);
     while(entry)
     {
         char buffer[5000] = {0};
     valread = read(new_socket,buffer,5000);
      //printf("%s",buffer);
     tokens = tokenize(buffer);
     //printf("%s\n",tokens[1]);
     int switchvalue=0;
      if (strcmp(tokens[0], "disconnect") == 0)
            switchvalue = 2;
        if (strcmp(tokens[0], "create") == 0)
            switchvalue = 3;
        if (strcmp(tokens[0], "read") == 0)
            switchvalue = 4;
        if (strcmp(tokens[0], "update") == 0)
            switchvalue = 5;
        if (strcmp(tokens[0], "delete") == 0)
            switchvalue = 6;


        switch (switchvalue) {
            case 2:{
              switchvalue=0;
              write(new_socket , "closed at server\n", strlen("closed at server\n"));
              close(new_socket);
            //   printf("Connection closed\n");
              entry=0;
              break;
            }
            case 3:
            {
               // printf("increate\n");
              switchvalue=0;
              int key;
          sscanf(tokens[1],"%d",&key);
              itr = KVpair.find(key);
              if(itr==KVpair.end()){
                KVpair.erase(key);
              string p;
              for(int i=3;tokens[i]!=NULL;i++){
            p=p+tokens[i];
            p=p+' ';
          }
              KVpair.insert(pair <int, string> (key, p));
              write(new_socket , "inserted", strlen("inserted")); 
            //   printf("Value inserted\n");
            }
              else
              {
            //   printf("exists\n");
              write(new_socket , "exists", strlen("exists"));
              } 
                break;
              

            }
            case 4:{
              switchvalue=0;
              int findkey;
              sscanf(tokens[1],"%d",&findkey);
              itr= KVpair.find(findkey);
              if(itr!=KVpair.end())
              {
              int n = itr->second.length();  
              char char_array[n+1];  
              strcpy(char_array, itr->second.c_str());
              write(new_socket , char_array, strlen(char_array));
              }
              else
              {
            //   printf("NOT FOUND\n");
              write(new_socket , "NOT FOUND", strlen("NOT FOUND"));
              }
              break;
                }
            case 5:{
                switchvalue=0;
              int findkey;
          sscanf(tokens[1],"%d",&findkey);
              itr = KVpair.find(findkey);
              if(itr!=KVpair.end()){
               try{
            erase(findkey);
                  }
            catch (const std::out_of_range& oor){
                int b;
            }
              string q;
              for(int i=3;tokens[i]!=NULL;i++){
            q=q+tokens[i];
            q=q+' ';
          }
              KVpair.insert(pair <int, string> (findkey, q));
              write(new_socket , "UPDATED", strlen("UPDATED")); 
            //   printf("Value updated\n");
            }
              else
              {
            //   printf("NOT FOUND\n");
              write(new_socket , "NOT FOUND", strlen("NOT FOUND"));
              } 
                break;
                  }
            case 6:{
              switchvalue=0;
              int findkey;
          sscanf(tokens[1],"%d",&findkey);
              itr = KVpair.find(findkey);
              if(itr!=KVpair.end()){
                  try{
                        erase(findkey);
                  }
            catch (const std::out_of_range& oor){
                int a;
            }
              write(new_socket , "DELETED", strlen("DELETED")); 
            //   printf("Value deleted\n");
        }
        else
        {
        // printf("NOT FOUND\n");
        write(new_socket , "NOT FOUND", strlen("NOT FOUND")); 
        }
              break;
            }
    }
        int i=0;
        while(tokens[i]!=NULL){
        free(tokens[i]);
        i++;
        }
        if(tokens!=NULL)
        free(tokens);
    }
}
}


int main(int argc, char *argv[])
{
  int prod_thread_id = 0;
  pthread_t prod_thread;

if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr(argv[1]); 
    int port;
    sscanf(argv[2],"%d",&port);
    address.sin_port = htons(port); 
       
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 10000000) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }


  //create master thread
  pthread_create(&prod_thread, NULL, generate_requests_loop, (void *)&prod_thread_id);
  pthread_join(prod_thread,NULL);
}



int isFull()
{
    if( (front == rear + 1) || (front == 0 && rear == SIZE-1)) return 1;
    return 0;
}

int isEmpty()
{
    if(front == -1) return 1;
    return 0;
}

void enQueue(int element)
{
    if(isFull())
     printf("\n");
    else
    {
        if(front == -1) front = 0;
        rear = (rear + 1) % SIZE;
        items[rear] = element;
    }
}


int deQueue()
{
    int element;
    if(isEmpty()) {
        return(-1);
    } else {
        element = items[front];
        if (front == rear){
            front = -1;
            rear = -1;
        }
        else {
            front = (front + 1) % SIZE;
            
        }
       return(element);
    }
}


char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
    tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(tokens[tokenNo++], token);
    tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}
