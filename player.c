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


//Enumeration pour les directon de la matrice pere
enum direction {ORIGINE, DROITE, GAUCHE, BAS, HAUT};
typedef enum direction direction; // define action as a shorthand for enum actions


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

//Fonctions booléennes
bool isPos(int x, int y, int mapxsize, int mapysize);
bool isAccess(int x, int y, char ** map);
bool isBombable(Case c);
bool secu(int x, int y, char ** map, int mapx, int mapy);


//Fonctions de déplacement
action reculer(Position, Position, char **, int, int);
action aller(Position, Position);
action actionXY(Position);

//Pour les matrices
int ** matrice(int l,int c);
void freeMatrice(int ** M, int l);
void initMat(int ** M, int tailleX, int tailleY, int val);
void affMat(int ** m, int l, int c);

//Module de détermination de la case la plus proche
void Init_tabs(int ** * d, int ** * p, int x, int y, char ** map, int mapxsize, int mapysize);
Case plusProche(int ** distance, int ** pere, char * * map, int mapxsize, int mapysize, int x, int y, char type);
void posPP(int ** distance, char ** map, int tailleX, int tailleY, char type, Position * pp, bool * trouve);
Position posN(Position pp, int ** pere, int ** distance, int mapxsize, int mapysize);

//Procédure d'affichage pour le débuggage
void printAction(action);
void affMap(char ** m, int l, int c);




/*
  bomberman function:
  Renvoie l'action la plus efficiente (voir compte rendu (CR))
  PS Dans les arguments on met d'abord Y et ensuite X pour suivre notre convention (cf CR)
 */
action bomberman(
		 char * * map, // array of char modeling the game map
		 int mapysize, // x size of the map
		 int mapxsize, // y size of the map
		 int y, // x position of BOMBERMAN (selon l'affichage, pas selon notre convetion)
		 int x, // y position of BOMBERMAN 
		 action last_action, // last action made, -1 in the be beginning 
		 int remaining_bombs, // number of bombs
		 int explosion_range // explosion range for the bombs 
		 ) {

  //Déclarations des variables
  action a; // action to choose and return
  if(DEBUG) printf("Debut tour\n");

  //Parametres modifiable 
  int distMonstre = 1; //Distance a partir de laquelle on s'enfuit en voyant un montsre
  int margeSecuBomb = 1; //Distance en plus du range de la bombe pour la sécurité

  //Création des matrices distance et pere remplis par la fonction Init_tabs
  int** distance = NULL;
  int** pere = NULL;
  Init_tabs(&distance, &pere, x,y,map,mapxsize,mapysize);


  //Déclaration (et calculs) des cases les plus proches.
  Case exit = plusProche(distance, pere, map, mapxsize, mapysize, x, y, EXIT);
  Case break_wall = plusProche(distance, pere, map, mapxsize, mapysize, x, y, BREAKABLE_WALL);
  Case bomb = plusProche(distance, pere, map, mapxsize, mapysize, x, y, BOMB);
  Case ghost = plusProche(distance, pere, map, mapxsize, mapysize, x, y, GHOST_ENEMY);
  Case flamme = plusProche(distance, pere, map, mapxsize, mapysize, x, y, FLAME_ENEMY);
  Case caseAFuire;  //Pour le premier cas(cf ci dessous)

  //Déclaration de la Position qui correspond 
  Position me;
  me.x = x;
  me.y = y;



  //Début du programme et de l'algo
  if(DEBUG){printf("Initialisation ok\n");}

  //1er cas : 
  //Si l'on vient de poser une bombe ou qu'il y en a une proche, on s'éloigne
  if (last_action == BOMBING || (bomb.dist !=-1 && bomb.dist <= explosion_range+margeSecuBomb) ){
    if(DEBUG){
      printf("%d\n", last_action);
      printf("bomb dist : %d\n", bomb.dist);
      printf("CAS 1\n");
    }
    if(last_action == BOMBING){
      caseAFuire = break_wall;
    }else{
      caseAFuire = bomb;
    }
    a = reculer(caseAFuire.posNext, me, map, mapxsize, mapysize);
  }

  //2e cas :
  //Si un monstre est à distance distMonstre ou moins de nous, on fuit.
  else if(ghost.existe && ghost.dist <= distMonstre)
  {
    reculer(ghost.posNext, me, map, mapxsize, mapysize);
  }
  else if(flamme.existe && flamme.dist <= distMonstre)
  {
    reculer(flamme.posNext, me, map, mapxsize, mapysize);
  }

  //3e cas :
  //Si la sortie est visible, on y va
  else if (exit.dist != -1)
  {
    if(DEBUG) printf("CAS 2 exit\n");
    a = aller(exit.posNext,me);
  }

  //4e cas :
  //Si on peut poser une bombe, on bombe
  //condition de "BOMBING" : atteindre le mur cassable le plus proche et avoir des Bombe
  else if (isBombable(break_wall) && remaining_bombs > 0)
  {
    if(DEBUG) printf("CAS 3 bombing\n");
    a = BOMBING;
  }

  //5e cas :
  //Dans le cas contraire, on se rapproche d'un mur cassable, pour le bombarder.
  else if(break_wall.dist>1)
  {
    if(DEBUG){
      printf("CAS 5 move \n");
      printf("posNext %d,%d\n", break_wall.posNext.x,break_wall.posNext.y);
    }
    
    a = aller(break_wall.posNext,me);
  }

  //6e cas :
  //On ne peut pas bomber et on est a une distance <= 1 d'un mur, c'est que l'on a plus
  //de bombes, et que l'on doit attendre, on s'éloigne alors du mur, pour s'en rapprocher
  // au prochain et donc simuler une attente.
  else
  {
    if(DEBUG) printf("Cas 6, gange temps et recule\n");
    a = reculer(break_wall.posNext, me, map, mapxsize, mapysize);
  }


//On libère les matrices distance et pere alloués dynamiquement au début du tour.
freeMatrice(distance,mapxsize);
freeMatrice(pere,mapxsize);

//On renvoie l'action du tour
if(DEBUG){printAction(a);}
return a;
}


