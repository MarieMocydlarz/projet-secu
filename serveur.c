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
#include <sys/stat.h>


#include "resultats.c"

#define TAILLE_BUF 128
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int sock_i;
int sock_cs;

/****Gestion des erreurs ****/
void erreur (const char *message) {
	perror(message);
	exit(1);
}

/****Gestion des signaux ****/
void arret (int sig) {
	if (close (sock_i) != -1)
		printf("\nDescripteur bien fermé\n");
	exit(0);
}

static void finish(){
	close(sock_cs);
}

static void child_handler(int signum)
{	
	switch(signum) {
		case SIGALRM: exit(EXIT_FAILURE); break;
		case SIGUSR1: exit(EXIT_SUCCESS); break;
		case SIGCHLD: exit(EXIT_FAILURE); break;
		case SIGTERM: finish(); exit(EXIT_SUCCESS); break;
	}
}

/****Vider stdin****/
static void purger(void){
	int c;
	while ((c = getchar()) != '\n' && c != EOF){}
}


int main (int argc, char *argv[]) { 

	/***Déclaration des variables***/
	int 	sock_s,
		newsock_i, 
		newsock_cs,
		choix,
		on=1,
		recherche=1;
	socklen_t len, len_addr_client = sizeof(struct sockaddr_in);
	struct sockaddr_in 	addrS,
				addrC,
				addrCS,	
				addra_contacter;
	char 	*buf = (char*) malloc (TAILLE_BUF),
		*ip; 
	struct sigaction act;
	FILE *fp, *res;
	char 	client[TAILLE_BUF]="",
		action[TAILLE_BUF]="", 
		nom_client[TAILLE_BUF]="", 
		IP[TAILLE_BUF]="", 
		a_contacter[TAILLE_BUF]="", 
		resultats[TAILLE_BUF] = "";

	/****Quitter proprement par ^C****/
	act.sa_handler = arret;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act,0);

	/***Ignorer la mort des fils***/
	signal(SIGCHLD,SIG_IGN);

	/***Serveur pour l'inscription***/
	addrS.sin_family = AF_INET;
	addrS.sin_addr.s_addr = inet_addr("192.168.157.129");
	addrS.sin_port = htons(6000); 

	sock_i = socket(AF_INET,SOCK_STREAM,0);

	/****On libère l'adresse IP****/
	setsockopt(sock_i, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind (sock_i,(struct sockaddr *) &addrS,(sizeof(struct sockaddr_in))) == -1)
		erreur("Erreur de nommage"); 

	if (getsockname (sock_i,(struct sockaddr *) &addrS,&len) == -1) 
		erreur("Erreur lors de la recuperation du port");
	for(;;) {

		/****Mise en place d'une liste d'attente****/
		listen (sock_i,5);

		printf("Serveur en attente\n");

		newsock_i = accept (sock_i,(struct sockaddr *) &addrC,&len_addr_client);
		if (newsock_i == -1) 
			erreur("Connection ratée");

		/**** Gestion des signaux ****/
		signal(SIGCHLD,child_handler);
		signal(SIGUSR1,child_handler);
		signal(SIGALRM,child_handler);
		signal(SIGTERM,child_handler);

		switch (fork()) {
			case -1:
				erreur ("Duplication ratée");
			case 0:

				/*****Inscription d'un client******/
				fp=fopen("clients_connus.txt", "a+");
				ip=inet_ntoa(addrC.sin_addr);
				sprintf(IP, "%s\n",ip);
				/****Parcours du fichier ouvert***/
				while(fgets(client,TAILLE_BUF,fp)!= NULL && recherche==1){
					/****Si l'IP apparait dans le fichier, on envoie que le client est déjà inscrit ****/
					recherche=1;
					if(strcmp(client,ip)==0 || strcmp(client,IP)==0){
						if(write(newsock_i,"Vous êtes déjà inscrit",TAILLE_BUF)<0)
							erreur("write");
						printf("Client connu\n");
						recherche=0;
						break;
					}
				
				}
				/****Si on arrive à la fin du fichier sans rencontrer l'IP, on l'ajoute et on envoie au client qu'il est bien inscrit****/
				if (recherche==1){
					sprintf(nom_client, "%s\n",ip);
					fwrite(nom_client,1,16,fp);
					printf("Nouveau client inscrit : %s\n", nom_client);
					if(write(newsock_i,"Inscription réalisée avec succès",TAILLE_BUF)<0)
						erreur("write");
					if(mkdir(ip, 0777)<0)
						erreur("mkdir");
				}
				fclose(fp);
				/*****Fin inscription d'un client******/
				/****Boucle pour pouvoir avoir plusieurs demandes à la suite****/
				for(;;){
					sock_s = socket(AF_INET,SOCK_STREAM,0);
					printf("\nPour demander un scan à un client tapez 1\nPour afficher le dernier résultat reçu tapez 2 (imposible si aucun scan n'a été demandé auparavant.)\n");
					fgets(action, 2, stdin);

					if(strcmp(action, "1")==0){
						purger();
						printf("Quel client voulez-vous contacter ?\n");
						fp=fopen("clients_connus.txt", "r+");
						fread(buf, TAILLE_BUF, 1, fp);
						printf("/*****************************/\n");
						printf("%s\n",buf);
						printf("/*****************************/\n");
						fclose(fp);
						fgets(a_contacter,TAILLE_BUF,stdin);
						addra_contacter.sin_family = AF_INET;
						addra_contacter.sin_addr.s_addr=inet_addr(a_contacter);
						addra_contacter.sin_port = htons(7001); 
						
						if (connect(sock_s,(struct sockaddr *) &addra_contacter,(sizeof(struct sockaddr_in))) == -1)
							erreur ("Connexion ratée pour les Scripts");

						if(read(sock_s,buf,TAILLE_BUF)<0)
							erreur("read");
						printf("Message recu de %s : %s \n",inet_ntoa(addrC.sin_addr),buf);

						printf("Scans disponibles :\n\t1.Connaitre les informations du clients (hostname, version, adresse IP, etc...) \n\t2. Connaitre les noms des utilisateurs de la machine \n\t3. Savoir quels paquets sont installés et quels paquets sont à mettre à jour \n\t4. Connaitre la date et l'heure de la dernière connexion locale\n\n"); 
						//On attend le choix
						scanf("%d",&choix);
						//On met le choix au bon format
						sprintf(buf,"%d",choix);
						//on envoit le choix à l'esclave
						if(write(sock_s,buf,TAILLE_BUF)<0)
							erreur("write");
						if(read(sock_s,buf,TAILLE_BUF)<0)
							erreur("read");
						printf("%s \n",buf);
						if(buf=="Script inconnu"){
							printf("Connexion coupée\n");
						}
						close(sock_s);

						sock_cs = socket(AF_INET,SOCK_STREAM,0);

						on=1;
						if(setsockopt(sock_cs, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0)
							erreur("setsock"); 

						addrCS.sin_family = AF_INET;
						addrCS.sin_addr.s_addr = inet_addr("192.168.157.129");
						addrCS.sin_port = htons(6001);
 						
						if (bind (sock_cs,(struct sockaddr *) &addrCS,(sizeof(struct sockaddr_in))) == -1)
							erreur("Erreur de nommage"); 
						if (getsockname (sock_cs,(struct sockaddr *) &addrCS,&len_addr_client) == -1) 
							erreur("Erreur lors de la recuperation du port");
						if(listen (sock_cs,5)<0)
							erreur("listen");

						newsock_cs = accept (sock_cs,(struct sockaddr *) &addrCS,&len_addr_client);

						if (newsock_cs == -1)
							erreur("Connection ratée");

						sprintf(buf,"%d",choix);
						ip=inet_ntoa(addrCS.sin_addr);

 						sprintf(resultats,"%s/resultat_%s.txt",ip,buf);
						res=fopen(resultats, "a+");
						while (strcmp(buf,"Fin du script.")!=0){
							if(read(newsock_cs,buf,TAILLE_BUF)<0)
								erreur("read");
							fputs(buf, res);
						}
						fclose(res);
						//on envoie la confirmation de réception des résultats
						if(write(newsock_cs,"Les résultats ont bien été reçus. Fin de la connexion.", TAILLE_BUF)<0)
							erreur("write");

						printf("\nConnexion terminée\n");
						purger();
						memset(buf,0,TAILLE_BUF);
						close(newsock_cs);
					}
					else if(strcmp(action, "2")==0){
						printf("/*****************************/\n");
						afficher_resultats(resultats);
						printf("\n/*****************************/\n");
						purger();
					}else{
						purger();
						printf("Je ne connais pas cette demande\n");
						break;
					}
				}
			default:
				break;

		}
		/* Ignorer certains signaux */
		signal(SIGCHLD,SIG_DFL); 
		signal(SIGTSTP,SIG_IGN); 
		signal(SIGTTOU,SIG_IGN);
		signal(SIGTTIN,SIG_IGN);
		signal(SIGHUP, SIG_IGN);
	}
	close(newsock_i);
}
