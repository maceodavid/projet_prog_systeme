#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_MATIERES 5
#define MAX_ETUDIANTS 50

typedef struct {
    char nom[50];
    char prenom[50];
    float moyennes[MAX_MATIERES];
} Etudiant;

typedef struct {
    char nom[50];
    char prenom[50];
    int indice_matiere;
} RequeteClient;

typedef struct {
    float moyenne;
    int code_erreur; // 0 si succès, 1 si étudiant non trouvé, 2 si indice invalide
} ReponseServeur;

Etudiant etudiants[MAX_ETUDIANTS]; // Tableau des étudiants
int nb_etudiants = 0; // Nombre d'étudiants enregistrés

void initialiser_etudiants() {
    // Initialisation des étudiants pour l'exemple
    strcpy(etudiants[0].nom, "David");
    strcpy(etudiants[0].prenom, "Maceo");
    etudiants[0].moyennes[0] = 12.5; // Mathématiques
    etudiants[0].moyennes[1] = 14.0; // Physique
    etudiants[0].moyennes[2] = 10.0; // Français
    etudiants[0].moyennes[3] = 15.5; // Histoire-géographie
    etudiants[0].moyennes[4] = 13.0; // Anglais
    nb_etudiants = 1;
}

void traiter_requete(int client_socket) {
    RequeteClient requete;
    ReponseServeur reponse = {0, 0};

    // Lire la requête
    read(client_socket, &requete, sizeof(RequeteClient));

    // Recherche de l'étudiant
    int found = 0;
    for (int i = 0; i < nb_etudiants; i++) {
        if (strcmp(etudiants[i].nom, requete.nom) == 0 &&
            strcmp(etudiants[i].prenom, requete.prenom) == 0) {
            found = 1;
            if (requete.indice_matiere >= 0 && requete.indice_matiere < MAX_MATIERES) {
                reponse.moyenne = etudiants[i].moyennes[requete.indice_matiere];
                reponse.code_erreur = 0;
            } else {
                reponse.code_erreur = 2; // Indice de matière invalide
            }
            break;
        }
    }
    if (!found) {
        reponse.code_erreur = 1; // Étudiant non trouvé
    }

    // Envoyer la réponse
    write(client_socket, &reponse, sizeof(ReponseServeur));
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    initialiser_etudiants();

    // Création de la socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attachement de la socket au port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de connexions...\n");

    // Boucle principale
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Client connecté.\n");
        traiter_requete(client_socket);
        close(client_socket);
        printf("Client déconnecté.\n");
    }

    return 0;
}