//****************************************************************************
//**************************Modules complémentaires***************************
//****************************************************************************

//##########################
//###Fonctions booléennes###
//##########################

//Fonction qui renvoie vrai si la position (x,y) est bien dans la map
bool isPos(int x, int y, int mapxsize, int mapysize){
  return (x>=0 && x<mapxsize) &&  (y>=0 && y<mapysize);
}

//Fonction qui renvoie vrai ssi la position (x,y) peut etre parcouru par bomberman
//ie si c'est un chemin ou un bonus
bool isAccess(int x, int y, char ** map){
  bool ok = false;
  char val = map[x][y];
  if(val == PATH) ok = true;
  if(val == FLAME_BONUS) ok = true;
  if(val == BOMB_BONUS) ok = true;

  return ok;
}

//Fonction qui renvoie true si la case peut etre bomber
//On fait simple, on peut seulement si on est a coté (ie la distance est d'un)
bool isBombable(Case c){
  return c.dist == 1;
}


//Fonction qui renvoie si la position (x,y) est sécurisée
//On considère que la case est sécurisé si l'on peut fuire sans faire demi tour
//ie s'il y a au moins 1 case accesible (la notre ne l'est pas puisque @ n'est pas considéré accesible)
bool secu(int x, int y, char ** map, int mapxsize, int mapysize){
  bool safe=false;
  if (isPos(x+1,y, mapxsize, mapysize) && isAccess(x+1,y,map)) safe=true;
  if (isPos(x-1,y, mapxsize, mapysize) && isAccess(x-1,y,map)) safe=true;
  if (isPos(x,y+1, mapxsize, mapysize) && isAccess(x,y+1,map)) safe=true;
  if (isPos(x,y-1, mapxsize, mapysize) && isAccess(x,y-1,map)) safe=true;
    
  return safe;
}




//##############################
//###Fonctions de déplacement###
//##############################


