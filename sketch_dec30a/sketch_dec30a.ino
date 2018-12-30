#include "LedControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



#define buttonDelay 400
#define ROWS 16
#define COLS 8
#define TRUE 1
#define FALSE 0
#define DOWN  1
#define LEFT  2
#define RIGHT 3
#define ROTATE  4

LedControl lc1=LedControl(12,10,11,2); 
const int rotateButton = 7;
const int downButton = 6;
const int leftButton = 5;
const int rightButton = 4;
unsigned long firstTime, lastTime;
int rotateButtonPressed = false;
int leftButtonPressed = false;
int rightButtonPressed = false;
char Table[ROWS][COLS] = {0};

typedef struct {
    char **array;
    int width, row, col;
} Shape;

Shape current;



void setup() {
  lc1.shutdown(0,false);
  lc1.shutdown(1,false);
  srand(time(0));
  pinMode(rotateButton, INPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);
  pinMode(downButton, INPUT);
  firstTime = millis();
  GetNewShape();
}

void loop() {
  lastTime = millis();
  
  if ( digitalRead(rotateButton)) {
    rotateButtonPressed = true;
  } else if (!digitalRead(rotateButton) && rotateButtonPressed){
    rotateButtonPressed = false;
    InputProccesor(ROTATE);
  }
  
  if ( digitalRead(leftButton)) {
    leftButtonPressed = true;
  } else if (!digitalRead(leftButton) && leftButtonPressed) {
    leftButtonPressed = false;
    InputProccesor(LEFT);
  }
  
  if ( digitalRead(rightButton)) {
    rightButtonPressed = true;
  } else if (!digitalRead(rightButton) && rightButtonPressed) {
    rightButtonPressed = false;
    InputProccesor(RIGHT);
  }
  
  if ( digitalRead(downButton)) {
    InputProccesor(DOWN);
  }
  
  if (lastTime - firstTime >= buttonDelay) {
    InputProccesor(DOWN);
    firstTime = millis();
  }
}




