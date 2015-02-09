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

void execution_script(char * buf){
	if (strcmp(buf,"1")==0){
		system("./script/script_1.sh > resultats./resultat_1.txt");
	}else if(strcmp(buf,"2")==0){
		system("./script/script_2.sh > resultats./resultat_2.txt");
	}else if(strcmp(buf,"3")==0){
		system("./script/script_3.sh > resultats./resultat_3.txt");
	}else if(strcmp(buf,"4")==0){
		system("./script/script_4.sh > resultats./resultat_4.txt");
	}else {
		printf("(printf)Je ne connais pas cette commande\n");
	}
	printf("execution terminée\n");
}