//Fonction reculer
//renvoie une action qui permet de s'éloigner de la position posNext à partir de la position me
//on choisit aléatoirement et on verifie que l'action choisie nous mène a une position valide,
//Accessible qui n'est pas posNext (on veut s'en éloigner) et qui est sécurisé pour ne pas
//s'enfermer dans une impasse après avoir poser une bombe par exemple
action reculer(Position posNext, Position me, char ** map, int mapxsize, int mapysize){

  int x,y;
  action a;
  bool ok = false;
  
  do {
    a=rand()%4+1; //Action choisi aléatoirement (le +1 permet de choisir entre 1 et 5 pour enlever le cas 0 : BOMBING)

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


//Fonction basique qui renvoie l'action à faire pour aller à une position voisine
//basé sur la fonction actionXY
action aller(Position p, Position me){
  Position pos;
  pos.x = p.x-me.x;
  pos.y = p.y-me.y;
  return actionXY(pos);
}


//Fonction qui à partir d'une position voisine à 0 renvoie l'action à faire pour y
//aller (à partir de 0)
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
    a=BOMBING; //Pour perdre et ne pas faire planter le jeu on renvoie une action quand même
  }
  return a;    
}



//#######################################
//#####Modules relatifs aux Matrices#####
//#######################################

//Fonction de création d'une matrice de taille variable (l,c)
//On renvoie un int ** crée en construisant un tableau dynamique
//de type int * et en remplissant chaque élément(correspondant aux ligne)
//par un int * (les collones)
int ** matrice(int l,int c){
  int ** M = NULL;
  M = malloc (sizeof(int*)*l); //Allocation du tableau de tableau de taille l
  if(M==NULL){
    printf("erreur Allocation Dynamique de la matrice");
    exit(0);
  }
  int i;
  for (i=0; i<l; i++){              //Pour chaque ligne
    M[i] = malloc (sizeof(int)*c);  //On remplit paar le tableau corresspondant à la collone
    if(M[i]==NULL)
    {
      printf("erreur Allocation Dynamique de la matrice à ligne %d",i);
      exit(0);
    }
  }
  return M;
}

//Procédure qui libère la mémoire en fin d'utilisation
//Libèreligne par ligne, puis le tableau de ligne M
void freeMatrice(int ** M, int l){
  int i;
  for (i=0; i<l; i++){
    free(M[i]);
  }
  free(M);
}

//Procédure qui initialise une matrice M à la valeur val
void initMat(int ** M, int tLi, int tCol, int val){
  int i,j;
  for(i=0;i<tLi;i++){
    for(j=0;j<tCol;j++){
      M[i][j]=val;
    }
  }
}

//Procédure qui affiche la matrice (pour les debugs)
void affMat(int ** m, int l, int c){
  int i,j;
  for(i=0;i<l;i++){
    for(j=0;j<c;j++){
      if(m[i][j]<0){
        printf("* "); //Pour une meilleur lisibilité des cases non visitées
      }else{
        printf("%d ",m[i][j]);
      }
    }
    printf("\n");
  }
}




//####################################################################################
//###Modules relatifs à la recherche de la case d'un type spécifié le plus proche###
//####################################################################################

//Procédure Init_tabs
//qui modifie et remplis coreectement les matrices d et p cooresspondant a distance et pere.
//
//-Tout d'abord on crée 2 matrices initialisés à -1 et -2 (cf CR)
//-Puis on déclare une sous fonction récursive pars(x,y) qui met a jours les voisins de la
//position (x,y) et se reappelle récursivement sur ce voisin s'il est accessible et inexploré ou
// déja visité mais avec un chemin plus long
//-On lance un appelle de cette fonction sur la positiion passé en argument de Init_tabs, qui 
//remplit correctement les matrices distance et pere
//-Enfin, on modifie la valeur de d et p pour qu'il pointent sur les matrices distances et pere
//correctement remplis.
void Init_tabs(int ** * d, int ** * p, int x, int y, char ** map, int mapxsize, int mapysize){

  int** distance = NULL;
  distance = matrice(mapxsize, mapysize);
  initMat(distance, mapxsize, mapysize, -1);
  distance[x][y] = 0;

  int** pere = NULL;
  pere = matrice(mapxsize, mapysize);
  initMat(pere, mapxsize, mapysize, -2);


  //On va définir une procédure récursive qui va mettre ses voisins inexlorés à une distance +1
  void pars(int x, int y){
    //pour chaque voisin
    int i,j;
    //Case gauche
    i=-1;
    j=0;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = DROITE; //Et on dit qu'il vient de droite (1 pour la droite par notre convention)
      if (isAccess(x+i,y+j,map))
      {
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }

    //Case droite
    i=1;
    j=0;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = GAUCHE; //Et on dit qu'il vient de gauche (2 pour la gauche par notre convention)
      if(isAccess(x+i,y+j,map)){
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }

    //Case haut
    i=0;
    j=1;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = BAS;
      if(isAccess(x+i,y+j,map)){
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }


    //Case bas
    i=0;
    j=-1;
    if(isPos(x+i,y+j,mapxsize,mapysize) && (distance[x+i][y+j] == -1 || distance[x+i][y+j] > distance[x][y]+1)){ //Si il est accessible et inexploré ou déja été exploré mais de manière moins optimale
      distance[x+i][y+j] = distance[x][y] + 1; //Alors on actualise sa distance
      pere[x+i][y+j] = HAUT;
      if(isAccess(x+i,y+j,map)){
        pars(x+i,y+j); //Et on continue le chemin récursivement si c'est un chemin
      }
    }
  }

  //On lance l'appelle initial de notre position
  pars(x,y);

  *p = pere;
  *d = distance;
}

