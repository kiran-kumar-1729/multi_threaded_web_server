/* run using: ./load_gen localhost <server port> <number of concurrent users>
   <think time (in s)> <test duration (in s)> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
int time_up;
FILE *log_file;



// user info struct
struct user_info {
  // user id
  int id;

  // socket info
  int portno;
  char *hostname;
  float think_time;

  // user metrics
  int total_count;
  float total_rtt;
};
void waitFor (unsigned int secs) 
{
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

// error handling function
void error(char *msg) {
  perror(msg);
  exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1) {
  return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

// user thread function
void *user_function(void *arg) {
  /* get user info */
  struct user_info *info = (struct user_info *)arg;

  int sockfd, n;
  char buffer[256];
  struct timeval start, end;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  while (1) {
    /* start timer */
    gettimeofday(&start, NULL);

    /* TODO: create socket */
         sockfd = socket(AF_INET, SOCK_STREAM, 0);
         if (sockfd < 0)
            error("ERROR opening socket");
    /* TODO: set server attrs */
         server = gethostbyname(info->hostname);
         if (server == NULL) 
          {
             fprintf(stderr, "ERROR, no such host\n");
             exit(0);
          }
         bzero((char *)&serv_addr, sizeof(serv_addr));
         serv_addr.sin_family = AF_INET;
         bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
         serv_addr.sin_port = htons(info->portno);
    /* TODO: connect to server */
         if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR connecting");
    /* TODO: send message to server */
          // GET /hello.htm HTTP/1.1
          char HTTP_version[50]="HTTP/1.1";
          char method[50]="GET";
          srand(time(NULL));
          int r=rand()%9;
          char url[50];
          if(r==0)
          strcpy(url,"/index.html");
	  else if(r==1)
	  strcpy(url,"/apart1/index.html");
	  else if(r==2)
	  strcpy(url,"/apart1/flat11/index.html");
	  else if(r==3)
	  strcpy(url,"/apart1/flat12/index.html");
	  else if(r==4)
	  strcpy(url,"/apart2/index.html");
	  else if(r==5)
	  strcpy(url,"apart2/flat21/index.html");
	  else if(r==6)
	  strcpy(url,"/apart3/index.html");
	  else if(r==7)
	  strcpy(url,"/apart3/flat31/index.html");
	  else if(r==8)
	  strcpy(url,"/apart3/flat32/index.html");
          char buffer[2048]="";
          //bzero(buffer, 2048);
          strcat(buffer,method);
          strcat(buffer," ");
          strcat(buffer,url);
          strcat(buffer," ");
          strcat(buffer,HTTP_version);
          
           // write something in buffer
          ///gettimeofday(&start, NULL); /// newly added line
          int n = write(sockfd, buffer, strlen(buffer));
          if (n < 0)
            error("ERROR writing to socket");
          bzero(buffer, 2048);
          
    /* TODO: read reply from server */
            n = read(sockfd, buffer, 2048);
           if(n==0)
            {
              break;
            }
           if (n < 0)
            error("ERROR reading from socket");
           //printf("Server response: %s\n", buffer);
           /////printf("got response from server\n");
           //gettimeofday(&end, NULL); /// newly added  line
           //printf("\nrtt time : : %lf\n",time_diff(&end,&start));
           //exit(0);
  
    /* TODO: close socket */
           close(sockfd);
    /* end timer */
    gettimeofday(&end, NULL);

    /* if time up, break */
    if (time_up)
      break;

    /* TODO: update user metrics */
       info->total_count=info->total_count+1;
       info->total_rtt=info->total_rtt+time_diff(&end,&start);
    /* TODO: sleep for think time */
       usleep(info->think_time*1000000);
  }

  /* exit thread */
  fprintf(log_file, "User #%d finished\n", info->id);
  fflush(log_file);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int user_count, portno, test_duration;
  float think_time;
  char *hostname;

  if (argc != 6) {
    fprintf(stderr,
            "Usage: %s <hostname> <server port> <number of concurrent users> "
            "<think time (in s)> <test duration (in s)>\n",
            argv[0]);
    exit(0);
  }

  hostname = argv[1];
  portno = atoi(argv[2]);
  user_count = atoi(argv[3]);
  think_time = atof(argv[4]);
  test_duration = atoi(argv[5]);

  printf("Hostname: %s\n", hostname);
  printf("Port: %d\n", portno);
  printf("User Count: %d\n", user_count);
  printf("Think Time: %f s\n", think_time);
  printf("Test Duration: %d s\n", test_duration);

  /* open log file */
  log_file = fopen("load_gen.log", "w");

  pthread_t threads[user_count];
  struct user_info info[user_count];
  struct timeval start, end;
  
  /* start timer */
  gettimeofday(&start, NULL);
  time_up = 0;
  for (int i = 0; i < user_count; ++i) {
    /* TODO: initialize user info */
    info[i].id=i;
    info[i].think_time=think_time;
    info[i].total_count=0;
    info[i].total_rtt=0;
    info[i].portno=portno;
    info[i].hostname=hostname;

    /* TODO: create user thread */ 
    pthread_create(&threads[i],NULL,user_function,&info[i]);

    fprintf(log_file, "Created thread %d\n", i);
  }

  /* TODO: wait for test duration */
     waitFor(test_duration);
   
  fprintf(log_file, "Woke up\n");

  /* end timer */
  time_up = 1;
  gettimeofday(&end, NULL);

  /* TODO: wait for all threads to finish */
    //printf("user_count = %d\n",user_count);
   for(int j=0;j<user_count;j++)
   {
      void *ret;
      //printf("waiting for thread %d to finish \n",j+1);
      if (pthread_join(threads[j], &ret) != 0)
        {
          perror("pthread_create() error");
          //exit(3);
        }
   }

  /* TODO: print results */
  FILE *fp;
  FILE *fp2;
  fp= fopen("points2.txt", "a+");
  fp2=fopen("points3.txt","a+");
  float res=0;
  int total=0;
  float thp=0;
  for(int k=0;k<user_count;k++)
  {
    //printf("total requests send by a this user number %d is %d and total rtt for this user is %f\n",k+1,info[k].total_count,info[k].total_rtt);
    total+=info[k].total_count;
    res+=info[k].total_rtt;
  }
  res=res/total;
  thp=total/test_duration;
  fprintf(fp,"%d %lf\n",total,res);
  fprintf(fp2,"%d %lf\n",total,thp);
  fclose(fp2);
  fclose(fp);
  /* close log file */
  fclose(log_file);

  return 0;
}