const Shape ShapesArray[7]= {
        {(char *[]){(char []){0,1,1},(char []){1,1,0}, (char []){0,0,0}}, 3},                           //S_tetrisBlock
        {(char *[]){(char []){1,1,0},(char []){0,1,1}, (char []){0,0,0}}, 3},                           //Z_tetrisBlock
        {(char *[]){(char []){0,1,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //T_tetrisBlock
        {(char *[]){(char []){0,0,1},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //L_tetrisBlock
        {(char *[]){(char []){1,0,0},(char []){1,1,1}, (char []){0,0,0}}, 3},                           //ML_tetrisBlock
        {(char *[]){(char []){1,1},(char []){1,1}}, 2},                                                   //SQ_tetrisBlock
        {(char *[]){(char []){0,0,0,0}, (char []){1,1,1,1}, (char []){0,0,0,0}, (char []){0,0,0,0}}, 4} //R_tetrisBlock
};

Shape CopyShape(Shape shape){
    Shape new_shape = shape;
    char **copyShape = shape.array;
    new_shape.array = (char**)malloc(new_shape.width*sizeof(char*));
    int i, j;
    for(i = 0; i < new_shape.width; i++){
        new_shape.array[i] = (char*)malloc(new_shape.width*sizeof(char));
        for(j=0; j < new_shape.width; j++) {
            new_shape.array[i][j] = copyShape[i][j];
        }
    }
    return new_shape;
}

void DeleteShape(Shape shape){
    int i;
    for(i = 0; i < shape.width; i++){
        free(shape.array[i]);
    }
    free(shape.array);
}

int CheckPosition(Shape shape){
    char **array = shape.array;
    int i, j;
    for(i = 0; i < shape.width;i++) {
        for(j = 0; j < shape.width ;j++){
            if((shape.col+j < 0 || shape.col+j >= COLS || shape.row+i >= ROWS)){ //Out of borders
                if(array[i][j])
                    return FALSE;
            }
            else if(Table[shape.row+i][shape.col+j] && array[i][j])
                return FALSE;
        }
    }
    return TRUE;
}

void GetNewShape(){
    Shape new_shape = CopyShape(ShapesArray[rand()%7]);
    new_shape.col = rand()%(COLS-new_shape.width+1);
    new_shape.row = 0;
    DeleteShape(current);
    current = new_shape;
    if(!CheckPosition(current)){
      for (int k = 0 ; k < 4 ; k++) {
        for (int i = 0 ; i < ROWS ; i++) {
          for (int j = 0 ; j < COLS ; j++) {
            lc1.setLed(i / 8 , i % 8, j, true);
          }
        }
        delay(300);
        for (int i = 0 ; i < ROWS ; i++) {
          for (int j = 0 ; j < COLS ; j++) {
            if ( (i + j) % 2 == 0 ) {
              lc1.setLed(i / 8 , i % 8, j, false);
            }
          }
        }
        delay(300);
        for (int i = 0 ; i < ROWS ; i++) {
          for (int j = 0 ; j < COLS ; j++) {
            lc1.setLed(i / 8 , i % 8, j, false);
          }
        }
        delay(300);
        for (int i = 0 ; i < ROWS ; i++) {
          for (int j = 0 ; j < COLS ; j++) {
            Table[i][j] = 0;
          }
        }
      }
    }
}

void RotateShape(Shape shape){
    Shape temp = CopyShape(shape);
    int i, j, k, width;
    width = shape.width;
    for(i = 0; i < width ; i++){
        for(j = 0, k = width-1; j < width ; j++, k--){
            shape.array[i][j] = temp.array[k][i];
        }
    }
    DeleteShape(temp);
}

void WriteToTable(){
    int i, j;
    for(i = 0 ; i < current.width ; i++){
        for(j = 0 ; j < current.width ; j++){
            if(current.array[i][j])
                Table[current.row+i][current.col+j] = current.array[i][j];
        }
    }
}

void lineDetector(){
    int i, j, sum, count=0;
    for(i = 0 ; i < ROWS; i++){
        sum = 0;
        for(j = 0 ; j < COLS ; j++) {
            sum += Table[i][j];
        }
        if(sum == COLS){
            count++;
            int l, k;
            for(k = i ; k >= 1 ; k--)
                for(l = 0 ; l < COLS ; l++)
                    Table[k][l]=Table[k-1][l];
            for(l = 0 ; l < COLS ; l++)
                Table[k][l]=0;
        }
    }
}

void UpdateMap(){
    //todo this
    char Buffer[ROWS][COLS] = {0};
    int i, j;
    for(i = 0; i < current.width ;i++){
        for(j = 0; j < current.width ; j++){
            if(current.array[i][j])
                Buffer[current.row+i][current.col+j] = current.array[i][j];
        }
    }

    for(i = 0; i < ROWS ; i++){
        for(j = 0; j < COLS ; j++){
            lc1.setLed(i / 8 , i % 8 , j , false);
            if(Table[i][j] + Buffer[i][j]) {
                lc1.setLed( i / 8 , i % 8 , j , true);
            }
        }
    }
}

void InputProccesor(int action){
    //todo this
    Shape temp = CopyShape(current);
    switch(action){
        case DOWN:
            temp.row++;
            if(CheckPosition(temp))
                current.row++;
            else {
                WriteToTable();
                lineDetector();
                GetNewShape();
            }
            break;
        case RIGHT:
            temp.col++;
            if(CheckPosition(temp))
                current.col++;
            break;
        case LEFT:
            temp.col--;
            if(CheckPosition(temp))
                current.col--;
            break;
        case ROTATE:
            RotateShape(temp);
            if(CheckPosition(temp))
                RotateShape(current);
            break;
    }
    DeleteShape(temp);
    UpdateMap();
}
