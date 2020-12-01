// compiler header files
#include <stdbool.h> // bool, true, false
#include <stdlib.h> // rand
#include <stdio.h> // printf

// program header file
#include "bomberman.h"

typedef struct Position Position;
struct Position
{
    int x; //Abscisse
    int y; //Ordonée
};

typedef struct Case Case;
struct Case
{  
    char val; //Type de la case
    int dist; //Distance à laquelle il se trouve
    Position pos; //Position de la case la plus proche
    Position posNext; //Position voisine de celle de bomberman où il faut allée pour arrivée le plus rapdement à la case
};


// global declarations
extern const char BOMBERMAN; // ascii used for bomberman
extern const char WALL; // ascii used for the walls
extern const char BREAKABLE_WALL; // ascii used for the breakable walls
extern const char PATH; // ascii used for the pathes
extern const char EXIT; // ascii used for the EXIT
extern const char BOMB; // ascii used for a bomb
extern const char BOMB_BONUS; // ascii used for the bombs
extern const char FLAME_BONUS; // ascii used for the flames
extern const char FLAME_ENEMY; // ascii used for the flame ennemy
extern const char GHOST_ENEMY; // ascii used for the ghost ennemy

extern const int BOMB_DELAY; // time before a bomb explode
extern const int BREAKABLE_WALL_SCORE; // reward for bombing a breakable wall
extern const int FLAME_ENEMY_SCORE; // reward for bombing a flame enemy
extern const int GHOST_ENEMY_SCORE; // reward for bombing a ghost enemy 
extern const int BOMB_BONUS_SCORE; // reward for a bomb bonus, which increases by 1 the number of bombs that can be dropped at a time
extern const int FLAME_BONUS_SCORE; // reward for a flame bonus, which increses by 1 the explosion range of bombs

extern bool DEBUG; // indicates whether the game runs in DEBUG mode

char * binome="Random"; // student names here

// prototypes of the local functions/procedures
void printAction(action);
action actionXY(Position);
action reculer(Position);

//Pour les matrices
int ** matrice(int l,int c);
void freeMatrice(int ** M, int l);
void initMat(int ** M, int tailleX, int tailleY, int val);
void affMat(int ** m, int l, int c);

