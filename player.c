// compiler header files
#include <stdbool.h> // bool, true, false
#include <stdlib.h> // rand
#include <stdio.h> // printf

// program header file
#include "bomberman.h"

//Structure position
typedef struct Position Position;
struct Position
{
    int x; //Abscisse
    int y; //Ordonée
};

//Structure case
typedef struct Case Case;
struct Case
{  
    bool existe; //Bool indiquant si le type recherché est(visible) sur la map
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

char * binome="GUDIN Félix - DUQUÉ Loukas"; // student names here

// prototypes of the local functions/procedures
void printAction(action);
action actionXY(Position);
bool secu(int x, int y, char ** map, int mapx, int mapy);
action reculer(Position, Position, char **, int, int);
action aller(Position, Position);
bool isBombable(Case c);

//Pour les matrices
int ** matrice(int l,int c);
void freeMatrice(int ** M, int l);
void initMat(int ** M, int tailleX, int tailleY, int val);
void affMat(int ** m, int l, int c);
void affMap(char ** m, int l, int c);

bool isPos(int x, int y, int mapxsize, int mapysize);
bool isAccess(int x, int y, char ** map);
Case plusProche(int ** distance, int ** pere, char * * map, int mapxsize, int mapysize, int x, int y, char type);
void posPP(int ** distance, char ** map, int tailleX, int tailleY, char type, Position * pp, bool * trouve);
Position posN(Position pp, int ** pere, int ** distance, int mapxsize, int mapysize);

void Init_tabs(int ** * d, int ** * p, int x, int y, char ** map, int mapxsize, int mapysize);

void affMap(char ** m, int l, int c);




/*
  bomberman function:
  This function randomly select a valid move for BOMBERMAN based on its current position on the game map.
 */
action bomberman(
		 char * * map, // array of char modeling the game map
		 int mapysize, // x size of the map
		 int mapxsize, // y size of the map
		 int y, // x position of BOMBERMAN
		 int x, // y position of BOMBERMAN
		 action last_action, // last action made, -1 in the be beginning 
		 int remaining_bombs, // number of bombs
		 int explosion_range // explosion range for the bombs 
		 ) {
  //printf("debut tour\n");

  action a; // action to choose and return
  int distMonstre = 1;
  int margeSecuBomb = 1;

  //On créé la matrice des distances initialisé à -1 (inateignable) pour toutes les cases sauf celle où l'on est (init à 0)
  int** distance = NULL;
  int** pere = NULL;
  Init_tabs(&distance, &pere, x,y,map,mapxsize,mapysize);

  Case exit = plusProche(distance, pere, map, mapxsize, mapysize, x, y, EXIT);
  Case break_wall = plusProche(distance, pere, map, mapxsize, mapysize, x, y, BREAKABLE_WALL);
  Case bomb = plusProche(distance, pere, map, mapxsize, mapysize, x, y, BOMB);
  Case ghost = plusProche(distance, pere, map, mapxsize, mapysize, x, y, GHOST_ENEMY);
  Case flamme = plusProche(distance, pere, map, mapxsize, mapysize, x, y, FLAME_ENEMY);

  Position me;
  me.x = x;
  me.y = y;


  if(DEBUG){printf("Initialisation ok\n");}

  //Si l'on vient de poser une bombe, on s'éloigne
  if (last_action == BOMBING || (bomb.dist !=-1 && bomb.dist <= explosion_range+margeSecuBomb) ){
    if(DEBUG){
      printf("%d\n", last_action);
      printf("bomb dist : %d\n", bomb.dist);
      printf("CAS 1\n");
    }
    Case caseAFuire;
    if(last_action == BOMBING){
      caseAFuire = break_wall;
    }else{
      caseAFuire = bomb;
    }
    a = reculer(caseAFuire.posNext, me, map, mapxsize, mapysize);
  }

  else if(ghost.existe && ghost.dist <= distMonstre)
  {
    reculer(ghost.posNext, me, map, mapxsize, mapysize);
  }
  else if(flamme.existe && flamme.dist <= distMonstre)
  {
    reculer(flamme.posNext, me, map, mapxsize, mapysize);
  }
  //Si la sortie est visible, on y va
  else if (exit.dist != -1)
  {
    //La fonction aller renvoie l'action la plus approprié pour aller à une position
    if(DEBUG) printf("CAS 2 exit\n");
    a = aller(exit.posNext,me);
  }
  //condition de "BOMBING" : atteindre le mur le plus proche (En fait nn, juste atteindre un mur mais éuivalente il me semble) et avoir des Bombe
  else if (isBombable(break_wall) && remaining_bombs > 0)
  {
    if(DEBUG) printf("CAS 3 bombing\n");
    a = BOMBING;
  }
  /*Si plusde bombe on va en cherché
  else if(remaining_bombs==0){
    if (!bombBonus.existe) printf("t'es dans la merde\n");
    a = aller(bombBonus.posNext,me);
  }*/
  //Dans le cas contraire, on se rapproche d'un mur cassable
  else if(break_wall.dist>1)
  {
    if(DEBUG){
      printf("CAS 5 move \n");
      printf("posNext %d,%d\n", break_wall.posNext.x,break_wall.posNext.y);
    }
    
    a = aller(break_wall.posNext,me);
  }
  else
  {
    if(DEBUG) printf("Cas 6, gange temps et recule\n");
    a = reculer(break_wall.posNext, me, map, mapxsize, mapysize);
  }

freeMatrice(distance,mapxsize);
freeMatrice(pere,mapxsize);
//printf("nbombs = %d\n", remaining_bombs);

//printf("act = ");
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


bool secu(int x, int y, char ** map, int mapxsize, int mapysize){
  bool test=true;
  int compteur=0;
  if (isPos(x+1,y, mapxsize, mapysize)){
    if(!isAccess(x+1,y,map)) compteur=compteur+1;
  }
  else {
    compteur=compteur+1;
  }
    
  if (isPos(x-1,y, mapxsize, mapysize)){
    if(!isAccess(x-1,y,map)) compteur=compteur+1;
  }
  else {
    compteur=compteur+1;
  }
    
  if (isPos(x,y+1, mapxsize, mapysize)){
    if(!isAccess(x,y+1,map)) compteur=compteur+1;
  }
  else {
    compteur=compteur+1;
  }
    
  if (isPos(x,y-1, mapxsize, mapysize)){
    if(!isAccess(x,y-1,map)) compteur=compteur+1;
  }
  else {
    compteur=compteur+1;
  }

  if (compteur>3) test=false;
  return test;
}

action reculer(Position posNext, Position me, char ** map, int mapxsize, int mapysize){

  int x,y;
  action a;
  bool ok;
  
  do {
    a=rand()%4+1; // ramdomly select an action: 0=BOMBING, 1=NORTH,...

    if(DEBUG) { // print the randomly selected action, only in DEBUG mode
      //printf("Candidate action is: ");
      //printAction(a);
      //printf("\n");
    }

    switch(a) { // check whether the randomly selected action is valid, i.e., if its preconditions are satisfied 
    case NORTH:
      x=me.x-1;
      y=me.y;
      
      if(isPos(x,y,mapxsize,mapysize) && isAccess(x,y,map) && !(x==posNext.x && y==posNext.y) && secu(x, y, map, mapxsize, mapysize)) ok=true;
      break;
    case EAST:
      x=me.x;
      y=me.y+1;
      
      if(isPos(x,y,mapxsize,mapysize) && isAccess(x,y,map) && !(x==posNext.x && y==posNext.y) && secu(x, y, map, mapxsize, mapysize)) ok=true;
      break;
    case SOUTH:
      x=me.x+1;
      y=me.y;
      
      if(isPos(x,y,mapxsize,mapysize) && isAccess(x,y,map) && !(x==posNext.x && y==posNext.y) && secu(x, y, map, mapxsize, mapysize)) ok=true;
      break;
    case WEST:
      x=me.x;
      y=me.y-1;
      
      if(isPos(x,y,mapxsize,mapysize) && isAccess(x,y,map) && !(x==posNext.x && y==posNext.y) && secu(x, y, map, mapxsize, mapysize)) ok=true;
      break;
    default:
      printf("On a un pb dans reculer\n");
      break;
    }
  } while(!ok);
  if(DEBUG) printAction(a);

  return a;
}

action aller(Position p, Position me){
  Position pos;
  pos.x = p.x-me.x;
  pos.y = p.y-me.y;
  return actionXY(pos);
}

action actionXY(Position p){
  int x = p.x;
  int y = p.y;

  action a;

  if(x==0 && y==1){
    a = EAST;
  }
  else if(x==0 && y==-1){
    a = WEST;
  }
  else if(x==1 && y==0){
    a = SOUTH;
  }
  else if(x==-1 && y==0){
    a = NORTH;
  }
  else{
    printf("Erreur position (x,y) pas dans le domaine de def !\n");
    printf("x = %d et y= %d\n", x,y);
    a=BOMBING;
  }
  return a;    
}

//On fait simple, on peut seulement si on est a coté
bool isBombable(Case c){
  return c.dist == 1;
}

///**********************************************************
//Matrice

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
void initMat(int ** M, int tLi, int tCol, int val){
  int i,j;
  for(i=0;i<tLi;i++){
    for(j=0;j<tCol;j++){
      M[i][j]=val;
    }
  }
}

//Affiche la matrice (pour les debugs)
void affMat(int ** m, int l, int c){
  int i,j;
  for(i=0;i<l;i++){
    for(j=0;j<c;j++){
      if(m[i][j]<0){
        printf("* ");
      }else{
        printf("%d ",m[i][j]);
      }
    }
    printf("\n");
  }
}


//*****************************************************************************

//Renvoie vrai si la position (x,y)est bien dans la map
bool isPos(int x, int y, int mapxsize, int mapysize){
  return (x>=0 && x<mapxsize-1) &&  (y>=0 && y<mapysize-1);
}

bool isAccess(int x, int y, char ** map){
  bool ok = false;
  char val = map[x][y];
  if(val == PATH) ok = true;
  if(val == FLAME_BONUS) ok = true;
  if(val == BOMB_BONUS) ok = true;

  return ok;
}

Case plusProche(
    int ** distance,
    int ** pere,
    char * * map, // array of char modeling the game map
    int mapxsize, // x size of the map
    int mapysize, // y size of the map
    int x, // x position of BOMBERMAN
    int y, // y position of BOMBERMAN)
    char type //Le type de case recherchée (WALL, BREAKABLE_WALL, PATH, EXIT...)
    ){

  //printf("tab ok\n");

  //On obtient à ce stade les tableau distance, et pere correctement remplis.
  //Reste à trouver par quelle chemin il faut passer

  if(DEBUG) affMat(distance, mapxsize, mapysize);
  //printf("\n");
  //affMat(pere, mapxsize, mapysize);

  //printf("\ndeb ok\n");


  Position pp;
  pp.x=0;
  pp.y=0;
  bool trouve = false;
  posPP(distance, map, mapxsize, mapysize, type, &pp, &trouve); //Position du plus proche
  if(DEBUG) printf("pp ok\n");

  Position next = posN(pp, pere, distance, mapxsize, mapysize); //Position suivante retrouvé grace au tableau pere
  if(DEBUG) printf("posN ok\n");



  Case pProche;

  pProche.existe = trouve;
  pProche.val = map[pp.x][pp.y];
  if(DEBUG) printf("val = %c\n", pProche.val);
  pProche.dist = distance[pp.x][pp.y];
  if(DEBUG) printf("d = %d\n", pProche.dist);

  pProche.pos = pp;
  if(DEBUG) printf("pos = %d,%d\n", pProche.pos.x,pProche.pos.y);
  pProche.posNext = next;
  if(DEBUG) printf("posNext = %d,%d\n", pProche.posNext.x,pProche.posNext.y);

  
  return pProche;
}

//PROCEDURE renvoyant la position la plus proche à partir de la matrice des distance
void posPP(int ** distance, char ** map, int tailleX, int tailleY, char type, Position * pp, bool * trouve){
  int distMin = 1000;
  
  int i,j;
  for (i = 0; i < tailleX; i++) {
    for (j = 0; j < tailleY; j++) {
      if(map[i][j]==type && distance[i][j]<distMin && distance[i][j]!=-1 && distance[i][j]!=0){
        distMin = distance[i][j];
        (*pp).x = i;
        (*pp).y = j;
        *trouve = true;
      }
    }
  }
}

//Fonction renvoyant la position où il faut aller pour aller vers la case pp trouvé en remontant le tableau des peres.
Position posN(Position pp, int ** pere, int ** distance, int mapxsize, int mapysize){
  int dist = distance[pp.x][pp.y];
  //affMat(distance,mapxsize,mapysize);
  //affMat(pere,mapxsize,mapysize);
  Position pos = pp;
  //printf("x,y = %d,%d\n", pos.x,pos.y);
  while(dist>1){ //Boucle FOR mieux ?
    switch(pere[pos.x][pos.y]) {
      //Viens de droite
      case 1:
        pos.x = pos.x+1; //On décale notre position à droite
        break;
      //Viens de gauche
      case 2:
        pos.x = pos.x-1; //On décale notre position à gauche
        break;
      //Viens du bas
      case 3:
        pos.y = pos.y-1; //On décale notre position en bas
        break;
      //Viens du haut
      case 4:
        pos.y = pos.y+1; //On décale notre position en haut
        break;
      //Sinon
      default:
        printf("Erreur, ou t'es papa ou t'es ?\n");
        break;
    }
    dist = dist - 1;
  }
  return pos;
}

//***************************
void affMap(char ** m, int l, int c){
  int i,j;
  for(i=0;i<l;i++){
    for(j=0;j<c;j++){
      printf(" %c ",m[i][j]);
    }
    printf("\n");
  }
}

//***
void Init_tabs(int ** * d, int ** * p, int x, int y, char ** map, int mapxsize, int mapysize){

  int** distance = NULL;
  distance = matrice(mapxsize, mapysize);
  initMat(distance, mapxsize, mapysize, -1);
  distance[x][y] = 0;

  int** pere = NULL;
  pere = matrice(mapxsize, mapysize);
  initMat(pere, mapxsize, mapysize, -2);


  //printf("ini ok\n");


  //On va définir une fonction récursive qui va mettre ses voisins inexlorés à une distance +1
  void pars(int x, int y){
    //printf("\n");
    //affMat(distance,mapxsize,mapysize);
    //pour chaque voisin
    int i,j;
    //Case gauche
    i=-1;
    j=0;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 1; //Et on dit qu'il vient de droite (1 pour la droite par notre convention)
      if (map[x+i][y+j]==PATH || map[x+i][y+j]==BOMB_BONUS || map[x+i][y+j]==FLAME_BONUS)
      {
        //printf("gauche\n");
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }

    //Case droite
    i=1;
    j=0;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 2; //Et on dit qu'il vient de gauche (2 pour la gauche par notre convention)
      if(map[x+i][y+j]==PATH || map[x+i][y+j]==BOMB_BONUS || map[x+i][y+j]==FLAME_BONUS){
        //printf("droite\n");
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }

    //Case haut
    i=0;
    j=1;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 3;
      if(map[x+i][y+j]==PATH || map[x+i][y+j]==BOMB_BONUS || map[x+i][y+j]==FLAME_BONUS){
        //printf("haut\n");
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }


    //Case bas
    i=0;
    j=-1;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = 4;
      if(map[x+i][y+j]==PATH || map[x+i][y+j]==BOMB_BONUS || map[x+i][y+j]==FLAME_BONUS){
        //printf("bas\n");
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }
  }

  pars(x,y);

  *p = pere;
  *d = distance;
}