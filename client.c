#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>

#define TAILLE_BUF 128
#include "fonction_execution.c"

int sock_i;


/****Gestion des erreurs ****/
void erreur (const char *message) {
	perror(message);
	exit(1);
}

/****Gestion des signaux****/
void arret (int sig) {
	if (close (sock_i) != -1)
		printf("\nDescripteur bien fermé\n");
	exit(0);
}
/****Vider stdin****/
static void purger(void){
	int c;
	while ((c = getchar()) != '\n' && c != EOF){}
}

int main (int argc, char *argv[]) {
	int 	sock_s,
		sock_a,
		newsock_a,
		on=1,
		recherche=1;
	struct sockaddr_in 	addrI,
				addrS,
				addrA;
	char 	buf[TAILLE_BUF]="",
		choix[TAILLE_BUF]="",
		resultats[TAILLE_BUF]="",
		action[TAILLE_BUF]="192.168.157.139",
		a_contacter[TAILLE_BUF]="",
		nom_serveur[TAILLE_BUF]="",
		serveur[TAILLE_BUF]="",
		IP[TAILLE_BUF]="";
	struct sigaction act;
	socklen_t len_addr_client = sizeof(struct sockaddr_in);
	char *ip_serveur;
	FILE *res, *serv;

	/****Quitter proprement par ^C****/
	act.sa_handler = arret;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act,0);

	//obtention d'un desc socket valide pour l'inscription
	sock_i = socket(AF_INET,SOCK_STREAM,0);

	//obtention d'un desc socket valide pour les script
	sock_a = socket(AF_INET,SOCK_STREAM,0);

	//nom du socket du serveur pour les script 
	addrA.sin_family = AF_INET;
	addrA.sin_addr.s_addr = inet_addr("192.168.157.139");
	addrA.sin_port = htons(7001);

	printf("Quel serveur voulez-vous contacter ?\n");
	
	/****Affichage des serveurs connus ****/
	serv=fopen("serveurs_connus.txt", "a+");
	fread(buf, TAILLE_BUF, 1, serv);
	printf("/*****************************/\n");
	printf("%s\n",buf);
	printf("/*****************************/\n");
	rewind(serv);
	/****Récupération de l'IP à contacter ****/
	fgets(a_contacter,18,stdin);
	/****Concaténation de l'ip avec le retour à la ligne ****/
	sprintf(IP, "%s\n",a_contacter);
	/****Parcours du fichier ouvert***/
	
	while(fgets(serveur,TAILLE_BUF,serv)!= NULL && recherche==1){
		/****Si l'IP apparait dans le fichier, on se connecte grâce à l'IP récupérée ****/
		recherche=1;
		if(strcmp(serveur,a_contacter)==0 || strcmp(serveur,IP)==0){
			/****Nom du socket du serveur pour l'inscription****/
			addrI.sin_family = AF_INET;
			addrI.sin_addr.s_addr = inet_addr(IP);
			addrI.sin_port = htons (6000);
			/****Connexion pour l'inscription****/
			if (connect(sock_i,(struct sockaddr *) &addrI,(sizeof(struct sockaddr_in))) == -1)
				erreur ("Connexion ratée pour inscription");
			recherche=0;
			break;
		}
	}
	/****Si on arrive à la fin du fichier et qu'on a pas rencontré l'IP, on la rajoute, et on s'y connacte ****/
	if (recherche==1){
		sprintf(nom_serveur, "%s\n",a_contacter);
		/****nom du socket du serveur pour l'inscription****/
		addrI.sin_family = AF_INET;
		addrI.sin_addr.s_addr = inet_addr(a_contacter);
		addrI.sin_port = htons (6000);
		//connexion pour l'inscription
		if (connect(sock_i,(struct sockaddr *) &addrI,(sizeof(struct sockaddr_in))) == -1){
			erreur ("Connexion ratée pour inscription");
		}else{
		/****On ajoute l'IP au fichier. ****/
		fwrite(nom_serveur,1,16, serv);
		printf("Nouveau serveur inscrit : %s\n", nom_serveur);
		}
	}
	fclose(serv);
	/****On lit ce qui arrive sur la socket****/
	if(read(sock_i,buf,TAILLE_BUF)<0)
		erreur("read");
	printf("%s\n", buf);
	/***Quitter la socket proprement****/
	close (sock_i);

	/****On libère l'adresse IP****/
	setsockopt(sock_a, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/****On lie la structure à la socket****/
	if (bind (sock_a,(struct sockaddr *) &addrA,(sizeof(struct sockaddr_in))) == -1)
		erreur("Erreur de nommage");
	if (getsockname (sock_a,(struct sockaddr *) &addrA,&len_addr_client) == -1) 
		erreur("Erreur lors de la recuperation du port");
	/****Mise en place d'une liste d'attente****/
	listen (sock_a,5);

	/****Boucle pour pouvoir avoir plusieurs demandes à la suite****/
	for(;;){
		/****connexion pour accepter les demandes de scripts****/
		newsock_a = accept (sock_a,(struct sockaddr *) &addrA,&len_addr_client);

		/****Gestion d'une erreur sur le accept ****/
		if (newsock_a == -1) 
			erreur("Connection ratée");
		/****Si on reçoit bien une connexion on va recevoir le script choisi****/
		if(newsock_a>0){
			/****On enregistre l'IP du serveur qui demande le script (pour pouvoir lui répondre)****/
			ip_serveur=inet_ntoa(addrA.sin_addr);
			printf("Une connexion a été établie par %s \n", ip_serveur);
			/****Demande et réception du scan à faire puis envoie de confirmation de bonne réception****/
			if(write(newsock_a,"Quel script voulez-vous executer ?",TAILLE_BUF)<0)
				erreur("write");
			if(read(newsock_a,choix,TAILLE_BUF)<0)
				erreur("read");
			printf("Le serveur a demandé le scan N°%s\n",choix);
			if(strcmp(choix,"1")==0||strcmp(choix,"2")==0||strcmp(choix,"3")==0||strcmp(choix,"4")==0){
			if(write(newsock_a,"Demande reçue et en cours de traitement. Fin de la connexion", TAILLE_BUF)<0)
				erreur("write");
			/***On ferme la socket proprement****/
			close(newsock_a);
		
			/****appel du script****/ 
			execution_script(choix);
			}else{
			printf("Ce script est inconnu. Fermeture du programme.\n");
			if(write(newsock_a,"Script inconnu", TAILLE_BUF)<0)
				erreur("write");
			/***On ferme la socket proprement****/
			close(newsock_a);
			break;
			}
			printf("Pour envoyer les résultats du scan tapez 1\n");
			/****Récupération du choix****/
			fgets(action,2,stdin);
			/****Si on reçoit "1" on va contacter le serveur, sinon on dit qu'on a pas comprit la demande****/
			if(strcmp(action, "1")==0){
				//On vide le buffer
				sock_s = socket(AF_INET,SOCK_STREAM,0);

				addrS.sin_family = AF_INET;
				addrS.sin_addr.s_addr = inet_addr(ip_serveur);
				addrS.sin_port = htons (6001);
				
				if (connect(sock_s,(struct sockaddr *) &addrS,(sizeof(struct sockaddr_in))) == -1)
					erreur ("Connexion ratée pour script");
				/****Ouverture du fichier de résultat en fonction du script demandé****/

				sprintf(resultats, "resultats./resultat_%s.txt",choix);	
				res=fopen(resultats, "r+");
				/****Tant qu'on est pas arrivé à la fin du fichier on envoie les lignes lues ****/

				while(fgets(resultats, TAILLE_BUF, res)!= NULL){
					if(write(sock_s,resultats, TAILLE_BUF)<0)
						erreur("write");
				}
				fclose(res);//On ferme le fichier de résultats
				if(write(sock_s,"Fin du script.", TAILLE_BUF)<0)//On envoie le fait que le script soit terminé.
					erreur("write");
				if(read(sock_s, buf, TAILLE_BUF)<0)
					erreur("read");
				printf("%s \n", buf);

				memset(resultats, 0,TAILLE_BUF);//On ré-initialise la variable "résultats"
				purger();
				/****quitter la socket proprement****/
				close(sock_s);
			}//Fin du If pour l'envoie du résultat
			else{
				printf("Je ne connais pas cette demande \n");
				break;
			}//Fin du Else
		}//Fin du If pour la connexion
	}//Fin du For
}//Fin du main
