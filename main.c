#include <windows.h>
#include <gl/gl.h>
#include "stb_easy_font.h"
#include <string.h>
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

#define mapW 10
#define mapH 10

int width, height;
int game=0;
int menu_1=1;
int menu_2=0;
int menu_3=0;
int menu_5=0;
int random = 1;
int levelDifficulty=0;
int notPlace=0;

BOOL isHorizontally_manual = TRUE;

int x_ship_up=0;
int y_ship_up=0;
int x_ship_down=0;
int y_ship_down=0;

int x_remember_ship=-1;
int y_remember_ship=-1;

int left=1;
int right=1;
int up=1;
int down=1;
int wall=0;


int x_zigzag=-1;
int y_zigzag=-1;
int dy_zigzag=0;



int NewShipCnt=0;


typedef struct
{
    char name[20];
    float vert[8];
    BOOL hover;
}TButton;

typedef struct
{
    float vert[8];
    float vertShip[8];
    float vertShoot[8];
    BOOL ship;
    BOOL shoot;
    BOOL hit;
    BOOL FinishShip;
} TCell;

typedef struct
{
    int count;
    int size;
} Ship;

TCell map_1[mapW][mapH];
TCell map_2[mapW][mapH];
TCell map_3[6][6];

char alphabet[10];

Ship ship1 = {4, 1};
Ship ship2 = {3, 2};
Ship ship3 = {2, 3};
Ship ship4 = {1, 4};

Ship shipNew;


int cntKill_1=0, cntKill_2=0;

TButton btn [] = {
    {"Start", {100,300, 300,300, 300,360, 100,360}, FALSE},
    {"Quit", {100,410, 300,410, 300,470, 100,470}, FALSE}
};
int btnCnt = sizeof(btn)/sizeof(btn[0]);

TButton lvl [] = {
    {"Easy", {400,250, 600,250, 600,310, 400,310}, FALSE},
    {"Medium", {400,360, 600,360, 600,420, 400,420}, FALSE},
    {"Difficult", {400,470, 600,470, 600,530, 400,530}, FALSE}
};

int lvlCnt=0;
int randomChoiceCnt=0;

TButton randomChoice[] = {
    {"Random", {700,300, 900,300, 900,360, 700,360}, FALSE},
    {"Not random", {700,410, 900,410, 900,470, 700,470}, FALSE}
};


TButton menu5 [] = {
    {"New Game", {200,500, 400,500, 400,560, 200,560}, FALSE},
    {"Quit", {600,500, 800,500, 800,560, 600,560}, FALSE}
};
int menu5Cnt = 0;

BOOL PointInButton(int x, int y, TButton btn)
{
    return(x > btn.vert[0]) && (x < btn.vert[4]) &&
        (y > btn.vert[1]) && (y < btn.vert[5]);
}

void print_string(float x, float y, char *text, float r, float g, float b)
{
  static char buffer[99999]; // ~500 chars
  int num_quads;

  num_quads = stb_easy_font_print(x, y, text, NULL, buffer, sizeof(buffer));

  glColor3f(r,g,b);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 16, buffer);
  glDrawArrays(GL_QUADS, 0, num_quads*4);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void TButton_Show(TButton btn)
{
    glEnableClientState(GL_VERTEX_ARRAY);
        if (btn.hover) glColor3f(1,0,0);
        else glColor3f(1,1,0);
        glVertexPointer(2, GL_FLOAT, 0, btn.vert);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();
        glTranslatef(btn.vert[0], btn.vert[1], 0);
        glScalef(2,2,2);
        print_string(30,10, btn.name, 0,0,0);
    glPopMatrix();
}

BOOL isCellInMap_point(int x, int y)
{
    return(x > 100) && (x < 600) &&
        (y > 100) && (y < 600);
}

BOOL isCellInMap(int x, int y)
{
    return(x>=0) && (y>=0) && (x<mapW) && (y<mapH);
}

TCell* FindCellByCoordinates(int x, int y, TCell (*map)[mapH])
{
    for (int i = 0; i < mapW; i++) {
        for (int j = 0; j < mapH; j++) {
            TCell* cell = &map[i][j];
            if (x >= cell->vert[0] && x <= cell->vert[4] &&
                y >= cell->vert[1] && y <= cell->vert[5]) {
                return cell;
            }
        }
    }
    return NULL;
}

