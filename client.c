/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <poll.h>

#define MAX_ROW 6
#define MAX_COL 7
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_ORANGE "\x1b[32m"
#define ANSI_COLOR_GREEN "\x1b[32m"


/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
int board[MAX_ROW][MAX_COL];
int player_colors[2];
int scores[2];

bool is_valid_movement(int col);
void print_board(int board[][MAX_COL]);

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int col=0;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  int turn;
  bool valid_string_condition;
  bool winning_condition = false;
  char winner_message[20];
  time_t finish;
  struct pollfd input_timer = { STDIN_FILENO, POLLIN|POLLPRI};
  int option;
  bool can_play;

  if (read (sd, &can_play, sizeof(bool)) <= 0)
  {
    perror ("[client]Eroare la read() de la server.\n");	
  }

  if(can_play == false){
    printf("Numarul maxim de clienti deja atins. \n");
    return 0;
  }

  while(1){

    winning_condition = false;
    option = 0;


    if (read (sd, &player_colors, sizeof(player_colors)) <= 0)
	 	{
	 		perror ("[client]Eroare la read() de la server.\n");	
	 	}

    if (read (sd, &board, sizeof(board)) <= 0)
    {
      perror ("[client]Eroare la read() de la server.\n");	
    }

    system("clear");
    printf("Score: %d %d \n\n", scores[0], scores[1]);
    fflush(stdout);
    print_board(board);
    printf("\n");

    while(!winning_condition){

      if (read (sd, &winning_condition, sizeof(bool)) <= 0)
      {
        perror ("[client]Eroare la read() de la server.\n");	
      }
    
      if(winning_condition == true){

        if (read (sd, &winner_message, sizeof(winner_message)) <= 0)
        {
          perror ("[client]Eroare la read() de la server.\n");	
        }

        system("clear");
        printf("Ati ");
        if(strcmp(winner_message,"castigat") == 0)
          printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET, winner_message);
        else if(strcmp(winner_message,"pierdut") == 0)
          printf(ANSI_COLOR_RED "%s" ANSI_COLOR_RESET, winner_message);
        else 
          printf(ANSI_COLOR_ORANGE "%s" ANSI_COLOR_RESET, winner_message);
        printf(".\n\n");
        printf("Foloseste una din comenzile: \n");
        printf("1) rematch\n");
        printf("2) exit\n");
        fflush(stdout);
        
        do{
          scanf("%d", &option);
          if(option == 1){
            if (write (sd, &option, sizeof(int)) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }
          }
          else if(option == 2){
            if (write (sd, &option, sizeof(int)) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }
            /* inchidem conexiunea, am terminat */
            //close (sd);
            return 0;
          }
        }while((option != 1) && (option != 2));

      }else{

        if (read (sd, &scores, sizeof(scores)) <= 0)
        {
          perror ("[client]Eroare la read() de la server.\n");	
        }
      
        /* apel ce blocheaza executia pana in momentul in care incepe tura jucatorului */
        if (read (sd, &board, sizeof(board)) <= 0)
          {
            perror ("[client]Eroare la read() de la server.\n");	
          }

        system("clear");
        printf("Score: %d %d \n\n", scores[0], scores[1]);
        print_board(board);
        printf("\n");
        fflush(stdout);

        /* trimiterea mesajului ce contine mutarea la server */
        do{

          system("clear");
          printf("Score: %d %d \n\n", scores[0], scores[1]);
          fflush(stdout);
          print_board(board);
          printf("\n");
          printf("Introduceti numarul coloanei pe care doriti sa puneti piesa: \n");
          fflush(stdout);

          /* rezolva bugul ce apare cand un user incearca sa faca mai multe mutari pe tura lui */
          if(poll(&input_timer,1,8000)) {
            //  finish = time(NULL);
            //  while(time(NULL) < finish + 1){    
            //     scanf("%d",&col);
            //   }
              scanf("%d",&col);
          } else {
            srand(time(NULL));
            col = rand() % MAX_COL;
          }

          if (write (sd, &col, sizeof(int)) <= 0)
          {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
          }

          if (read (sd, &valid_string_condition, sizeof(bool)) <= 0)
          {
            perror ("[client]Eroare la read() de la server.\n");	
          }
    
        }while(!valid_string_condition);

        if (read (sd, &board, sizeof(board)) <= 0)
        {
          perror ("[client]Eroare la read() de la server.\n");	
        }

        system("clear");
        printf("Score: %d %d \n\n", scores[0], scores[1]);
        fflush(stdout);
        print_board(board);
        printf("\n");
        fflush(stdout);

      }
    }
  }
}


bool is_valid_movement(int col){
    /* input invalid */
  if(( col >= MAX_COL ) || ( col < 0 )){
    return false;
  }

  /* cazul in care coloana este plina */
  if( board[0][col] != 0 ){
    return false;
  }

  return true;
}

void print_board(int board[][MAX_COL]){
  for(int i = 0 ; i < MAX_ROW ; i++)
  {
    for(int j = 0 ; j < MAX_COL ; j++)
      {
        if(player_colors[0] == board[i][j]){
          printf(ANSI_COLOR_RED "● " ANSI_COLOR_RESET);
          fflush(stdout);
        }
        else if(player_colors[1] == board[i][j]){
          printf(ANSI_COLOR_GREEN "● " ANSI_COLOR_RESET);
          fflush(stdout);
        }
        else{
          printf("● ");
          fflush(stdout);
        }
      }
    printf("\n");
    fflush(stdout);
  }
}