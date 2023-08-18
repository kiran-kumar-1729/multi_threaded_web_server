
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <bits/stdc++.h>
using namespace std;
queue<int> requests;
pthread_mutex_t mutex1=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t req_available = PTHREAD_COND_INITIALIZER;
int thread_pool_size=10;//(int)(thread::hardware_concurrency());
pthread_t thread_pool[10];
void error(char* msg) {
  perror(msg);
  exit(1);
}
vector<string> split(const string &s, char delim) {
  vector<string> elems;

  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}
void signal_handler(int sig_num){
	exit(0);
}
void handle_request(int socket){
  char buffer[8192];
  //string buffer
  int newsockfd=socket;
  //printf("socket id in thread : %d\n",newsockfd);
  int n;
  string HTTP_version;
  string method;
  string url;
  string status_code; 
  string status_text; 
  string content_type;
  string content_length;
  string body;
  //while(1){
  bzero(buffer, 8192);
  //memset(buffer,0,8191);
  n = read(newsockfd, buffer, 8191);
  //printf("read : %d\n",n);
  string s="";
  if (n < 0){
    char err[]="ERROR reading from socket";
    //printf("here is the problem\n");
    //close(newsockfd);
    error(err);
  }
  //printf("success read: %d\n",n);
  else if(n==0){          // additional ones
     close(newsockfd);
     return;
  }
  else{
  //printf("Here is the message: \n%s\n", buffer);
  //printf("received string: \n%s\n", s);
  for(int i=0;i<=8191;i++){
    if(buffer[i]=='\0')
    break;
    s=s+buffer[i];
  }
  //cout<<s<<endl;
  vector<string> lines = split(s, '\n');
  vector<string> first_line = split(lines[0], ' ');
  method=first_line[0];
  url=string("html_files") + first_line[1];
  //cout<<"req url : "<<url<<endl;
  string cmp="html_files/favicon.ico";
  if(0){
    //pass
  }
  else{
  HTTP_version="HTTP/1.0";
   if(method != "GET") {
    cerr << "Method '" << method << "' not supported" << endl;
    exit(1);
  }
  struct stat sb;
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    status_code = "200";
    status_text = "OK";
    content_type = "text/html";
    //cout<<"code is working successfully"<<endl;
    if (S_ISDIR(sb.st_mode)) {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
      url=url+"/index.html";
    }
    ifstream fin;
    string line;
    body="";
    fin.open(url);
    while(fin){
        getline(fin,line);
        body+=line+"\n";
    }
    fin.close();
    int length=strlen(body.c_str());
    content_length=to_string(length);
    time_t t;
    struct tm * tt; 
    time (&t);
    tt = localtime(&t);
    //string msg=HTTP_version+" "+status_code+" "+status_text+"\n"+"Date: "+asctime(tt)+"\n"+"content-type: "+content_type+"\n"+"content_length: "+content_length+"\n"+"  "+body;
    string msg=HTTP_version+" "+status_code+" "+status_text+"\r\n"+"Content-Length: "+content_length+"\r\n"+"Content-Type: "+content_type+"\r\n"+"Connection: Closed\r\n"+"Date: "+asctime(tt)+"\r\n\r\n"+body;
    /*
    TODO : open the file and read its contents
    */

    /*
    TODO : set the remaining fields of response appropriately
    */
    //cout<<msg<<endl;
    bzero(buffer, 8192);
    strcpy(buffer,msg.c_str());
  }

  else {
    status_code = "404";
    content_type="text/html";
    ifstream fin;
    body="";
    body=body+"<html>\n"+"<head>\n"+" <title>Not Found</title>\n"+"</head>\n"+"<body>\n"+" <h1> PAGE NOT FOUND HERE.</h1>\n"+"</body>\n"+"</html>";
    content_length=to_string(body.size());
    string msg=HTTP_version+" "+status_code+" Not Found\n"+"Content-Length: "+content_length+"\n"+"Connecetion: Closed\n"+"Content-Type: "+content_type+"\n"+"\n"+body;
    strcpy(buffer,msg.c_str());

    /*
    TODO : set the remaining fields of response appropriately
    */
  }
  n=write(newsockfd,buffer,8191);
  //printf("success write: %d\n",n);
  close(newsockfd);
  if (n < 0){
    char err[]="ERROR writing to socket";
    error(err);
  }
  //break;
      // additional ones
    //close(newsockfd)
  //}
  }
  //pthread_join(newsockfd,NULL);
  }
}
void *thread_func(void *args){
      int f=0;
      while(true){
        int socket;
        pthread_mutex_lock(&mutex1);
        while(requests.empty()){
          pthread_cond_wait(&req_available,&mutex1);
        }
        socket=requests.front();
        requests.pop();
        pthread_mutex_unlock(&mutex1);
        handle_request(socket);
      }
}
int main(int argc, char *argv[]) {
  signal(SIGINT, signal_handler);
  for(int i=0;i<thread_pool_size;i++){
    pthread_create(&thread_pool[i],NULL,thread_func,NULL);
  }
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  //char buffer[256];
  string buffer;
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    char err[]="ERROR opening socket";
    error(err);
  }

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    char err[]="ERROR on binding";
    error(err);
  }
  /* listen for incoming connection requests */
  listen(sockfd, 100);
  clilen = sizeof(cli_addr);
  /* accept a new request, create a newsockfd */
  //pthread_t thread_id[100];
  pthread_t thread_id;
  int count=-1;
  while(1){

  newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
  if (newsockfd < 0){
    char err[]="ERROR on accept";
    error(err);
  }
  else{
    pthread_mutex_lock(&mutex1);
    requests.push(newsockfd);
    pthread_mutex_unlock(&mutex1);
    pthread_cond_signal(&req_available);
  }
  }
  //for(int i=0;i<thread_pool_size;i++){
    //pthread_exit(thread_pool[i],NULL);
  //}
  return 0;
}