BOOL isNeighbourShip(TCell (*map)[mapH], int x, int y)
{
    BOOL isShip = FALSE;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (isCellInMap(x+dx, y+dy) && map[x+dx][y+dy].ship)
            {
                return TRUE;
            }
            if (isCellInMap(x+dx, y+dy) && map[x+dx][y+dy].hit)
            {
                map[x+dx][y+dy].hit=FALSE;
                isShip = isNeighbourShip(map,x+dx, y+dy);
                map[x+dx][y+dy].hit=TRUE;
            }
        }
    }
    return isShip;
}

void killedShip(TCell (*map)[mapH], int x, int y)
{
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if(isCellInMap(x+dx, y+dy) && map[x+dx][y+dy].hit)
            {
                map[x+dx][y+dy].hit=FALSE;
                killedShip(map,x+dx, y+dy);
            }
            if(isCellInMap(x+dx, y+dy))
            map[x+dx][y+dy].shoot = TRUE;
        }
    }
}

void ZigZag()
{
    if(x_zigzag==9)
    {
        y_zigzag +=2;
        x_zigzag = -1;
        dy_zigzag++;
    }
    if(dy_zigzag==5)
    {
        y_zigzag=0;
        x_zigzag = -1;
        dy_zigzag=0;
    }
    x_zigzag = x_zigzag + 1;
    if(y_zigzag%2==0 || y_zigzag==-1)
    {
        y_zigzag = y_zigzag + 1;
    }
    else
    {
        y_zigzag = y_zigzag - 1;
    }


}

void botShoot_1()
{
    int x = rand()%mapW;
    int y = rand()%mapH;
    if (!isCellInMap(x, y) || map_2[x][y].shoot)
    {
        botShoot_1();
    }
    else
    {
        map_2[x][y].shoot=TRUE;
        if(map_2[x][y].ship)
        {
            botShoot_1();
        }
    }

}

void botShoot_2_body(int x, int y)
{
    map_2[x][y].shoot=TRUE;
    if(map_2[x][y].FinishShip)
    {
        map_2[x][y].hit=TRUE;
        map_2[x][y].ship=FALSE;
        if(isCellInMap(x-1, y) && !map_2[x-1][y].shoot && up==1)
        {
            left=0;
            right=0;
            botShoot_2_body(x-1,y);
            if(x_remember_ship==-1 && y_remember_ship==-1)
            {
                up=0;
                left=1;
                right=1;
                x_remember_ship=x;
                y_remember_ship=y;
                return;
            }
            x_remember_ship=x;
            y_remember_ship=y;
            left=0;
            right=0;
            up=2;
            return;
        }
        if(isCellInMap(x+1, y) && !map_2[x+1][y].shoot && down==1)
        {
            left=0;
            right=0;
            botShoot_2_body(x+1,y);
            if(x_remember_ship==-1 && y_remember_ship==-1)
            {
                down=0;
                left=1;
                right=1;
                x_remember_ship=x;
                y_remember_ship=y;
                return;
            }
            x_remember_ship=x;
            y_remember_ship=y;
            left=0;
            right=0;
            down=2;
            return;
        }

        if(isCellInMap(x, y-1) && !map_2[x][y-1].shoot && left==1)
        {
            down=0;
            up=0;
            botShoot_2_body(x,y-1);
            if(x_remember_ship==-1 && y_remember_ship==-1)
            {
                left=0;
                x_remember_ship=x;
                y_remember_ship=y;
                return;
            }
            left=2;
            x_remember_ship=x;
            y_remember_ship=y;
            return;
        }

        if(isCellInMap(x, y+1) && !map_2[x][y+1].shoot && right==1)
        {
            down=0;
            up=0;
            botShoot_2_body(x,y+1);
            if(x_remember_ship==-1 && y_remember_ship==-1)
            {
                right=0;
                x_remember_ship=x;
                y_remember_ship=y;
                return;
            }
            right=2;
            x_remember_ship=x;
            y_remember_ship=y;
            return;
        }
        wall=1;
        return;
    }
    x_remember_ship=-1;
    y_remember_ship=-1;
    return;
}

