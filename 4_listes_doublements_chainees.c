#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

struct Noeud
{
    int valeur;
    struct Noeud *suivant;
    struct Noeud *precedent;
};

struct Noeud* creationNoeud(int valeur, struct Noeud *Noeudsuivant, struct Noeud *Noeudprecedent){ //Creation d'un nouveau noeud dans la lsite chainée
    struct Noeud* nouvNoeud = (struct Noeud*)malloc(sizeof(struct Noeud));
    nouvNoeud -> valeur = valeur;
    nouvNoeud -> suivant = Noeudsuivant;
    nouvNoeud -> precedent = Noeudprecedent;
    return nouvNoeud;
} 

int longueurliste(struct Noeud* tete){ //Retourne la longueur d'une liste chainée à partir du noeud de tete jusqu'au noeud de queue
    int len = 0;
    struct Noeud* noeud = tete;
    while(noeud != NULL){
        len++;
        noeud = noeud->suivant;
    }
    return len;
}

void afficheliste(struct Noeud* tete){ //Affiche toute la lsite chainée à partir du noeud de tete jusqu'au noeud de fin
    struct Noeud* noeud = tete;
    while(noeud != NULL){
        printf("<%p>:%d\n",(void*)noeud,noeud->valeur);
        noeud = noeud->suivant;
    }
}

struct Noeud* SupprTete(struct Noeud* tete){ //Retire la tete de la liste chainée et le noeud suivant devient la nouvelle tete de la liste chainée
    struct Noeud* temp = tete;
    tete = tete->suivant;
    if (tete != NULL) {
        tete->precedent = NULL; // La nouvelle tête n'a pas de précédent
    }
    free(temp); //libère la zone mémoire
    return tete;
}

void SupprQueue(struct Noeud* tete){ //Retire la queue de la liste chainée et le noeud précédent devient la nouvelle queue de la liste chainée
    struct Noeud* noeud = tete;
    while(noeud->suivant != NULL){
        noeud = noeud->suivant;
    } //On cherche à atteindre la queue de la lsite chainée
    noeud->precedent->suivant = NULL;
    free(noeud);
}

void AjtQueue(struct Noeud* tete){ //Ajoute un nouveau noeud à la queue de la liste chainée
    struct Noeud* noeud = tete;
    while(noeud->suivant != NULL){
        noeud = noeud->suivant;
    }
    struct Noeud* nouvQueue = creationNoeud((noeud->valeur +1),NULL, noeud);
    noeud->suivant = nouvQueue;
}

struct Noeud* AjtTete(struct Noeud* tete){ //Ajoute un nouveau noeud à la tete de la liste chainée
    struct Noeud* nouvTete = creationNoeud(0,tete, NULL);
    if (tete != NULL) {
        tete->precedent = nouvTete; // On met à jour le précédent de l'ancienne tête
    }
    return nouvTete;
}

void concatenelistes(struct Noeud* tete1,struct Noeud* tete2){ //Concatène 2 listes chainées ensemble. La tete de la lsite sera tete2 et la queue de la 
    struct Noeud* noeud = tete1;
    while(noeud->suivant != NULL){
        noeud = noeud->suivant;
    }
    noeud->suivant = tete2; // La queue de 1 pointe vers la tête de 2
    tete2->precedent = noeud; // La tête de 2 pointe vers la queue de 1 en précédent
}

void elevaucarre(struct Noeud* tete){//Fonction élévation au carré de l'ensemble de la lsite chainée
    struct Noeud* noeud = tete;
    while(noeud != NULL){
        noeud->valeur = noeud->valeur * noeud->valeur; //élévation au carré valeur par valeur
        noeud = noeud->suivant;
    }
}

int main() {
    printf("Veuillez insérer un nombre n : ");
    int n;
    scanf("%d", &n);
    struct Noeud* tete = creationNoeud(n,NULL, NULL);
    while(n > 1){
        n--;
        struct Noeud* nouvNoeud = creationNoeud(n, tete, NULL);
        tete->precedent = nouvNoeud;
        tete = nouvNoeud;
    }
    printf("Longeur de la liste chainée : %d \n",longueurliste(tete));
    printf("Liste chainée :\n");
    afficheliste(tete);
    tete = SupprTete(tete);
    SupprQueue(tete);
    printf("Liste chainée avec tete et queue supprimées :\n");
    afficheliste(tete);
    tete = AjtTete(tete);
    AjtQueue(tete);
    printf("Liste chainée avec tete et queue ajoutées (tête passée à la valeur 0) :\n");
    afficheliste(tete);
    struct Noeud* tete2 = creationNoeud(13, NULL, NULL);
    struct Noeud* nouvNoeud2 = creationNoeud(12, tete2, NULL);
    tete2->precedent = nouvNoeud2; tete2 = nouvNoeud2;
    nouvNoeud2 = creationNoeud(11, tete2, NULL);
    tete2->precedent = nouvNoeud2; tete2 = nouvNoeud2;
    nouvNoeud2 = creationNoeud(10, tete2, NULL);
    tete2->precedent = nouvNoeud2; tete2 = nouvNoeud2;
    concatenelistes(tete,tete2);
    printf("Liste chainée avec concatenation :\n");
    afficheliste(tete);
    printf("Liste chainée avec fonction élévation au carrée :\n");
    elevaucarre(tete);
    afficheliste(tete);
    return 0;
}