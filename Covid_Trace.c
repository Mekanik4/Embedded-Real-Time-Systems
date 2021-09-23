#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define NUMBER_OF_ADDRESSESS 1000
#define MAC_LENGTH 17
#define RUNTIME 25920
#define QUEUESIZE 50
#define P 1
#define Q 1

typedef struct{ 
    struct timeval tv;
    char value[MAC_LENGTH];
    int id;
} macaddress;

int delay[3000];
int counter = 0;
int elementsLeft = (RUNTIME*10);
macaddress closeMacs[3000];
macaddress macsNearME[30000];
char ** addressess;
int i=0, j=0;

struct timeval tic(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

double toc(struct timeval begin){
    struct timeval end;
    gettimeofday(&end, NULL);
    double stime = ((double)(end.tv_sec - begin.tv_sec) * 1000) +
                   ((double)(end.tv_usec - begin.tv_usec) / 1000);
    stime = stime / 1000;
    return (stime);
}

void *producer(void *args);
void *consumer(void *args);
void *timeCounter();


typedef struct{ 
    struct timeval tv;
    int arguement;
   
} element;

typedef struct {
    macaddress *(*work)(char **);
    void *arg;
} workFunction;

typedef struct
{
    workFunction *buf[QUEUESIZE];
    long head, tail;
    int full, empty;
    pthread_mutex_t *mut;
    pthread_cond_t *notFull, *notEmpty;
} queue;

char ** mac_produce();
bool testCOVID();
void writeFile(char **a, char *n, int limit);
void writeBinaryFile(int *array, char *n);
macaddress *BTnearMe(char **array);
void fun(macaddress *mac);
queue *queueInit(void);
void queueDelete(queue *q);
void queueAdd(queue *q, workFunction *in);
void queueDel(queue *q, workFunction *out);

struct timeval start; 
struct timeval hours; 

pthread_cond_t cond;                                                            
pthread_mutex_t mutex;

int main()
{
    addressess =  mac_produce();
    queue *fifo;
    time_t t;
    srand((unsigned) time(&t));
    pthread_t pro[P], con[Q], timeThread;
    start = tic();
    hours = tic();
    fifo = queueInit();
    if (fifo == NULL)
    {
        fprintf(stderr, "main: Queue Init failed.\n");
        exit(1);
    }
    if (pthread_mutex_init(&mutex, NULL) != 0) {                                  
      perror("pthread_mutex_init() error");                                       
      exit(1);                                                                    
    }                                                                             
                                                                                
    if (pthread_cond_init(&cond, NULL) != 0) {                                    
      perror("pthread_cond_init() error");                                        
      exit(2);                                                                    
    }            
    pthread_create(&timeThread, NULL, timeCounter, NULL);
    for (int k = 0; k < P; k++)
        pthread_create(&pro[k], NULL, producer, fifo);
    for (int k = 0; k < Q; k++)
        pthread_create(&con[k], NULL, consumer, fifo);
    for (int k = 0; k < P; k++)
        pthread_join(pro[k], NULL);
    for (int k = 0; k < Q; k++)
        pthread_join(con[k], NULL);
    pthread_join(timeThread, NULL);
    queueDelete(fifo);
    
    return 0;
}

void *timeCounter(){
      while(1){
        //wait 0.1 seconds to send the signal to the consumer
        usleep(100000);
        //send the signal
        if (pthread_cond_signal(&cond) != 0) {                                        
          perror("pthread_cond_signal() error");                                      
          exit(4);                                                                    
        }  
      }
  
}

void *producer(void *q)
{
    queue *fifo;
    int k;
    int *a = (int *)malloc(RUNTIME*10 * sizeof(int));
    for (k = 0; k < (RUNTIME*10); k++){
         a[k] = k;
    }

    workFunction *po;
    po=(workFunction*)malloc(RUNTIME*10 * sizeof (workFunction));

    element *el;
    el=(element *)malloc(RUNTIME*10 * sizeof(element));

    fifo = (queue *)q;
    
    for (k = 0; ; k++)
    {
        (el+k)->arguement = a[counter];
        (po+k)->arg = (el+k);
        (po+k)->work=BTnearMe;
       
        pthread_mutex_lock(fifo->mut);
        
        while (fifo->full)
        {
           // printf("producer: queue FULL.\n");
            pthread_cond_wait(fifo->notFull, fifo->mut);
        }

        queueAdd(fifo, (po+i));
        counter++;
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notEmpty);
        double final = toc(start);

        if (final > RUNTIME){
          printf("The time has passed!\n");
        }

        
    }
    return (NULL);
}
int ctr=0;
int del=0;

