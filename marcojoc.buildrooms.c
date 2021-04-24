/******
 * marcojoc.buildrooms.c
 * by Camille Marcojos, CS344 - Operating Systems, Summer 2020
 * This program creates a series of files that hold descriptions of the in-game rooms and how the rooms are connected.
 ******/
#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

struct room{
        int id;
        char* name;
        char* type;
        int num_connections;
        struct room* outbound_connections[6];
    };
typedef struct room Room;

bool IsGraphFull(Room* rooms);
Room GetRandomRoom(Room* rooms);
void AddRandomConnection(Room* rooms);
bool CanAddConnectionFrom(Room x);
bool IsSameRoom(Room x, Room y);
bool ConnectionAlreadyExists(Room x,Room y);
void ConnectRoom(Room* x, Room* y);

int main() {

    srand(time(NULL));

    //HOLDS ALL ROOM INFORMATION AND THEIR CONNECTIONS
    Room room_graph[7];

    //HOLDS THE 10 ROOM NAMES 
    char* rooms_list[10]={"Crowther", "Dungeon","Gryfndor","PLOVER","twisty",
                            "XYZZY","Zork","Huffpuff","Slythrn","Ravnclw"};

    //printf("...printing rooms list before shuffle...\n");
    //for(int i=0; i<10; i++){
        //printf("%s\n",rooms_list[i]);
    //}
    
    //SHUFFLE THE ARRAY OF ROOMS SO THAT EACH ITERATION OF THE PROGRAM WILL GENERATE A RANDOM LIST OF ROOMS
    for(int i=0; i<10; i++){
        char* temp = rooms_list[i];
        int randIdx = rand() % 10;
        rooms_list[i] = rooms_list[randIdx];
        rooms_list[randIdx] = temp;
    }

    //printf("\n...printing rooms list after shuffle...\n");
    //for(int i=0; i<10; i++){
        //printf("%s\n",rooms_list[i]);
    //}

    //input the first 7 rooms from rooms_list into room_graph
    for(int i=0; i<7; i++){
        room_graph[i].name=rooms_list[i];
        room_graph[i].num_connections=0;
        room_graph[i].id=i;

        if(i==0){
            room_graph[i].type="START_ROOM";
        }else if(i==6){
            room_graph[i].type="END_ROOM";
        }else{
            room_graph[i].type="MID_ROOM";
        };
    }

    //printf("\n...printing rooms in room graph...\n");
    //for(int i=0; i<7; i++){
        //printf("Room %i name: %s type: %s\n",i,room_graph[i].name,room_graph[i].type);
    //}

    //CHECKING IF ALL ROOMS HAVE AT LEAST 3 CONNECTIONS
    while(IsGraphFull(room_graph) == false){
        AddRandomConnection(room_graph);
    }

    //printf("\n...printing room connections...\n");
    //for(int i=0; i < 7; i++){
        //printf("Room %d name: %s type: %s num connections: %d\n", i, room_graph[i].name,room_graph[i].type, room_graph[i].num_connections);
        //for(int j=0; j<room_graph[i].num_connections; j++){
            //printf("Outbound connections - room name: %s type: %s num connections: %d\n", room_graph[i].outbound_connections[j]->name, room_graph[i].outbound_connections[j]->type, room_graph[i].outbound_connections[j]->num_connections);
        //}
    //}

    //CREATING THE DIRECTORY AS MARCOJOC.ROOMS.<RANDOM NUMBER BT 0 AND 100000
    //printf("Creating directory.\n");
    char dirName[256];
    int randNum = rand() % 100001;
    memset(dirName,'\0',sizeof(dirName));
    sprintf(dirName,"marcojoc.rooms.%d",randNum);
    mkdir(dirName,0755);
    //printf("Directory created.\n");

    //CHANGING INTO THE NEWLY CREATED DIRECTORY
    //printf("Changing into directory\n");
    if(chdir(dirName) != 0)
    {
        printf("ERROR in changing directory.\n");
        return 0;
    };
    //printf("Changed directory success\n");


    //CREATING THE ROOM FILES
    FILE* roomFile;

    for(int i=0; i<7; i++)
    {
        FILE* roomFile=fopen(room_graph[i].name,"w");

        fprintf(roomFile,"ROOM NAME: %s\n", room_graph[i].name);
        for(int j=0; j < room_graph[i].num_connections; j++){
            fprintf(roomFile,"CONNECTION %d: %s\n", j+1, room_graph[i].outbound_connections[j]->name);
        }
        fprintf(roomFile,"ROOM TYPE: %s\n", room_graph[i].type);
        fclose(roomFile);
    };

}

/**
 * This function checks if all the rooms have at least 3 connections
 * 
 **/
bool IsGraphFull(Room* rooms)  
{
    for(int i=0; i<7; i++){
        if(rooms[i].num_connections < 4){
            //printf("Rooms are not full.\n");
            return false;
        }
    }
    //printf("Rooms are full.\n");
    return true;
}

/**
 * Driver function for getting 2 random rooms, checking their validity, and adding the connection between the 2 rooms
 **/
void AddRandomConnection(Room* rooms)  
{
  Room A;  // Maybe a struct, maybe global arrays of ints
  Room B;

  while(true)
  {
    A = GetRandomRoom(rooms);
    //printf("\nGot random room in A. Name: %s type: %s\n", A.name, A.type);

    if (CanAddConnectionFrom(A) == true){
        break;
    }
        
  }

  do
  {
    B = GetRandomRoom(rooms);
    //printf("\nGot random room in B. Name: %s type: %s\n", B.name, B.type);
  }
  while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

  ConnectRoom(&rooms[A.id], &rooms[B.id]);  // TODO: Add this connection to the real variables, 
  ConnectRoom(&rooms[B.id], &rooms[A.id]);  //  because this A and B will be destroyed when this function terminates

}


// Returns a random Room, does NOT validate if connection can be added
Room GetRandomRoom(Room* rooms)
{
  int randNum;
  randNum = rand() % 7;
  return rooms[randNum];
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(Room x) 
{
    if(x.num_connections < 6){
        //printf("\nWe can add connection.\n");
        return true;
    }else{
        //printf("\nCan't add connection.\n");
        return false;
    }
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(Room x, Room y) 
{
  if(strcmp(x.name, y.name) == 0){
      //printf("A and B are the same room.\n");
      return true;
  }else{
      //printf("A and B are NOT the same room.\n");
      return false;
  }
}

// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(Room x,Room y)
{
    if(x.num_connections == 0 || y.num_connections == 0){
        //printf("Room A or B have no connections yet.\n");
        return false;
    }else{
        for(int i=0; i < x.num_connections; i++){
            if(strcmp(x.outbound_connections[i]->name,y.name) == 0){
                //printf("A and B connected already.\n");
                return true;
            }
        }
    }
    
    //printf("A and B are NOT connected yet.\n");
    return false;
}

// Creates a connection between rooms
void ConnectRoom(Room* x, Room* y) 
{
    x->outbound_connections[x->num_connections] = y;
    x->num_connections++;
}