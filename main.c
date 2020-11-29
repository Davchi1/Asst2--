#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <ctype.h>

void* checkDir(void* arg);
void* checkFile(void* arg);
#define MAXCHAR 1000
//Thread stuff
//--------------------------------------------------------------------------------
typedef struct threadPasser{
  char* ptPath;
  struct dirent* ptDent;
  int arrPosition;
}tPasser;

//Linked list that consists of a thread and a pointer to the next node

typedef struct node{
  pthread_t id;
  char* dirName;
  struct node* next;
  int arrPosition;
}node_t;
//Function to create a new thread node
node_t* create_new_node(pthread_t ids, char* directoryName){
  node_t* res = malloc(sizeof(node_t));
  res ->dirName = directoryName;

  res -> id = ids;
  res->next=NULL;
  return res;
}
//Function to join and print out all the threads created for files and directories
void printlist(node_t *head){
  node_t* pTemp = head;
  while(pTemp!=NULL){
   // printf("Directory Name: %s \n",pTemp->dirName);
   // printf("thread id = %ld\n",pthread_self());
    pthread_join(pTemp->id,NULL);
    pTemp=pTemp->next;
  }
}

//----------------------------------------------------------------------------------
typedef struct File{
char* fileName;
int totalWords;
struct File* next;
struct Tokens* tokenList;
}file;
//A Doubly linked list for the tokens... for alphabetical insertion

typedef struct Tokens{
char* tokenName;
float wordFrequency;
struct Tokens* next;
struct Tokens* prev;
}token;
static pthread_t id[10000];
static pthread_mutex_t lock;
static pthread_mutex_t nlock;
static pthread_mutex_t mutexDirectories = PTHREAD_MUTEX_INITIALIZER;
//Traverse all entries in the directory.... Look for other directories or files
//IF a directory we want to create a new directory thread... if it is a File we want to create a new file thread... Otherwise we ignore the entry 
void* checkDir(void* arg){

  tPasser* directory = (tPasser*)arg;
  
  DIR *openDir = opendir(directory->ptPath);
 
  struct dirent* dent;
  int arrpos = directory->arrPosition;
  dent = readdir(openDir); 
  dent = readdir(openDir); 
  while((dent = readdir(openDir)) != NULL){
    //We find a directory
    
    if(dent->d_type == 4){
      
       char* newName = (char*)malloc(sizeof(strlen(directory->ptPath)+strlen(dent->d_name))+2);      
       strcat(newName,directory->ptPath);
       strcat(newName,"/");
       strcat(newName,dent->d_name);      
       printf("Opening directory... : %s\n",newName);
       tPasser* newDirectory = malloc(sizeof(tPasser));
       newDirectory->ptPath = newName;        
       newDirectory->ptDent = dent;
       //newName="";
       pthread_create(&id[arrpos],NULL,checkDir,newDirectory);
       directory->arrPosition+=1;
    }else if(dent->d_type==8){
       char* newName = (char*)malloc(sizeof(strlen(directory->ptPath)+strlen(dent->d_name))+2);      
       strcat(newName,directory->ptPath);
       strcat(newName,"/");
       strcat(newName,dent->d_name);      
       printf("Opening file... : %s\n",newName);
       tPasser* newDirectory = malloc(sizeof(tPasser));
       newDirectory->ptPath = newName;        
       newDirectory->ptDent = dent;
       //newName="";
       pthread_create(&id[arrpos],NULL,checkFile,(void*)newDirectory);
       directory->arrPosition+=1;
    }


  }

  pthread_exit(NULL);
  

}
/*Tokenize every file. The program has no way of determining whether a file contains text or not, so it must assume every file contains text.*/

