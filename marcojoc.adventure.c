/******
 * marcojoc.adventure.c
 * by Camille Marcojos, CS344 - Operating Systems, Summer 2020
 * This program provides an interface for playing the adventure game using the most recently generated rooms 
 * from the marcojoc.buildrooms.c program
 ******/
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

struct room { 
    int id;
    char* name; 
    char* type; 
    int room_connections; 
    char* connected_rooms[6];
}; 
typedef struct room Room;

char newestDirName[256]; // Holds the name of the newest dir that contains prefix
char gameDir[256]; // Holds the name of the dir to play the game
Room room_graph[7];

void findNewestDir();
void printRooms();
void game();
Room findStartRoom();
void printLocation(Room currRoom);
Room getCurrentRoom(char* name);
bool checkRoomType(Room curr_room);


int main()
{
    findNewestDir();
    char gameDir[256];

    int i;

    sprintf(gameDir,"./%s",newestDirName);
    //printf("Directory: %s", gameDir);
    chdir(gameDir);
    
    DIR* dirToCheck;
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    dirToCheck = opendir("."); // Open up the directory this program was run in
    char newFileName[256];
    memset(newFileName, '\0', sizeof(newFileName));
    
    if(dirToCheck > 0) // Make sure the current directory could be opened
    {
        int i=0;
        while((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
            if (strcmp(fileInDir->d_name, ".") && strcmp(fileInDir->d_name, "..")){  //To get rid of "." and ".." files:

                //printf("%s\n", fileInDir->d_name);
                // read in a file
                char str[250];
                FILE* fp=fopen(fileInDir->d_name, "r");
                fgets(str,250, fp);
                char* token=strtok(str,":");
                token=strtok(NULL," \n");
                //printf("%s\n", token);
                room_graph[i].name = calloc(16, sizeof(char));
                strcpy(room_graph[i].name, token);
                //printf("In array %d - room name: %s\n",i, room_graph[i].room_name);
                int j=0;

                //Reading from file - if token2 contains START_ROOM, MID_ROOM, END_ROOM, set the type in
                //the room_graph. Otherwise, it's a connected room and must be added to the connected rooms array
                while(fgets(str, 250, fp)) {
                    //printf("%s\n", str);
                    char* token2=strtok(str,":");
                    //printf("%s\n", token2);
                    token2=strtok(NULL," ");
                    //printf("%s", token2);
                    room_graph[i].room_connections=0;
                    if(strcmp(token2,"START_ROOM\n") == 0)
                    {
                        room_graph[i].type="START_ROOM";
                        //printf("In array %d - room type: %s\n",i, room_graph[i].room_type);
                    }else if(strcmp(token2,"MID_ROOM\n")==0){ 
                        room_graph[i].type="MID_ROOM";
                        //printf("In array %d - room type: %s\n",i, room_graph[i].room_type);
                    }else if(strcmp(token2,"END_ROOM\n") == 0){
                        room_graph[i].type="END_ROOM";
                        //printf("In array %d - room type: %s\n",i, room_graph[i].room_type);
                    }else{
                        room_graph[i].connected_rooms[j] = calloc(10, sizeof(char));
                        strncpy(room_graph[i].connected_rooms[j], token2, strlen(token2)-1);
                        //strcpy(room_graph[i].connected_rooms[j],token2);
                        //printf("In array %d - room connections %d: %s\n",i,j, room_graph[i].connected_rooms[j]);
                        j++;
                        //printf("In array %d - Num Connections %d: %d\n",i,j, room_graph[i].room_connections);
                    }

                }
                room_graph[i].room_connections=j;
                room_graph[i].id=i;
                fclose(fp);
                j=0;
                i++;
            }
        }

    }else{
        printf("Could not be opened.\n");
    }
    closedir(dirToCheck);

    //printRooms();

    game();

    return 0;
}

/** 
 * finds most recently created directory with most recent game files
 * taken from class reading: 2.4 Manipulating Directories
 **/
void findNewestDir(){
    int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32]= "marcojoc.rooms."; // Prefix we're looking for
    memset(newestDirName, '\0', sizeof(newestDirName));

    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir

    dirToCheck = opendir("."); // Open up the directory this program was run in

    if(dirToCheck > 0) // Make sure the current directory could be opened
    {
        while((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
            if(strstr(fileInDir->d_name, targetDirPrefix) !=NULL) // If entry has prefix
            {
                //printf("Found the prefix: %s\n", fileInDir->d_name);
                stat(fileInDir->d_name, &dirAttributes);    // Get attributes of the entry

                if((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0',sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                    //printf("Newer subdir: %s, new time: %d\n", fileInDir->d_name, newestDirTime);
                }
            }
        }
    }
    closedir(dirToCheck); // Close the directory we opened

    //printf("Newest entry found is: %s\n", newestDirName);

}

/**
 * Prints room information and connections in room graph
 **/
void printRooms(){
    printf("PRINTING DETAILS\n");
    int i,j;
    for(i=0; i<7; i++)
    {
        printf("Room Name: %s\n", room_graph[i].name);
        printf("Room ID: %d\n", room_graph[i].id);
        for(j=0; j<room_graph[i].room_connections; j++)
        {
        printf("Connections: %s\n", room_graph[i].connected_rooms[j]);
        }
        printf("Room Type: %s\n", room_graph[i].type);
    }
}

/**
 * Driver function for running the game
 **/
void game(){
    char userInput[256];
    int stepCount;
    stepCount=0;
    bool match = false; //for checking if the userinput matches any of the room name
    bool play = true; //for replaying the prompt if end room hasn't been found yet
    Room* path = (Room*) malloc(100*sizeof(Room)); //holds the path the player is taking

    Room current_room;
    current_room = findStartRoom();
    printLocation(current_room);

    //loop for re-prompting the player if end room hasn't been found yet
    do{
        fgets(userInput,256,stdin);
        userInput[strlen(userInput)-1]='\0';

        for(int i=0; i<current_room.room_connections; i++){
            if(strcmp(userInput,current_room.connected_rooms[i]) == 0){
                //printf("Matching.\n");
                match=true;
                break;
            }
        }

        //conditional statement if player inputs a room name that matches or doesn't match one of the connections in the prompt.
        //if it doesn't match any of the room names then an error msg appears and they are re-prompted
        if(match){
            current_room=getCurrentRoom(userInput);
            path[stepCount]=current_room;
            stepCount++;

            //checking if the room is END_ROOM
            if(checkRoomType(current_room)){
                play=false;
            }else{
                printLocation(current_room);
                match=false;
            }
        }else{
            printf("\n\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
            printLocation(current_room);
        }
        
    }while(play);

    //prompt for when player successfully finds the end room
    printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
    for(int i=0; i<stepCount; i++){
        printf("%s\n",path[i].name);
    }

    free(path);

}

// Check if room is the end room
// parameter: struct room
// return: bool (true or false)
bool checkRoomType(Room curr_room){
    if(strcmp(curr_room.type,"END_ROOM") == 0){
        return true;
    }else{
        return false;
    }
}

// Gets the start room for the game
// parameter: none
// return: struct room
Room findStartRoom(){
    
    int i,j;
    for(i=0; i<7; i++){
        if(strcmp(room_graph[i].type,"START_ROOM") == 0){
            return room_graph[i];
        };
    }
}

// Gets the room according to user input
// parameter: char* which is the user input
// returns: struct room
Room getCurrentRoom(char* name){
    
    int i,j;
    for(i=0; i<7; i++){
        if(strcmp(room_graph[i].name,name) == 0){
            return room_graph[i];
        };  
    }
}

// Prints the prompt for current room location and its connections
// parameter: struct room
// returns: nothing
void printLocation(Room currRoom){

    printf("\nCURRENT LOCATION: %s\n", currRoom.name);
    printf("POSSIBLE CONNECTIONS: ");
    int i;
    int numConnections=currRoom.room_connections;
    for(i=0; i<numConnections; i++){
        printf("%s", currRoom.connected_rooms[i]);
        if (i != (numConnections-1)){
            printf(", ");
        }else
            printf(".\n");
    }
    printf("WHERE TO? >");

}
