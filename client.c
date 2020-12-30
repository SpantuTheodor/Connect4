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

#define MAX_ROW 6
#define MAX_COL 7

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
int board[MAX_ROW][MAX_COL];

bool is_valid_movement(int col);

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

  
  // printf("[client] Am citit %d\n",nr);

  // /* trimiterea mesajului la server */
  // if (write (sd,&nr,sizeof(int)) <= 0)
  //   {
  //     perror ("[client]Eroare la write() spre server.\n");
  //     return errno;
  //   }

  // /* citirea raspunsului dat de server 
  //    (apel blocant pina cind serverul raspunde) */
  // if (read (sd, &nr,sizeof(int)) < 0)
  //   {
  //     perror ("[client]Eroare la read() de la server.\n");
  //     return errno;
  //   }
  // /* afisam mesajul primit */
  // printf ("[client]Mesajul primit este: %d\n", nr);

  int turn;
  while(1){
  
    /* citirea mesajului */
    // printf ("[client]Introduceti un numar: ");
    // fflush (stdout);
    // read (0, buf, sizeof(buf));
    // nr=atoi(buf);
 
    /* apel ce blocheaza executia pana in momentul in care incepe tura jucatorului */
    if (read (sd, &turn, sizeof(int)) <= 0)
	 		{
	 		  perror ("[client]Eroare la read() de la server.\n");	
	 		}


    /* trimiterea mesajului ce contine mutarea la server */
    printf("Introduceti numarul coloanei pe care doriti sa puneti piesa: \n");
    fflush(stdout);
    
    do{
      scanf("%d",&col);
    }while(!is_valid_movement(col));

    if (write (sd, &col, sizeof(int)) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }
    }


  /* inchidem conexiunea, am terminat */
  close (sd);
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