//Fonction principale de cette partie
//renvoie la case de type paramétré la plus proche de la position (x,y) a partir des
//modules auxiliaire qu'elle appelle
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

  Case pProche; //Case a renvoyer

  Position pp;//Position plus proche
  pp.x=0;
  pp.y=0;

  bool trouve = false; //Bool qui vrai ssi la case sera trouvé (visible sur la map)
  Position next; //Position suivante pour aller vers la case

  posPP(distance, map, mapxsize, mapysize, type, &pp, &trouve); //Position du plus proche et de s'il existe
  next = posN(pp, pere, distance, mapxsize, mapysize); //Position suivante retrouvé grace au tableau pere

  //MAJ des champs de Case pProche avant de la renvoyer
  pProche.existe = trouve;
  pProche.val = map[pp.x][pp.y];
  pProche.dist = distance[pp.x][pp.y];
  pProche.pos = pp;
  pProche.posNext = next;
  
  return pProche;
}

//Procédure posPP
//qui modifie la position pp et le booléen trouvé passé par addresse en argument.
//Principe : cherche en parcourant toute la map la position la plus proche (avec la distance minimale)
//correspondant à une case de type type et qui est accessible (dist != -1) et pas la notre (dist !=0)
void posPP(int ** distance, char ** map, int tailleX, int tailleY, char type, Position * pp, bool * trouve){
  int distMin = 1000;
  
  int i,j;
  for (i = 0; i < tailleX; i++) {
    for (j = 0; j < tailleY; j++) {
      if(map[i][j]==type && distance[i][j]<distMin && distance[i][j]!=-1 && distance[i][j]!=0){
        distMin = distance[i][j];//Maj du min
        //MAJ de la position correspondant à la distance min
        (*pp).x = i;
        (*pp).y = j;
        *trouve = true;//Si une telle case existe, existe devient true
      }
    }
  }
}


//Fonction posN
//renvoyant la position où il faut aller pour aller vers la case pp trouvé en remontant le tableau des peres.
Position posN(Position pp, int ** pere, int ** distance, int mapxsize, int mapysize){
  int dist = distance[pp.x][pp.y];
  Position pos = pp;
  while(dist>1){
    switch(pere[pos.x][pos.y]) {
      //Viens de droite
      case DROITE:
        pos.x = pos.x+1; //On décale notre position à droite
        break;
      //Viens de gauche
      case GAUCHE:
        pos.x = pos.x-1; //On décale notre position à gauche
        break;
      //Viens du bas
      case BAS:
        pos.y = pos.y-1; //On décale notre position en bas
        break;
      //Viens du haut
      case HAUT:
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






//####################################
//###Procédures d'affichage (DEBUG)###
//####################################

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

//Procédure d'affichage de la map (pour les tests)
void affMap(char ** m, int l, int c){
  int i,j;
  for(i=0;i<l;i++){
    for(j=0;j<c;j++){
      printf(" %c ",m[i][j]);
    }
    printf("\n");
  }
}