file* fHead;
void* checkFile(void* arg){
  tPasser* myFile = (tPasser*)arg;
  file* newFileMember = malloc(sizeof(file));
  newFileMember->fileName=myFile->ptPath;
  //Traverse the file... for every unique word create a new node. - Insert in order

  FILE * fp;

  fp = fopen(myFile->ptPath,"r");
  if(fp==NULL){
    pthread_exit(NULL);
  }
  //Using the buffer check if the word is present in the tokens LL if it is increment the token for that word by 1 ... If it is not create a new node holding that string and insert
  token* headToken = malloc(sizeof(token));

  char wordBuffer[MAXCHAR];
  int c;
  int i = 0;
  do{
    c = fgetc(fp);
    //Found a word
    if(isspace(c)){
      printf("Got here %s\n",wordBuffer);
      //First node
      if(headToken->tokenName == NULL){
        
        headToken->tokenName= wordBuffer;
        printf("Head Token: %s ",headToken->tokenName);
        headToken->wordFrequency = 1;
        token* temp = malloc(sizeof(token));
        headToken->next = temp;
        temp->prev = headToken;      
      }
      //If first node is fill traverse LL
      else{
        token* ptr = headToken;
        // A G U V 
        // F        ^
        while(ptr->next != NULL){
          //If word already exists
          if(strcmp(ptr->tokenName,wordBuffer) == 0){
            ptr->wordFrequency++;
            break;
          }
          if(ptr->next==NULL){
            
          }
          //Perform insertion
          else if(strcmp(wordBuffer,ptr->tokenName)>0){
            token* newNode = malloc(sizeof(token));
            newNode->tokenName=wordBuffer;
            newNode->wordFrequency=1;
            token* temp = ptr->next;
            ptr->next = newNode;
            temp->prev = newNode;
            newNode->next = temp;
            newNode->prev = ptr;
            //Greater then insert after pointer    
          }
        }
       
        //apple bat
        
      }
      i = 0;
      memset(wordBuffer, 0, MAXCHAR);
    }else{
      wordBuffer[i] = c;
      i++;
    }  
    if(feof(fp)){
      break;
    }
   
  }while(1);
  //Test print out tokens
 
  printf(" \n");
  token* tmpPtr = headToken;
  while(tmpPtr!=NULL){
    //printf("%s -> ", tmpPtr->tokenName);
    tmpPtr=tmpPtr->next;
  }

  fclose(fp);
  //To insert traverse the LL and using strcmp look for a place to insert the new node... adjust the double LL pointers as necesary... have two pointers for the  traversal
  
  //printf("File path: %s\n",myFile->ptPath);
  /*struct dirent *dent = myFile->ptDent;
  char* path = myFile->ptPath;*/

  pthread_exit(NULL);

}
/*
typedef struct File{
char* fileName;
int totalWords;
struct File* next;
struct Tokens* tokenList;
}file;

*/
//
//File Linked List

//(File Name, Total Words, tokenList<>) -> (File Name, Total Words, tokenList<>) -> (File Name, Total Words, tokenList<>)

//wordTokens Linked List

//(Token name,wordFrequency) -> (Token name, wordFrequency) -> (Token name, wordFrequency)
//File will be our shared data structure
//DIR
  //./DIR/DIR2
      //DIR/DIR2/DIR3




int main(int argc, char** argv){

DIR *pDir;
struct dirent *dentt;
pDir = opendir(argv[1]);
//Checks if the command line passes in a directory, otherwise exits the process.
//
if(pDir == NULL){
  printf("You have to pass in a directory in the command linee\n");
  exit(1);
}
dentt = readdir(pDir);

if(dentt->d_type != 4){
  printf("You have to pass in a directory in the command line\n");
  exit(1);
}
//Starting struct
tPasser* starter;
char* start = malloc(sizeof(strlen(argv[1])+3));
start[0]='.';
start[1]='/';
//printf("name: %s\n",dent->d_name);
strcat(start, argv[1]);
starter ->ptDent = dentt;
starter ->ptPath = start;
starter->arrPosition=0;
//printf("start: %s\n",starter->ptPath);
//printf("%s\n" ,starter->ptPath);
//Passes in the root directory struct into the dir function
node_t* pHead;
node_t* pTemp;
printlist(pHead);
checkDir((void*)starter);
int x = 0;
for(; x<1000;x++){
pthread_join(id[x], NULL);
}

//Joining the threads created for files and directories 





//If so we can pass it into the dir function

//  NAME ,THREAD
// (Dir,Thread,Pointer) -> (Dir,Thread,Pointer) -> (Dir,Thread,Pointer)-> (Dir,Thread,Pointer)
//pthread_join(&id, NULL);
// (File Name, Total Words, wordsTokens<>) -> (File Name, Total Words, wordsTokens<>) -> (File Name, Total Words, wordsTokens<>)


return 0;
}

//In order to read a file we need the directory path
//The directory path will be the directory the file is in concat with the file name
//The name of the directory of the file will be the directory concat with the directory that called its thread