void *consumer(void *q)
{
    queue *fifo;
    fifo = (queue *)q;
    double prevTime=0.0;
    
    while (elementsLeft > (Q-1))
    {
        pthread_mutex_lock(fifo->mut);
        while (fifo->empty)
        {
            //printf("consumer: queue EMPTY.\n");
            pthread_cond_wait(fifo->notEmpty, fifo->mut);
            if(elementsLeft<0){
                exit(0);
            }
        }
       
        workFunction d;
        element *e;
        
        queueDel(fifo, &d);
        elementsLeft--;
        e=d.arg;
        
        macaddress *mac = (*d.work)(addressess);
        double final = toc(start);  
        mac->id = ctr;
        
        fun(mac);

    //calculate the delay and add it in an array
        int num = (final - prevTime)*1000000;
        delay[del] = ((((num/10)%10)*10)+(num%10)+(((num/100)%10)*100));
        ctr++;
        del++;
        prevTime = final;
        if (del==3000){
          writeBinaryFile(delay,"Latency.bin");
          del=0;
        }
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notFull);

       //wait for the timer thread to send a signal to proceed
        if (pthread_cond_wait(&cond, &mutex) != 0) {                                  
          perror("pthread_cond_timedwait() error");                                   
          exit(7);                                                                    
        } 
    }

    return (NULL);
}

queue *queueInit(void)
{
    queue *q;

    q = (queue *)malloc(sizeof(queue));
    if (q == NULL)
        return (NULL);

    q->empty = 1;
    q->full = 0;
    q->head = 0;
    q->tail = 0;
    q->mut = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(q->mut, NULL);
    q->notFull = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notFull, NULL);
    q->notEmpty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notEmpty, NULL);

    return (q);
}

void queueDelete(queue *q)
{
    pthread_mutex_destroy(q->mut);
    free(q->mut);
    pthread_cond_destroy(q->notFull);
    free(q->notFull);
    pthread_cond_destroy(q->notEmpty);
    free(q->notEmpty);
    free(q);
}

void queueAdd(queue *q, workFunction *in)
{
    q->buf[q->tail] = in;
    q->tail++;
    if (q->tail == QUEUESIZE)
        q->tail = 0;
    if (q->tail == q->head)
        q->full = 1;
    q->empty = 0;

    return;
}

void queueDel(queue *q, workFunction *out)
{
    *out = *q->buf[q->head];
    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;

    return;
}

//This function returns a random mac address from the MAC_Addressess.txt file
macaddress *BTnearMe(char **array){
  macaddress *mac;
  mac = (macaddress *)malloc(sizeof(macaddress));
  if (mac == NULL)
    return (NULL);
  
  int x=(rand() % NUMBER_OF_ADDRESSESS);
  for (int k=0; k<MAC_LENGTH; k++){
    mac->value[k]=array[x][k];
  }
  mac->tv = tic();
  return (mac);
}

void uploadContacts(macaddress *closeMacs, int limit){
  printf("Uploading contacts...\n");
  printf("       .\n       .\n");
  char **arr = (char **)malloc(limit*sizeof(char *));
  char *b="none";
  for (int k=0; k<limit; k++){
    char *a=closeMacs[k].value;
    if (strncmp(a, b, strlen(b)) != 0){
      arr[k] = a;
    }
  }
  char *name=(char *)malloc(100 * sizeof(char));
  
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(name, "Contacts_%d-%d-%d_%d-%d-%d.txt", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour+1, tm.tm_min, tm.tm_sec);
  writeFile(arr, name, (limit-1));
  printf("Contacts have been uploaded successfully!\n\n");
}

// function for checking if a macaddress is in an array
int *in(macaddress *arr, macaddress *target, int limit) {
  int res[2]={0, 0};
  int *ptr=res;
  char *b=target->value;
  for (int k=0; k<limit; k++) {
    char *a=arr[k].value;
    if(strncmp(a, b, strlen(b)) == 0) {
      res[0]=1;
      res[1]=k;
    }
  }
  return ptr;
}

