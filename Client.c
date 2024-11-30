#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

typedef struct {
    char nom[50];
    char prenom[50];
    int indice_matiere;
} RequeteClient;

typedef struct {
    float moyenne;
    int code_erreur; // 0 si succès, 1 si étudiant non trouvé, 2 si indice invalide
} ReponseServeur;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    RequeteClient requete;
    ReponseServeur reponse;

    // Créer la socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erreur : la création de la socket a échoué\n");
        return -1;
    }

    // Configurer l'adresse du serveur
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir l'adresse IP du serveur en format binaire
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Erreur : adresse IP invalide ou non supportée\n");
        return -1;
    }

    // Se connecter au serveur
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Erreur : connexion au serveur échouée\n");
        return -1;
    }

    // Remplir la requête
    printf("Entrez le nom de l'étudiant : ");
    scanf("%s", requete.nom);
    printf("Entrez le prénom de l'étudiant : ");
    scanf("%s", requete.prenom);
    printf("Entrez l'indice de la matière (0-4) : ");
    scanf("%d", &requete.indice_matiere);

    // Envoyer la requête au serveur
    send(sock, &requete, sizeof(RequeteClient), 0);

    // Recevoir la réponse du serveur
    read(sock, &reponse, sizeof(ReponseServeur));

    // Afficher la réponse
    if (reponse.code_erreur == 0) {
        printf("La moyenne de %s %s pour la matière %d est : %.2f\n",
               requete.nom, requete.prenom, requete.indice_matiere, reponse.moyenne);
    } else if (reponse.code_erreur == 1) {
        printf("Erreur : Étudiant introuvable\n");
    } else if (reponse.code_erreur == 2) {
        printf("Erreur : Indice de matière invalide\n");
    }

    // Fermer la connexion
    close(sock);

    return 0;
}
