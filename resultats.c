#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#define TAILLE_BUF 128


void afficher_resultats(char * resultats){
char affichage[TAILLE_BUF];
FILE *res;

res=fopen(resultats, "a+");
while(fgets(affichage,TAILLE_BUF, res)!=NULL){
	printf("%s", affichage);
}

}
