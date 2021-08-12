#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>


#define NUMBER_OF_ADDRESSESS 100
#define MAC_LENGTH 17

typedef struct{ 
    struct timeval tv;
    char value[MAC_LENGTH];
    int id;
} macaddress;

char ** mac_produce();
bool testCOVID();
void writeFile(char **a, char *n, int limit);
//char *readFile(char *filename, int line);

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

macaddress *BTnearMe(char **array){
  macaddress *mac;
  mac = (macaddress *)malloc(sizeof(macaddress));
  if (mac == NULL)
    return (NULL);
  time_t t;
  srand((unsigned) time(&t));
  int x=(rand() % NUMBER_OF_ADDRESSESS);
  for (int i=0; i<MAC_LENGTH; i++){
    mac->value[i]=array[x][i];
  }
  mac->tv = tic();
  return (mac);
}

void uploadContacts(macaddress *closeMacs, int limit){
  printf("Uploading contacts...\n");
  printf("       .\n       .\n");
  char **arr = (char **)malloc(limit*sizeof(char *));
  char *b="none";
  for (int i=0; i<limit; i++){
    char *a=closeMacs[i].value;
    if (strncmp(a, b, strlen(b)) != 0){
      arr[i] = a;
    }
  }
  char *name=(char *)malloc(100 * sizeof(char));
  int hours, minutes, seconds, day, month, year;
  time_t now;
  time(&now); 
  sprintf(name, "Contacts_%s.txt", ctime(&now));
  writeFile(arr, name, (limit-1));
  sleep(2);
  printf("Contacts have been uploaded successfully!\n\n");
}

int *in(macaddress *arr, macaddress *target, int limit) {
  int res[2]={0, 0};
  int *ptr=res;
  char *b=target->value;
  for (int i=0; i<limit; i++) {
    char *a=arr[i].value;
    if(strncmp(a, b, strlen(b)) == 0) {
      res[0]=1;
      res[1]=i;
    }
  }
  return ptr;
}

int main() {
  
  char ** addressess =  mac_produce();

  macaddress *macsNearME = (macaddress *)malloc(1*sizeof(macaddress));
  macaddress *closeMacs = (macaddress *)malloc(1*sizeof(macaddress));

  struct timeval start = tic();
  struct timeval hours = tic();
  int i=0, j=0;
  
  for (;;){
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
    macaddress *mac = BTnearMe(addressess);
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
      if (final1>12){
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
            closeMacs = (macaddress *)realloc(closeMacs, (i+1)*sizeof(macaddress));
          }
        }
        else{
          struct timeval start2 = closeMacs[b[1]].tv;
          double final2 = toc(start2);
          if (final2 > 12096){
            strcpy(closeMacs[b[1]].value, "none");
          }
        }
      }
    }
    else{
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
        macsNearME = (macaddress *)realloc(macsNearME, (j+1)*sizeof(macaddress));
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
    double fhours = toc(hours);
    if(fhours > 144){
      if (testCOVID()){
        printf("The test is positive!\n\n");
        uploadContacts(closeMacs, (i+1));
      }
      else
        printf("The test is negative!\n\n");
      hours=tic();
    }
    double final = toc(start);
    if (final > 25920){
      break;
    }
    sleep(0.1);
  }
 
  free(macsNearME);
  free(closeMacs);
  return 0;
}

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
  time_t t;
  srand((unsigned) time(&t));
  if ( (rand()%100) > 80){
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