void fun(macaddress *mac) {
    // check how much time each macaddress is in the arrays
  for(int k=0; k<j; k++){
    struct timeval begin = macsNearME[k].tv;
    double end = toc(begin);
    if(end>12)
      strcpy(macsNearME[k].value, "none");
  }
  for(int k=0; k<i; k++){
    struct timeval begin = closeMacs[k].tv;
    double end = toc(begin);
    if(end>12096)
      strcpy(closeMacs[k].value, "none");
  }
  
    /*
    printf("New mac: ");
    for (int k=0; k<MAC_LENGTH; k++)
      printf("%c",mac->value[k]);                          //kai auto einai print gia elegxo
    printf("\n");
    */
  int *a=in(macsNearME, mac, (j+1));
  if (a[0] == 1){
    struct timeval start1 = macsNearME[a[1]].tv;
    double final1 = toc(start1);
    if (final1 > 12){
      strcpy(macsNearME[a[1]].value, "none");
    }
    else{
      int *b=in(closeMacs, mac, (i+1));
      if (b[0] == 0){
        if (final1 >= 3){
          strcpy(closeMacs[i].value, mac->value);
          closeMacs[i].tv = mac->tv;
          closeMacs[i].id=i;
          i++;
        }
      }
      else{
        struct timeval start2 = closeMacs[b[1]].tv;
        if (toc(start2) > 12096){
          strcpy(closeMacs[b[1]].value, "none");
        }
      }
    }
  }
  else{
    // check for empty space in macsNearME
    int pos=-1;
    char *empty="none";
    for (int k=0; k<j; k++){
      char *str=macsNearME[k].value;
      if(strncmp(str, empty, strlen(empty)) == 0)
        pos=k;
    } 
    if (pos == -1){
      strcpy(macsNearME[j].value, mac->value);
      macsNearME[j].tv = mac->tv;
      macsNearME[j].id = j;
      j++;
    }
    else{
      strcpy(macsNearME[pos].value, mac->value);
      macsNearME[pos].tv = mac->tv;
      macsNearME[pos].id = pos;
    }
  }
    /*
  printf("//////  Macs Near Me   ///////\n");
  for (int k=0; k<(j+1); k++)
    printf("%s\n",macsNearME[k].value);
  printf("\n");                                              //auta einai ta prints gia na elegxete ti simbainei
  printf("//////  Close Macs   ///////\n");
  for (int k=0; k<(i+1); k++)
    printf("%s\n",closeMacs[k].value);
  printf("\n");
    */
  if(toc(hours) > 144){
    if (testCOVID()){
      printf("The test is positive!\n\n");
      uploadContacts(closeMacs, (i+1));
    }
    else
      printf("The test is negative!\n\n");
    hours=tic();
  }
}

// function to produce the macaddressess used for the program
char ** mac_produce(){
  
  time_t t;
  srand((unsigned) time(&t));

  char ** addressess = (char **)malloc(NUMBER_OF_ADDRESSESS*sizeof(char *));

  for (int i=0 ; i < NUMBER_OF_ADDRESSESS ; i++){

    char str[MAC_LENGTH]="";
    int ctr=1;
    
    for (int j=0 ; j < 12 ; j++){
      int val=(rand() % 16);   //get random integer between 0-15
      char hex[10];  
      sprintf(hex, "%x", val); //make integer a hex
      char x[10];
      sprintf(x, "%s", hex);  //make hex a char
      strncat(str, &x, 1);  //add char to string
      
      if ( (ctr % 2) == 0 && (ctr != 12)){
        const char x=':';   
        strncat(str, &x, 1);  //add ':'
      }
      ctr++;
    }
    char * address = (char*)malloc(MAC_LENGTH*sizeof(char));
    strcpy(address,str);
    addressess[i]=address;
  }
  writeFile(addressess,"MAC_Addressess.txt", NUMBER_OF_ADDRESSESS);
  return addressess;
}

bool testCOVID(){
  printf("Testing for COVID 19...\n");
  printf("          .\n          .\n");
  bool isPositive=false;
  if ( (rand()%100) > 90){
    isPositive=true;
  }
  return isPositive;
}

void writeFile(char **array, char *n, int limit){
    FILE *fp;
    char *name = (char *)malloc(100 * sizeof(char));
    sprintf(name,"%s", n);
    fp = fopen(name, "w"); 
    for (int i = 0; i < limit; i++){
      if(array[i]!=NULL)
        fprintf(fp, "%s\n", array[i]); 
    }
    fclose(fp);
}

void writeBinaryFile(int *array, char *n){
  FILE *fp;
  char *name = (char *)malloc(100*sizeof(char));
  sprintf(name,"%s", n);
  fp = fopen(name, "ab");
  for (int k = 0; k < 3000; k++)
    fwrite(&array[k],sizeof(array[k]),1,fp); 
  printf("Binary file has been updated!\n");
  fclose(fp);
}



