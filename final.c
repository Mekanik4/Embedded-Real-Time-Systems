#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdint.h>


#define NUMBER_OF_ADDRESSESS 5
#define MAC_LENGTH 17

struct uint48 {
    uint64_t x:48;
} __attribute__((packed));

typedef struct{ 
    struct timeval tv;
    char value[MAC_LENGTH];
    int id;
} macaddress;

typedef struct contact
{
  struct uint48 macaddress;
  //double timestamp;
} contact;

char ** mac_produce();
bool testCOVID();
void writeFile(char **a, char *n);
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
  printf("%d\n",x);
  for (int i=0; i<MAC_LENGTH; i++){
    mac->value[i]=array[x][i];
  }
  mac->tv = tic();
  return (mac);
}

void uploadContacts(macaddress *closeMacs, int limit){
  printf("Uploading contacts...\n");
  char **arr = (char **)malloc(NUMBER_OF_ADDRESSESS*sizeof(char *));
  char *b="none";
  for (int i=0; i<limit; i++){
    char *a=closeMacs[i].value;
    if (strncmp(a, b, strlen(b)) != 0){
      arr[i] = a;
    }
  }
  writeFile(arr, "Contacts.txt");
  system("cp Contacts.txt embedded/");
  system("cd embedded");
  system("git add Contacts.txt");
  system("git commit -m \"Contacts\"");
  system("git push");
  //sleep(2);
  printf("Contacts have been uploaded successfully!\n");
  free(arr);
}

int *in(macaddress *arr, macaddress *target, int limit) {
  int res[2]={0, 0};
  int *ptr=res;
  char *b=target->value;
  printf("%s\n",arr[0].value);
  for (int i=0; i<limit; i++) {
    char *a=arr[i].value;
    printf("%s\n",a);
    if(strncmp(a, b, strlen(b)) == 0) {
      res[0]=1;
      res[1]=i;
    }
  }
  return ptr;
}

int main() {
  
  char ** addressess =  mac_produce();

  /*
  macaddress *mac=BTnearMe(addressess);
  for (int i=0; i<MAC_LENGTH; i++)
    printf("%c",mac->value[i]);
  printf("\n");
  //printf("size=%d\n",sizeof(contact));
  
  //uploadContacts(addressess);
  */
  macaddress *macsNearME = (macaddress *)malloc(1*sizeof(macaddress));
  macaddress *closeMacs = (macaddress *)malloc(1*sizeof(macaddress));

  struct timeval start = tic();
  struct timeval hours = tic();
  int i=0, j=0;
  
  for (;;){
    macaddress *mac = BTnearMe(addressess);
    printf("New mac: ");
    for (int k=0; k<MAC_LENGTH; k++)
      printf("%c",mac->value[k]);
    printf("\n");
    int *a=in(macsNearME, mac, (j+1));
    printf("%d\n",a[0]);
    if (a[0] == 1){
      struct timeval start1 = macsNearME[a[1]].tv;
      double final1 = toc(start1);
      if (final1>12){
        strcpy(macsNearME[a[1]].value, "none");
      }
      else{
        int *b=in(closeMacs, mac, (i+1));
        printf("%d\n",b[0]);
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
      strcpy(macsNearME[j].value, mac->value);
      macsNearME[j].tv = mac->tv;
      macsNearME[j].id = j;
      j++;
      macsNearME = (macaddress *)realloc(macsNearME, (j+1)*sizeof(macaddress));
    }
    double fhours = toc(hours);
    if(fhours > 14){
      if (testCOVID()){
        uploadContacts(closeMacs, (i+1));
      }
    }
    double final = toc(start);
    if (final > 25920){
      break;
    }
    printf("//////  Macs Near Me   ///////\n");
    for (int k=0; k<(j+1); k++)
      printf("%s\n",macsNearME[k].value);
    printf("\n");
    printf("//////  Close Macs   ///////\n");
    for (int k=0; k<(i+1); k++)
      printf("%s\n",closeMacs[k].value);
    printf("\n");
    sleep(1);
  }

  /*
  for (int i=0 ; i < NUMBER_OF_ADDRESSESS ; i++){
    for(int j=0; j < MAC_LENGTH; j++)
      printf("%c", addressess[i][j]);
    printf("\n");
  }
  
  bool test=testCOVID();
  printf("The test is %s", test ? "positive" : "negative"); 
  printf("\n");
  */
 
  /*
  int x;
  printf("insert no. of mac address: ");
  scanf("%d",&x);
  for (int i=0; i<MAC_LENGTH; i++)
    printf("%c", addressess[(x-1)][i]);
  printf("\n");
  */
 
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
  writeFile(addressess,"MAC_Addressess.txt");
  return addressess;
}

bool testCOVID(){

  bool isPositive=false;
  time_t t;
  srand((unsigned) time(&t));
  if ( (rand()%100) > 90){
    isPositive=true;
  }
  return isPositive;
}

void writeFile(char **array, char *n){
    FILE *fp;
    char *name = (char *)malloc(6 * sizeof(char));
    sprintf(name, n);
    fp = fopen(name, "w"); 
    for (int i = 0; i < NUMBER_OF_ADDRESSESS; i++){
        fprintf(fp, "%s\n", array[i]); 
    }
    fclose(fp);
}



/*
char *readFile(char *filename, int line){
  FILE *fp = fopen(filename, "r");
  char *mac;
  if (fp == NULL){
      printf("Error: could not open file %s", filename);
  }
  char buffer[(MAC_LENGTH+1)];
  int i=0;
  while (fgets(buffer, (MAC_LENGTH+1), fp)){
    i++;
    if(i == line){ 
      mac=&buffer[0];   
      printf("%s\n", buffer); 
    } 
  }
  fclose(fp);
  
   for (int i=0; i<MAC_LENGTH; i++)
    printf("%c", mac[i]);
  printf("\n");
  return mac;
}
*/
