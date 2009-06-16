/*
*  C Implementation: server.c
*
*       Description: Test client program for LAN-based play in Tux,of Math Command.
*
*
* Author: Akash Gangil, David Bruce, and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*
* NOTE: This file was initially based on example code from The Game Programming Wiki
* (http://gpwiki.org), in a tutorial covered by the GNU Free Documentation License 1.2.
* No invariant sections were indicated, and no separate license for the example code
* was listed. The author was also not listed. AFAICT,this scenario allows incorporation of
* derivative works into a GPLv2+ project like TuxMath - David Bruce 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "SDL_net.h"
#include "transtruct.h"
#include "mathcards.h"
#include "mathcards.c"

TCPsocket sd;           /* Socket descriptor */
SDLNet_SocketSet set;

MC_FlashCard flash;

int main(int argc, char **argv)
{
  IPaddress ip;           /* Server address */
  int quit, len, sockets_used;
  char buffer[512];  // for command-line input
  char buf[512];     // for network messages from server
  MC_FlashCard* fc;
  int x, i = 0;

  /* Simple parameter checking */
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s host port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
 
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, argv[1], atoi(argv[2])) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  // Create a socket set to handle up to 16 sockets
  set = SDLNet_AllocSocketSet(16);
  if(!set) {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  sockets_used = SDLNet_TCP_AddSocket(set, sd);
  if(sockets_used == -1) {
    printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
    // perhaps you need to restart the set and make it bigger...
  }

  /* Send messages */
  quit = 0;
  while (!quit)
  {
    //Get user input from command line and send it to server: 
    printf("Write something:\n>");
    scanf("%s", buffer);

    if(strcmp(buffer, "exit") == 0)
      quit = 1;
    if(strcmp(buffer, "quit") == 0)
      quit = 1;

    len = strlen(buffer) + 1;
    if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
    {
      fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
      exit(EXIT_FAILURE);
    }

    //Now we check for any responses from server:
    //FIXME have to make this not block so we can keep checking until all the messages are gone
    //do
    {
      char command[NET_BUF_LEN];
      int i = 0;
      int numready;

      //This is supposed to check to see if there is activity and time out
      // after 1000 ms if no activity
      numready = SDLNet_CheckSockets(set, 1000);
      if(numready == -1)
      {
        printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        //most of the time this is a system error, where perror might help you.
        perror("SDLNet_CheckSockets");
      }
      else if(numready)
      {
        printf("There are %d sockets with activity!\n",numready);
        // check all sockets with SDLNet_SocketReady and handle the active ones.
        if(SDLNet_SocketReady(sd))
        {
          x = SDLNet_TCP_Recv(sd, buf, sizeof(buf));
          /* Copy the command name out of the tab-delimited buffer: */
          for (i = 0; buf[i] != '\t' && i < NET_BUF_LEN; i++)
            command[i] = buf[i];
          command[i] = '\0';
          printf("buf is %s\n", buf);
          printf("command is %s\n", command);
          /* Now we process the buffer according to the command: */
          if(strcmp(command, "SEND_QUESTION") == 0)
          {
            /* function call to parse buffer into MC_FlashCard */
          }
        }
      }
      
    }// while (x > 0);

  }
 
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}


//function to receive a flashcard(question) by the client
//FIXME - this is going to change - will just need a function to convert the
//buffer string into a flashcard.
int RecvQuestion(void)
{
  char ch[5];
  int x, i = 0;

  x = SDLNet_TCP_Recv(sd, &(flash.question_id), sizeof(flash.question_id));
  printf("no:(1):::QUESTION_ID::::Received %d bytes\n",x);
 
  x = SDLNet_TCP_Recv(sd, &(flash.difficulty), sizeof(flash.difficulty));
  printf("no:(2):::DIFFICULTY::::Received %d bytes\n",x);
 
  x = SDLNet_TCP_Recv(sd, &(flash.answer), sizeof(flash.answer));
  printf("no:(3):::ANSWER::::Received %d bytes\n",x);

  do{
    x = SDLNet_TCP_Recv(sd, &ch[i], 1);      
    printf("<<<SUB-PACKET%d>>>no:(4):::ANSWER_STRING::::Received %d bytes\n", i, x);
    i++;
  }while(ch[i-1]!='\0');

  strncpy(flash.answer_string, ch, i + 1);

  x = SDLNet_TCP_Recv(sd, flash.formula_string, 13);

  printf("no:(5):::FORMULA_STRING::::Received %d bytes\n",x);
  printf("RECEIVED >>\n");
  printf("QUESTION_ID    >>          %d\n",flash.question_id);  
  printf("FORMULA_STRING >>          %s\n",flash.formula_string);  
  printf("ANSWER_STRING  >>          %s\n",flash.answer_string);  
  printf("ANSWER         >>          %d\n",flash.answer);  
  printf("DIFFICULTY     >>          %d\n",flash.difficulty);  
       
  return 1;
}



 