void botShoot_2(int x, int y)
{
    botShoot_2_body(x,y);
    if(map_2[x][y].FinishShip)
    {
        if(!isNeighbourShip(map_2, x, y))
        {
            killedShip(map_2, x, y);
            cntKill_2++;
            if(cntKill_2>=10)
                return;
            do
            {
                x_remember_ship=rand()%mapW;
                y_remember_ship=rand()%mapH;
            }while(map_2[x_remember_ship][y_remember_ship].shoot);
            left=1;
            right=1;
            up=1;
            down=1;
            botShoot_2(x_remember_ship, y_remember_ship);
        }
        if(wall)
        {
            wall=0;
            if(cntKill_2>=10)
                return;
            botShoot_2(x_remember_ship, y_remember_ship);
        }
    }
}

void botShoot_3(int x, int y)
{
    botShoot_2_body(x,y);
    if(map_2[x][y].FinishShip)
    {
        if(!isNeighbourShip(map_2, x, y))
        {
            killedShip(map_2, x, y);
            cntKill_2++;
            if(cntKill_2>=10)
                return;
            do
            {
                ZigZag();
                x_remember_ship=x_zigzag;
                y_remember_ship=y_zigzag;
            }while(map_2[x_remember_ship][y_remember_ship].shoot);
            left=1;
            right=1;
            up=1;
            down=1;
            botShoot_3(x_remember_ship, y_remember_ship);
        }
        if(wall==1)
        {
            wall=0;
            if(cntKill_2>=10)
                return;
            botShoot_3(x_remember_ship, y_remember_ship);
        }
    }
}

void shoot(int x, int y)
{
    for(int j=0; j<mapH; j++)
    {
        for(int i=0; i<mapW; i++)
        {
            if ((x > map_1[i][j].vert[0]) && (x < map_1[i][j].vert[4]) &&
                (y > map_1[i][j].vert[1]) && (y < map_1[i][j].vert[5]))
            {
                if(map_1[i][j].shoot == FALSE)
                {
                    map_1[i][j].shoot = TRUE;
                    if(!map_1[i][j].ship)
                    {
                        if(levelDifficulty==1)
                            botShoot_1();
                        else if(levelDifficulty==2)
                        {
                            if(x_remember_ship==-1 && y_remember_ship==-1)
                            {
                                do
                                {
                                    x_remember_ship=rand()%mapW;
                                    y_remember_ship=rand()%mapH;
                                }while(map_2[x_remember_ship][y_remember_ship].shoot);
                                left=1;
                                right=1;
                                up=1;
                                down=1;
                            }
                            botShoot_2(x_remember_ship, y_remember_ship);
                        }

                        else if(levelDifficulty==3)
                        {
                            if(x_remember_ship==-1 && y_remember_ship==-1)
                            {
                                do
                                {
                                    ZigZag();
                                    x_remember_ship=x_zigzag;
                                    y_remember_ship=y_zigzag;
                                }while(map_2[x_remember_ship][y_remember_ship].shoot);
                                left=1;
                                right=1;
                                up=1;
                                down=1;
                            }
                            botShoot_3(x_remember_ship, y_remember_ship);
                        }
                    }
                    return;
                }

            }
        }
    }
}

