/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

/* portul folosit */
#define PORT 2908
#define MAX_ROW 6
#define MAX_COL 7

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int number_of_players;

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

int board[MAX_ROW][MAX_COL];

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void prepare_board_for_new_game();
void print_board(int board[][MAX_COL]);
int make_movement(int col, int id_player);
bool is_winning_state(int id_player);
bool is_valid_movement(int col);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;
  number_of_players ++;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
  int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);

  char buf[10];
  int row;
  int col;
  int id_player = tdL.idThread + 1;
  bool winning_condition = false;
  bool valid_string_condition;

  //bool status = pthread_mutex_init(&lock, NULL);
  int turn = id_player;
  printf("%d", tdL.cl);
  while(1){
      
    prepare_board_for_new_game();
    winning_condition = false;

    while(number_of_players != 2);
    while(!winning_condition){
        
      system("clear");
      print_board(board);
      sleep(1);

      pthread_mutex_lock(&lock);

      if (write (tdL.cl,&board,sizeof(board)) <= 0)
        {
          perror ("[client]Eroare la write() spre client.\n");
        }

    do{
        
      if (read (tdL.cl, &col,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");	
      }
        
      valid_string_condition = is_valid_movement(col);

      if (write (tdL.cl,&valid_string_condition,sizeof(bool)) <= 0)
      {
        perror ("[client]Eroare la write() spre client.\n");
      }

    }while(!valid_string_condition);

    if(row = make_movement(col,id_player)){
        
      if (write (tdL.cl,&board,sizeof(board)) <= 0)
      {
        perror ("[client]Eroare la write() spre client.\n");
      }
        
      if(is_winning_state(id_player) == true){
        winning_condition = true;
        system("clear");
        printf("CASTIG");
        fflush(stdout);
        sleep(5);
      }
    }
    else{
      printf("Esti vagabont \n");
    }
        
    pthread_mutex_unlock(&lock);

    }
  }
}

void prepare_board_for_new_game(){
  for(int i = 0 ; i < MAX_ROW ; i++)
    for(int j = 0 ; j < MAX_COL ; j++)
      {
        board[i][j] = 0;
      }
}

void print_board(int board[][MAX_COL]){
  for(int i = 0 ; i < MAX_ROW ; i++)
  {
    for(int j = 0 ; j < MAX_COL ; j++)
      {
        printf("%d ",board[i][j]);
        fflush(stdout);
      }
    printf("\n");
    fflush(stdout);
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

int make_movement(int col, int id_player){

  /*cazul in care coloana nu este nici plina nici goala */
  int i = 1;  
  while( i < MAX_ROW){
    if( board[i][col] != 0 ){
      board[i-1][col] = id_player;
      return i-1;
    }
    i++;
  }
  
  /* cazul in care coloana este goala */
  board[i-1][col] = id_player;
  return i-1;

}

bool is_winning_state(int id_player){

  int dx[] = {0, 0, 0, -1, -2, -3, -1, -2, -3, -1, -2, -3};
  int dy[] = {1, 2, 3, 1, 2, 3, 0, 0, 0, -1, -2, -3};

  for(int row = 0; row < MAX_ROW ; row++){
    for(int col = 0; col < MAX_COL ; col++){
      if(board[row][col] == id_player){
        int i=0;
        bool conditie;
        while(i<sizeof(dx)){
          conditie = true;
          for(int j = i ; j < i+3 ; j++){
            if (((row + dx[j] < MAX_ROW) && (row + dx[j] >= 0)) && ((col + dy[j] < MAX_COL) && (col + dy[j] >= 0))){
              if(board[row + dx[j]][col + dy[j]] != id_player)
                conditie = false;
            }
            else conditie = false;
          }
          if(conditie == true){
            return true;
          }
          i+=3;
        }
      }
    }
  }

  return false;

}