bool isPos(int x, int y, int mapxsize, int mapysize);
Case plusProche(char * * map, int mapxsize, int mapysize, int x, int y, char type);
Position posPP(int ** distance, char ** map, int tailleX, int tailleY, char type){



/*
  bomberman function:
  This function randomly select a valid move for BOMBERMAN based on its current position on the game map.
 */
action bomberman(
		 char * * map, // array of char modeling the game map
		 int mapxsize, // x size of the map
		 int mapysize, // y size of the map
		 int x, // x position of BOMBERMAN
		 int y, // y position of BOMBERMAN
		 action last_action, // last action made, -1 in the be beginning 
		 int remaining_bombs, // number of bombs
		 int explosion_range // explosion range for the bombs 
		 ) {
  action a; // action to choose and return
  
  //Si l'on vient de poser une bombe, on s'éloigne
  if (last_action == BOMBING){
    a = reculer(plusProche(BOMB).posNext);
  }
  //Si la sortie est visible, on y va
  else if (visible(EXIT))
  {
    //La fonction position retourne la position d'un caractère unique sur la map (utile pour EXIT seulement (?!))
    //La fonction aller renvoie l'action la plus approprié pour aller à une position
    a = aller(position(EXIT));
  }
  //condition de "BOMBING" : atteindre le mur le plus proche (En fait nn, juste atteindre un mur mais éuivalente il me semble) et avoir des Bombe
  else if (isBombable(plusProche(BREAKABLE_WALL).pos) && remaining_bombs > 0)
  {
    a = BOMBING;
  }
  //Si plusde bombe on peut rien faire
  else if(remaining_bombs==0){

  }
  //Dans le cas contraire, on se rapproche d'un mur cassable
  else
  {
    a = aller(plusProche(BREAKABLE_WALL).pos);
  }

if(DEBUG){printAction(a);}
return a; // answer to the game engine
}

/*
  printAction procedure:
  This procedure prints the input action name on screen.
 */
void printAction(action a) {
  switch(a) {
  case BOMBING:
    printf("BOMBING");
    break;
  case NORTH:
    printf("NORTH");
    break;
  case EAST:
    printf("EAST");
    break;
  case SOUTH:
    printf("SOUTH");
    break;
  case WEST:
    printf("WEST");
    break;
  }
}


action reculer(Position posNext, Position me){
  dx = me.x-posNext.x;
  dy = me.y-posNext.y;

  return
}

action actionXY(int x, int y){
  if(x==0 && y==1){
    return NORTH;
  }
  else if(x==0 && y==-1){
    return SOUTH;
  }
  else if(x==1 && y==0){
    return WEST;
  }
  else if(x==-1 && y==0){
    return EAST;
  }
  else{
    printf("Erreur position (x,y) pas dans le domaine de def !\n");
  }
    
}

//***********************Matrices**************************
//Renvoie un pointeur sur pointeur sur int correspondant a un tableau de dim 2
int ** matrice(int l,int c){
  int ** M = NULL;
  M = malloc (sizeof(int*)*l);
  if(M==NULL){
    printf("erreur Allocation Dynamique de la matrice");
    exit(0);
  }
  int i;
  for (i=0; i<l; i++){
    M[i] = malloc (sizeof(int)*c);
    if(M[i]==NULL)
    {
      printf("erreur Allocation Dynamique de la matrice à ligne %d",i);
      exit(0);
    }
  }
  return M;
}

//libère la mémoire en fin d'utilisation
void freeMatrice(int ** M, int l){
  int i;
  for (i=0; i<l; i++){
    free(M[i]);
  }
  free(M);
}

//initialise une matrice
void initMat(int ** M, int tailleX, int tailleY, int val){
  int i,j;
  for(i=0;i<tailleX;i++){
    for(j=0;j<tailleY;j++){
      M[i][j]=val;
    }
  }
}

//Affiche la matrice (pour les debugs)
void affMat(int ** m, int l, int c){
  int i,j;
  for(i=0;i<l;i++){
    for(j=0;j<c;j++){
      printf(" %d ",m[i][j]);
    }
    printf("\n");
  }
}
//*****************************************************************************

//Renvoie vrai si la position (x,y)est bien dans la map
bool isPos(int x, int y, int mapxsize, int mapysize){
  return (x>=0 && x<mapxsize) &&  (y>=0 && y<mapysize);
}

Case plusProche(
    char * * map, // array of char modeling the game map
    int mapxsize, // x size of the map
    int mapysize, // y size of the map
    int x, // x position of BOMBERMAN
    int y // y position of BOMBERMAN)
    char type //Le type de case recherchée (WALL, BREAKABLE_WALL, PATH, EXIT...)
    ){

  //On créé la matrice des distances initialisé à -1 (inateignable) pour toutes les cases sauf celle où l'on est (init à 0)
  int** distance = NULL;
  distance = matrice(mapxsize, mapysize);
  initMat(distance, mapxsize, mapysize, -1);
  distance[x][y] = 0;

  //On créé la matrice des père initialisé à -1 (innateint) pour toutes les cases
  int** pere = NULL;
  pere = matrice(mapxsize, mapysize);
  initMat(pere, mapxsize, mapysize, -1);


  //affMat(distance,mapxsize,mapysize);


  //On va définir une fonction récursive qui va mettre ses voisins inexlorés à une distance +1
  void pars(int x, int y){
    //pour chaque voisin
    int i,j;
    //Case gauche
    i=-1;
    j=0;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 1; //Et on dit qu'il vient de droite (1 pour la droite par notre convention)
      if (map[x+i][y+j]==PATH)
      {
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }

    //Case droite
    i=1;
    j=0;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 2; //Et on dit qu'il vient de gauche (2 pour la gauche par notre convention)
      if(map[x+i][y+j]==PATH){
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }

    //Case haut
    i=0;
    j=1;
    if(isPos(x+i,y+j,mapxsize,mapysize) && map[x+i][y+j]==PATH && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 3;
      if(map[x+i][y+j]==PATH){
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }

    //Case bas
    i=0;
    j=-1;
    if(isPos(x+i,y+j,mapxsize,mapysize) && map[x+i][y+j]==PATH && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 4;
      if(map[x+i][y+j]==PATH){
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }
  }

  pars(x,y);

  //On obtient à ce stade les tableau distance, et pere correctement remplis.
  //Reste à trouver par quelle chemin il faut passer

  Position pp = posPP(distance, map, mapxsize, mapysize, type); //Position du plus proche
  Position next = posN(pp, pere, distance, mapxsize, mapysize); //Position suivante retrouvé grace au tableau pere

  Case pProche;
  pProche.val = map[pp.x][pp.y];
  pProche.dist = distance[pp.x][pp.y];
  pProche.pos = pp;
  pProche.posNext = next;

  //On oublie pas de libérer la mémoire
  freeMatrice(distance,mapxsize);
  freeMatrice(pere,mapxsize);

  return pProche;
}

//Fonction renvoyant la position la plus proche à partir de la matrice des distance
Position posPP(int ** distance, char ** map, int tailleX, int tailleY, char type){
  Position pp;
  int distMin = 1000;
  for (int i = 0; i < tailleX; i++) {
    for (int j = 0; i < tailleY; j++) {
      if(map[i][j]==type && distance[i][j]<distMin){
        distMin = distance[i][j];
        pp.x = x;
        pp.y = y;
      }
    }
  }
  return pp;
}

Position posN(Position pp, int ** pere, int ** distance, int mapxsize, int mapysize){
  int dist = distance[pp.x][pp.y];
  Position pos = pp;
  while(dist>1){ //Boucle FOR mieux ?
    switch 
    }
  }
}