BOOL isCellForShip(int x, int y, TCell (*map)[mapH])
{
    if (!isCellInMap(x, y) || map[x][y].ship || map[x][y].FinishShip)
    {
        return FALSE;
    }
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (isCellInMap(x+dx, y+dy) && map[x+dx][y+dy].FinishShip) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

BOOL randomShip_N(TCell (*map)[mapH], Ship *ship_n, int x, int y, BOOL isHorizontally, int SizeShip)
{
    if(SizeShip == 0) {
        for(int i=0; i<ship_n->size; i++) {
            if(isHorizontally) {
                map[x][y-i-1].FinishShip = TRUE;
            } else {
                map[x-i-1][y].FinishShip = TRUE;
            }
        }
        return TRUE;
    }

    if(isCellForShip(x, y, map) == FALSE) {
        return FALSE;
    }

    map[x][y].ship = TRUE;
    SizeShip--;

    BOOL result;
    if(isHorizontally) {
        result = randomShip_N(map, ship_n, x, y+1, TRUE, SizeShip);
    } else {
        result = randomShip_N(map, ship_n, x+1, y, FALSE, SizeShip);
    }

    if(result == FALSE) {
        for(int i=0; i<(ship_n->size-SizeShip); i++) {
            if(isHorizontally) {
                map[x][y-i].ship = FALSE;
            } else {
                map[x-i][y].ship = FALSE;
            }
        }
        return randomShip_N(map, ship_n, rand()%mapW, rand()%mapH, rand()%2, ship_n->size);
    }

    return TRUE;
}

void randomShip(TCell (*map)[mapH], Ship *ship_n)
{
    for(int i=0; i<ship_n->count; i++) {
        int x = rand()%mapW;
        int y = rand()%mapH;
        BOOL isHorizontally = rand()%2;
        BOOL result = randomShip_N(map, ship_n, x, y, isHorizontally, ship_n->size);
        if(result == FALSE) {
            i--;
        }
    }
}

void enterTCell(TCell (*map)[mapH], int x, int y)
{
    int dy=30;
    int dx=30;
    for(int j=0; j<mapH; j++)
    {
        for(int i=0; i<mapW; i++)
        {
            map[i][j].vert[0]=x;
            map[i][j].vert[1]=y;
            map[i][j].vert[2]=x+dx;
            map[i][j].vert[3]=y;
            map[i][j].vert[4]=x+dx;
            map[i][j].vert[5]=y+dy;
            map[i][j].vert[6]=x;
            map[i][j].vert[7]=y+dy;
            y=y+dy;
        }
        x=x+dx;
        y=100;
    }
}

void enterShip(TCell (*map)[mapH], int x, int y)
{
    int dy=18;
    int dx=18;
    for(int j=0; j<mapH; j++)
    {
        for(int i=0; i<mapW; i++)
        {
            map[i][j].vertShip[0]=x;
            map[i][j].vertShip[1]=y;
            map[i][j].vertShip[2]=x+dx;
            map[i][j].vertShip[3]=y;
            map[i][j].vertShip[4]=x+dx;
            map[i][j].vertShip[5]=y+dy;
            map[i][j].vertShip[6]=x;
            map[i][j].vertShip[7]=y+dy;
            y=y+30;
        }
        x=x+30;
        y=106;
    }
}

void enterShoot(TCell (*map)[mapH], int x, int y)
{
    int dy=18;
    int dx=18;
    for(int j=0; j<mapH; j++)
    {
        for(int i=0; i<mapW; i++)
        {
            map[i][j].vertShoot[0]=x;
            map[i][j].vertShoot[1]=y;
            map[i][j].vertShoot[2]=x+dx;
            map[i][j].vertShoot[3]=y+dy;
            map[i][j].vertShoot[4]=x+dx;
            map[i][j].vertShoot[5]=y;
            map[i][j].vertShoot[6]=x;
            map[i][j].vertShoot[7]=y+dy;
            y=y+30;
        }
        x=x+30;
        y=106;
    }
}

void enterTCell_Ship(int x, int y)
{
    int x_2 = x+6;
    int y_2 = y+6;
    int dy=30;
    int dx=30;
    int dy_2=18;
    int dx_2=18;

    for(int j=0; j<6; j++)
    {
        for(int i=0; i<6; i++)
        {
            map_3[i][j].vert[0]=x;
            map_3[i][j].vert[1]=y;
            map_3[i][j].vert[2]=x+dx;
            map_3[i][j].vert[3]=y;
            map_3[i][j].vert[4]=x+dx;
            map_3[i][j].vert[5]=y+dy;
            map_3[i][j].vert[6]=x;
            map_3[i][j].vert[7]=y+dy;
            y=y+dy;

            map_3[i][j].vertShip[0]=x_2;
            map_3[i][j].vertShip[1]=y_2;
            map_3[i][j].vertShip[2]=x_2+dx_2;
            map_3[i][j].vertShip[3]=y_2;
            map_3[i][j].vertShip[4]=x_2+dx_2;
            map_3[i][j].vertShip[5]=y_2+dy_2;
            map_3[i][j].vertShip[6]=x_2;
            map_3[i][j].vertShip[7]=y_2+dy_2;
            y_2=y_2+30;
        }
        x=x+dx;
        y=420;

        x_2=x_2+30;
        y_2=426;
    }
}

void from_point_to_xy(TCell (*map)[mapH], int *x, int *y)
{
    for(int j=0; j<mapH; j++)
    {
        for(int i=0; i<mapW; i++)
        {
            if ((*x > map[i][j].vert[0]) && (*x < map[i][j].vert[4]) &&
                (*y > map[i][j].vert[1]) && (*y < map[i][j].vert[5]))
            {
                *x=i;
                *y=j;
            }
        }
    }
}

BOOL manualShip_map_2(Ship *ship_n, int x, int y, int SizeShip)
{
    if(SizeShip == 0) {
        for(int i=0; i<ship_n->size; i++) {
            if(isHorizontally_manual) {
                map_2[x][y-i-1].FinishShip = TRUE;
            } else {
                map_2[x+i+1][y].FinishShip = TRUE;
            }
        }
        return TRUE;
    }

    if(isCellForShip(x, y, map_2) == FALSE) {
        return FALSE;
    }

    map_2[x][y].ship = TRUE;
    SizeShip--;

    BOOL result;
    if(isHorizontally_manual) {
        result = manualShip_map_2( ship_n, x, y+1, SizeShip);
    } else {
        result = manualShip_map_2( ship_n, x-1, y, SizeShip);
    }

    if(result == FALSE) {
        for(int i=0; i<(ship_n->size-SizeShip); i++) {
            if(isHorizontally_manual) {
                map_2[x][y-i].ship = FALSE;
            } else {
                map_2[x+i][y].ship = FALSE;
            }
        }
        return FALSE;
    }
    return TRUE;
}

BOOL manualShip_map_3(TCell (*map)[6], Ship *ship_n, int x, int y, int SizeShip)
{
    if(SizeShip == 0) {
        for(int i=0; i<ship_n->size; i++) {
            if(isHorizontally_manual) {
                map[x][y-i-1].FinishShip = TRUE;
            } else {
                map[x+i+1][y].FinishShip = TRUE;
            }
        }
        return TRUE;
    }
    map[x][y].ship = TRUE;
    SizeShip--;

    BOOL result;
    if(isHorizontally_manual) {
        result = manualShip_map_3(map, ship_n, x, y+1, SizeShip);
    } else {
        result = manualShip_map_3(map, ship_n, x-1, y, SizeShip);
    }
    return TRUE;
}

void manualShip(TCell (*map)[6], Ship *ship_n)
{
    manualShip_map_3(map, ship_n, 3, 1, ship_n->size);
}

void Game_New()
{
    srand(time(NULL));

    memset(map_1, 0, sizeof(map_1));
    enterTCell(map_1, 100, 100);
    enterShip(map_1, 106, 106);
    enterShoot(map_1, 106, 106);
    randomShip(map_1, &ship1);
    randomShip(map_1, &ship2);
    randomShip(map_1, &ship3);
    randomShip(map_1, &ship4);


    memset(map_2, 0, sizeof(map_2));
    enterTCell(map_2, 600, 100);
    enterShip(map_2, 606, 106);
    enterShoot(map_2, 606, 106);
    if(random)
    {
        randomShip(map_2, &ship1);
        randomShip(map_2, &ship2);
        randomShip(map_2, &ship3);
        randomShip(map_2, &ship4);
    }
}


void ShowShoot(TCell map)
{
    glEnableClientState(GL_VERTEX_ARRAY);
        glColor3f(1,0,0);
        glVertexPointer(2, GL_FLOAT, 0, map.vertShoot);
        glLineWidth(4);
        glDrawArrays(GL_LINES, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ShowShip(TCell map)
{
    glEnableClientState(GL_VERTEX_ARRAY);
        glColor3f(0,0,0);
        glVertexPointer(2, GL_FLOAT, 0, map.vertShip);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ShowField(TCell map)
{
    glEnableClientState(GL_VERTEX_ARRAY);
        glColor3f(0.5,0.5,0.5);
        glVertexPointer(2, GL_FLOAT, 0, map.vert);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);
        glColor3f(1,1,1);
        glVertexPointer(2, GL_FLOAT, 0, map.vert);
        glLineWidth(1);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void Num_Show(float y, float dy, float x)
{
    char number = '1';
    y=y/3;
    dy=dy/3;
    x=x/3;
    for (int i = 0; i < 9; i++) {
        alphabet[0] = number;
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(3,3,3);
            print_string(x,y, alphabet, 1,1,1);
        glPopMatrix();
        number++;
        y+=dy;
    }
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        glScalef(3,3,3);
        print_string(x-20/3,y, "10", 1,1,1);
    glPopMatrix();

}

void Letter_Show(float x, float dx)
{
    char letter = 'a';
    x=x/3;
    dx=dx/3;
    for (int i = 0; i < 10; i++) {
        alphabet[0] = letter;
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(3,3,3);
            print_string(x,22, alphabet, 1,1,1);
        glPopMatrix();
        letter++;
        x+=dx;
    }
}

void Ne_Random_Game()
{
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        glScalef(3,3,3);
        print_string(615/3,650/3, "PLACE THE SHIPS", 1,1,1);
    glPopMatrix();

    memset(map_3, 0, sizeof(map_3));
    enterTCell_Ship(660, 420);
    if(NewShipCnt<1)
        shipNew=ship4;
    if(NewShipCnt<3 && NewShipCnt>=1)
        shipNew=ship3;
    if(NewShipCnt<6 && NewShipCnt>=3)
        shipNew=ship2;
    if(NewShipCnt<10 && NewShipCnt>=6)
        shipNew=ship1;
    manualShip(map_3, &shipNew);
    if(NewShipCnt==10)
    {
        random=1;
    }
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        for(int j=0; j<6; j++)
        {
            for(int i=0; i<6; i++)
            {
                ShowField(map_3[i][j]);
                if (map_3[i][j].ship)
                {
                    ShowShip(map_3[i][j]);
                }
            }
        }
    glPopMatrix();
}

void Game_Show()
{
    if(NewShipCnt>=10 && !menu_5)
    {
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(3,3,3);
            print_string(420/3,50/3, "YOUR TURN", 1,1,1);
        glPopMatrix();
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(2,2,2);
            print_string(180/2,440/2, "ENEMY FIELD", 0.5,0.5,0.5);
        glPopMatrix();
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(2,2,2);
            print_string(690/2,440/2, "YOUR FIELD", 0.5,0.5,0.5);
        glPopMatrix();

    }


    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        Letter_Show(110, 30);
        Num_Show(105, 30, 80);
        for(int j=0; j<mapH; j++)
        {
            for(int i=0; i<mapW; i++)
            {
                ShowField(map_1[i][j]);
                if (map_1[i][j].shoot)
                {
                    if(map_1[i][j].ship)
                    {
                        map_1[i][j].hit=TRUE;
                        map_1[i][j].ship=FALSE;
                        if(!isNeighbourShip(map_1, i, j))
                        {
                            killedShip(map_1, i, j);
                            cntKill_1++;
                        }
                    }
                    if (map_1[i][j].FinishShip)
                    {
                        ShowShip(map_1[i][j]);
                        ShowShoot(map_1[i][j]);
                    }
                    else
                    ShowShoot(map_1[i][j]);
                }
            }
        }
    glPopMatrix();

    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        Letter_Show(610, 30);
        Num_Show(105, 30, 580);
        for(int j=0; j<mapH; j++)
        {
            for(int i=0; i<mapW; i++)
            {
                ShowField(map_2[i][j]);
                if (map_2[i][j].ship)
                {
                    ShowShip(map_2[i][j]);
                }
                if (map_2[i][j].shoot)
                {
                    if(map_2[i][j].ship)
                    {
                        map_2[i][j].hit=TRUE;
                        map_2[i][j].ship=FALSE;
                        if(!isNeighbourShip(map_2, i, j))
                        {
                            killedShip(map_2, i, j);
                            cntKill_2++;
                        }
                    }
                    if (map_2[i][j].FinishShip)
                    {
                        ShowShip(map_2[i][j]);
                        ShowShoot(map_2[i][j]);
                    }
                    else
                    ShowShoot(map_2[i][j]);
                }
            }
        }
    glPopMatrix();
}


void ShowNotPlace()
{
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        glScalef(2,2,2);
        print_string(615/2,700/2, "Wrong place. Try again ...", 1,0,0);
    glPopMatrix();
}

void ShowMenu()
{
    if(menu_1)
    {
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(3,3,3);
            print_string(80/3,250/3, "Select an action", 1,1,1);
        glPopMatrix();
    }
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        for(int i = 0; i< btnCnt; i++)
            TButton_Show(btn[i]);
    glPopMatrix();
}

void ShowMenu_2()
{
    if(menu_2)
    {
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(3,3,3);
            print_string(377/3,200/3, "Difficulty level", 1,1,1);
        glPopMatrix();
    }

    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        for(int i = 0; i< lvlCnt; i++)
            TButton_Show(lvl[i]);
    glPopMatrix();
}

void ShowMenu_3()
{
    if(menu_3)
    {
        glPushMatrix();
            glLoadIdentity();
            glOrtho(0,width, height, 0, -1, 1);
            glScalef(3,3,3);
            print_string(675/3,250/3, "Location of ships", 1,1,1);
        glPopMatrix();
    }
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        for(int i = 0; i< randomChoiceCnt; i++)
            TButton_Show(randomChoice[i]);
    glPopMatrix();
}

void ShowMenu_5()
{
    if(menu_5)
    {
        if(cntKill_1==10)
        {
            glPushMatrix();
                glLoadIdentity();
                glOrtho(0,width, height, 0, -1, 1);
                glScalef(3,3,3);
                print_string(430/3,50/3, "YOU WIN", 0,1,0);
            glPopMatrix();
        }
        if(cntKill_2==10)
        {
            glPushMatrix();
                glLoadIdentity();
                glOrtho(0,width, height, 0, -1, 1);
                glScalef(3,3,3);
                print_string(420/3,50/3, "YOU LOSE", 1,0,0);
            glPopMatrix();
        }

    }
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width, height, 0, -1, 1);
        for(int i = 0; i< menu5Cnt; i++)
            TButton_Show(menu5[i]);
    glPopMatrix();
}



int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1000,
                          800,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);


    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);


            if(cntKill_1==10 || cntKill_2==10)
            {
                menu5Cnt = sizeof(menu5)/sizeof(menu5[0]);
                menu_5=1;
            }

            if(notPlace)
            {
                ShowNotPlace();
            }


            ShowMenu();

            ShowMenu_2();

            ShowMenu_3();

            ShowMenu_5();

            if(!random && NewShipCnt<10)
            {
                Ne_Random_Game();
            }

            if(game)
            Game_Show();


            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_LBUTTONDOWN:
        {
            x_ship_down = LOWORD(lParam);
            y_ship_down = HIWORD(lParam);
            for(int i = 0; i< btnCnt; i++)
                if(PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i]))
                {
                    if(strcmp(btn[i].name, "Quit")==0)
                    {
                        PostQuitMessage(0);
                    }
                    if(strcmp(btn[i].name, "Start")==0)
                    {
                        btnCnt=0;
                        lvlCnt = sizeof(lvl)/sizeof(lvl[0]);
                        menu_1=0;
                        menu_2=1;
                    }
                }
            for(int i = 0; i< lvlCnt; i++)
                if(PointInButton(LOWORD(lParam), HIWORD(lParam), lvl[i]))
                {
                    if(strcmp(lvl[i].name, "Easy")==0)
                    {
                        lvlCnt=0;
                        randomChoiceCnt = sizeof(randomChoice)/sizeof(randomChoice[0]);
                        menu_2=0;
                        menu_3=1;
                        levelDifficulty=1;
                    }
                    if(strcmp(lvl[i].name, "Medium")==0)
                    {
                        lvlCnt=0;
                        randomChoiceCnt = sizeof(randomChoice)/sizeof(randomChoice[0]);
                        menu_2=0;
                        menu_3=1;
                        levelDifficulty=2;
                    }
                    if(strcmp(lvl[i].name, "Difficult")==0)
                    {
                        lvlCnt=0;
                        randomChoiceCnt = sizeof(randomChoice)/sizeof(randomChoice[0]);
                        menu_2=0;
                        menu_3=1;
                        levelDifficulty=3;
                    }
                }

            for(int i = 0; i< randomChoiceCnt; i++)
                if(PointInButton(LOWORD(lParam), HIWORD(lParam), randomChoice[i]))
                {
                    if(strcmp(randomChoice[i].name, "Random")==0)
                    {
                        menu_3=0;
                        random=1;
                        game=1;
                        randomChoiceCnt=0;
                        NewShipCnt=10;
                        Game_New();
                    }
                    if(strcmp(randomChoice[i].name, "Not random")==0)
                    {
                        menu_3=0;
                        random=0;
                        game=1;
                        randomChoiceCnt=0;
                        NewShipCnt=0;
                        Game_New();
                    }
                }
            for(int i = 0; i< menu5Cnt; i++)
                if(PointInButton(LOWORD(lParam), HIWORD(lParam), menu5[i]))
                {
                    if(strcmp(menu5[i].name, "New Game")==0)
                    {
                        game=0;
                        lvlCnt = sizeof(lvl)/sizeof(lvl[0]);
                        menu_5=0;
                        menu_2=1;
                        menu5Cnt=0;
                        cntKill_1=0;
                        cntKill_2=0;
                        x_remember_ship=-1;
                        y_remember_ship=-1;
                        left=1;
                        right=1;
                        up=1;
                        down=1;
                        wall=0;
                        x_zigzag=-1;
                        y_zigzag=-1;
                        dy_zigzag=0;
                    }
                    if(strcmp(menu5[i].name, "Quit")==0)
                    {
                        PostQuitMessage(0);
                    }
                }

            if(isCellInMap_point(LOWORD(lParam), HIWORD(lParam)) && (cntKill_1!=10) && (cntKill_2!=10) && NewShipCnt>=10)
            {
                shoot(LOWORD(lParam),HIWORD(lParam));
            }

        }
        break;

        case WM_MOUSEMOVE:
            for(int i = 0; i< btnCnt; i++)
                btn[i].hover=PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i]);

            for(int i = 0; i< lvlCnt; i++)
                lvl[i].hover=PointInButton(LOWORD(lParam), HIWORD(lParam), lvl[i]);

            for(int i = 0; i< randomChoiceCnt; i++)
                randomChoice[i].hover=PointInButton(LOWORD(lParam), HIWORD(lParam), randomChoice[i]);
            for(int i = 0; i< menu5Cnt; i++)
                menu5[i].hover=PointInButton(LOWORD(lParam), HIWORD(lParam), menu5[i]);
        break;



        case WM_LBUTTONUP:
            x_ship_up = LOWORD(lParam);
            y_ship_up = HIWORD(lParam);
            if(FindCellByCoordinates(x_ship_down, y_ship_down, map_3) &&
               FindCellByCoordinates(x_ship_up, y_ship_up, map_2))
            {
                int i=x_ship_up;
                int j=y_ship_up;
                from_point_to_xy(map_2, &i, &j);
                if(!manualShip_map_2(&shipNew, i, j, shipNew.size))
                {
                    notPlace=1;
                    break;
                }
                NewShipCnt++;
                notPlace=0;

            }
        break;

        case WM_RBUTTONDOWN:
            if((LOWORD(lParam) > 660) && (LOWORD(lParam) < 840) &&
                (HIWORD(lParam) > 420) && (HIWORD(lParam) < 600))
            {
                isHorizontally_manual=!isHorizontally_manual;
            }
        break;

        case WM_SIZE:
            width = LOWORD(lParam);
            height = HIWORD(lParam);
            glViewport(0,0, width, height);
            glLoadIdentity();
            float k = width / (float)height;
            glOrtho(-k, k , -1,1, -1,1 );
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
