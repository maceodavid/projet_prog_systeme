#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define MAX_ETUDIANTS 50
#define MAX_MATIERES 5

typedef struct {
    char nom[50];
    char prenom[50];
    int indice_matiere;
} RequeteClient;

typedef struct {
    float moyenne;
    int code_erreur; // 0 si succès, 1 si étudiant non trouvé, 2 si indice invalide
} ReponseServeur;

typedef struct {
    char nom[50];
    char prenom[50];
    float moyennes[MAX_MATIERES];
} Etudiant;

Etudiant etudiants[MAX_ETUDIANTS];
int nb_etudiants = 0;

pthread_mutex_t mutex_etudiants = PTHREAD_MUTEX_INITIALIZER;

// variable globale pour signaler l'arrêt
volatile sig_atomic_t server_running = 1;

void traiter_requete(int client_socket) {
    RequeteClient requete;
    ReponseServeur reponse = {0, 0};

    if (read(client_socket, &requete, sizeof(RequeteClient)) <= 0) {
        perror("Erreur lors de la lecture de la requête");
        close(client_socket);
        return;
    }

    if (strcmp(requete.nom, "fin") == 0 && strcmp(requete.prenom, "") == 0) {
        printf("Message de fin reçu. Arrêt du serveur.\n");
        server_running = 0; // Indiquer que le serveur doit s'arrêter
        close(client_socket);
        return;
    }

    pthread_mutex_lock(&mutex_etudiants); // Verrouiller l'accès au tableau des étudiants

    int found = 0;
    for (int i = 0; i < nb_etudiants; i++) {
        if (strcmp(etudiants[i].nom, requete.nom) == 0 &&
            strcmp(etudiants[i].prenom, requete.prenom) == 0) {
            found = 1;
            if (requete.indice_matiere >= 0 && requete.indice_matiere < MAX_MATIERES) {
                reponse.moyenne = etudiants[i].moyennes[requete.indice_matiere];
                reponse.code_erreur = 0; // Succès
            } else {
                reponse.code_erreur = 2; // Indice de matière invalide
            }
            break;
        }
    }
    if (!found) {
        reponse.code_erreur = 1; // Étudiant introuvable
    }

    pthread_mutex_unlock(&mutex_etudiants); // déverrouiller l'accès au tableau des étudiants

    write(client_socket, &reponse, sizeof(ReponseServeur));

    close(client_socket);
    printf("Client traité et déconnecté.\n");
}

// Fonction exécutée par chaque thread pour gérer un client
void *gerer_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg); // libérer la mémoire allouée pour le socket
    traiter_requete(client_socket);
    return NULL;
}


void initialiser_etudiants() {
    strcpy(etudiants[0].nom, "David");
    strcpy(etudiants[0].prenom, "Maceo");
    etudiants[0].moyennes[0] = 12.5;
    etudiants[0].moyennes[1] = 14.0;
    etudiants[0].moyennes[2] = 10.0;
    etudiants[0].moyennes[3] = 15.5;
    etudiants[0].moyennes[4] = 13.0;

    strcpy(etudiants[1].nom, "Ravida");
    strcpy(etudiants[1].prenom, "Lelyan");
    etudiants[1].moyennes[0] = 16.0;
    etudiants[1].moyennes[1] = 14.5;
    etudiants[1].moyennes[2] = 12.0;
    etudiants[1].moyennes[3] = 13.5;
    etudiants[1].moyennes[4] = 15.0;

    nb_etudiants = 2; 
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

 
    initialiser_etudiants();


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erreur : création de la socket");
        exit(EXIT_FAILURE);
    }

  
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erreur : bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

  
    if (listen(server_fd, 10) < 0) {
        perror("Erreur : listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (server_running) {
        int *client_socket = malloc(sizeof(int)); // allouer dynamiquement pour chaque client
        if ((*client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            if (!server_running) break; 
            perror("Erreur : accept");
            free(client_socket);
            continue;
        }

        printf("Nouveau client connecté.\n");

        // créer un thread pour gérer le client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, gerer_client, client_socket) != 0) {
            perror("Erreur : création du thread");
            free(client_socket);
            continue;
        }

        // détacher le thread pour éviter les fuites
        pthread_detach(thread_id);
    }

    close(server_fd);
    printf("Serveur arrêté.\n");
    return 0;
}
