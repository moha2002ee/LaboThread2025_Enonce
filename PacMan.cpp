#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "GrilleSDL.h"
#include "Ressources.h"

// Dimensions de la grille de jeu
#define NB_LIGNE 21
#define NB_COLONNE 17

// Macros utilisees dans le tableau tab
#define VIDE 0
#define MUR 1
#define PACMAN 2
#define PACGOM 3
#define SUPERPACGOM 4
#define BONUS 5
#define FANTOME 6

// Autres macros
#define LENTREE 15
#define CENTREE 8

typedef struct
{
  int L;
  int C;
  int couleur;
  int cache;
} S_FANTOME;

typedef struct
{
  int presence;
  pthread_t tid;
} S_CASE;

S_CASE tab[NB_LIGNE][NB_COLONNE];
////////////////////////////////////////////
// Variables globales
////////////////////////////////////////////
pthread_t tidPacMan;
pthread_t tidFantome[4];
pthread_t tidBonus;
int L = LENTREE;
int C = CENTREE;
int dir = GAUCHE;
pthread_mutex_t mutexTab = PTHREAD_MUTEX_INITIALIZER;

/////////////////////
// Fonctions a coder
/////////////////////
void *PacMan(void *arg);
void DessineGrilleBase();
void Attente(int milli);
void setTab(int l, int c, int presence = VIDE, pthread_t tid = 0);
void DeplacePacMan(int direction);

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  EVENT_GRILLE_SDL event;
  sigset_t mask;
  struct sigaction sigAct;
  char ok;

  srand((unsigned)time(NULL));

  // Ouverture de la fenetre graphique
  printf("(MAIN%p) Ouverture de la fenetre graphique\n", pthread_self());
  fflush(stdout);
  if (OuvertureFenetreGraphique() < 0)
  {
    printf("Erreur de OuvrirGrilleSDL\n");
    fflush(stdout);
    exit(1);
  }

  DessineGrilleBase();
  /*
    // Exemple d'utilisation de GrilleSDL et Ressources --> code a supprimer
    DessinePacMan(17,7,GAUCHE);  // Attention !!! tab n'est pas modifie --> a vous de le faire !!!
    DessineChiffre(14,25,9);
    DessineFantome(5,9,ROUGE,DROITE);
    DessinePacGom(7,4);
    DessineSuperPacGom(9,5);
    DessineFantomeComestible(13,15);
    DessineBonus(5,15);
    */
  pthread_create(&tidPacMan, NULL, PacMan, NULL);
  DessinePacMan(LENTREE, CENTREE, DROITE);
 

  ok = 0;
  while (!ok)
  {
    event = ReadEvent();
    if (event.type == CROIX)
      ok = 1;
    if (event.type == CLAVIER)
    {
      switch (event.touche)
      {
      case 'q':
        ok = 1;
        break;
      case KEY_RIGHT: 
        printf("Fleche droite !\n");
        dir = DROITE;
        break;
      case KEY_LEFT:
        printf("Fleche gauche !\n");
        dir = GAUCHE;
        break;
      case KEY_UP:
        printf("Fleche haut !\n");
        dir = HAUT;
        break;
      case KEY_DOWN:
        printf("Fleche bas !\n");
        dir = BAS;
        break;
      }
    }
  }
  printf("Attente de 1500 millisecondes...\n");
  Attente(1500);
  // -------------------------------------------------------------------------

  // Fermeture de la fenetre
  printf("(MAIN %p) Fermeture de la fenetre graphique...", pthread_self());
  fflush(stdout);
  FermetureFenetreGraphique();
  printf("OK\n");
  fflush(stdout);

  exit(0);
}

//*********************************************************************************************
void Attente(int milli)
{
  struct timespec del;
  del.tv_sec = milli / 1000;
  del.tv_nsec = (milli % 1000) * 1000000;
  nanosleep(&del, NULL);
}

//*********************************************************************************************
void setTab(int l, int c, int presence, pthread_t tid)
{
  tab[l][c].presence = presence;
  tab[l][c].tid = tid;
}

//*********************************************************************************************
void DessineGrilleBase()
{
  int t[NB_LIGNE][NB_COLONNE] = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                 {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
                                 {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1},
                                 {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                                 {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1},
                                 {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
                                 {1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1},
                                 {1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1},
                                 {1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1},
                                 {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
                                 {1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1},
                                 {1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1},
                                 {1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1},
                                 {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
                                 {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1},
                                 {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
                                 {1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1},
                                 {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
                                 {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1},
                                 {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                                 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

  for (int l = 0; l < NB_LIGNE; l++)
    for (int c = 0; c < NB_COLONNE; c++)
    {
      if (t[l][c] == VIDE)
      {
        setTab(l, c);
        EffaceCarre(l, c);
      }
      if (t[l][c] == MUR)
      {
        setTab(l, c, MUR);
        DessineMur(l, c);
      }
    }
}

//*********************************************************************************************
void *PacMan(void *arg)
{
  while (1)
  {
    pthread_mutex_lock(&mutexTab);

    // Vérifie si Pac-Man peut se déplacer dans la direction actuelle
    int newL = L;
    int newC = C;
    switch (dir)
    {
    case GAUCHE:
      if (tab[L][C - 1].presence != MUR)
        newC--;
      break;
    case DROITE:
      if (tab[L][C + 1].presence != MUR)
        newC++;
      break;
    case HAUT:
      if (tab[L - 1][C].presence != MUR)
        newL--;
      break;
    case BAS:
      if (tab[L + 1][C].presence != MUR)
        newL++;
      break;
    }

    // Efface l'ancienne position de Pac-Man
    EffaceCarre(L, C);
    setTab(L, C, VIDE);

    // Met à jour la position de Pac-Man si elle a changé
    if (newL != L || newC != C)
    {
      L = newL;
      C = newC;
    }

    // Dessine Pac-Man à la nouvelle position
    DessinePacMan(L, C, dir);
    setTab(L, C, PACMAN);

    pthread_mutex_unlock(&mutexTab);

    // Attente de 0,3 secondes
    Attente(300);
  }

  return NULL;
}

//*********************************************************************************************
void DeplacePacMan(int direction)
{
  pthread_mutex_lock(&mutexTab);

  // Efface l'ancienne position de Pac-Man
  EffaceCarre(L, C);
  setTab(L, C, VIDE);

  // Met à jour la position de Pac-Man en fonction de la direction
  switch (direction)
  {
  case GAUCHE:
    if (tab[L][C - 1].presence != MUR)
      C--;
    break;
  case DROITE:
    if (tab[L][C + 1].presence != MUR)
      C++;
    break;
  case HAUT:
    if (tab[L - 1][C].presence != MUR)
      L--;
    break;
  case BAS:
    if (tab[L + 1][C].presence != MUR)
      L++;
    break;
  }

  // Dessine Pac-Man à la nouvelle position
  DessinePacMan(L, C, direction);
  setTab(L, C, PACMAN);

  pthread_mutex_unlock(&mutexTab);
}