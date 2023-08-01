#include <Adafruit_GFX.h> //-------------------------------Librería gráfica
#include <FastLED_NeoMatrix.h> //-------Librería para el control de la matrix y uso de color (fusiona NeoMatrix con FastLED)
#include <FastLED.h> 

//Definiciones:
//Las sub-matrices apiladas forman una matriz.

#define PIN 9 //------------------------------------------PIN de salida de datos de la placa Arduino UNO
#define sub_MatrixWidth 8 //------------------------------Ancho de la sub-matriz (En número de leds)
#define sub_MatrixHeight 8 //-----------------------------Alto de la sub-matriz
#define super_MatrixWidth 2 //----------------------------Ancho de la matriz (En número de submatrices)
#define super_MatrixHeight 2 //---------------------------Alto de la matriz
#define matrixWidth 16 //---------------------------------Ancho de la matriz (En número de leds)
#define matrixHeight 16 //--------------------------------Alto de la matriz
#define total_Leds (matrixWidth*matrixHeight) //----------Número total de leds
CRGB leds[total_Leds];


//---------Declaración del objeto 'matrix' de la clase 'FastLED_NeoMatrix'--------

FastLED_NeoMatrix matrix = FastLED_NeoMatrix(leds, sub_MatrixWidth, sub_MatrixHeight, super_MatrixWidth,super_MatrixHeight, 
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT //------------------Posición de la entrada de datos (DIN) en la sub-matriz
+ NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE //--------------Dirección de la transmisión de datos en los leds de la sub-matriz
+ NEO_TILE_BOTTOM + NEO_TILE_RIGHT //----------------------Posición de la primera submatriz que recibe los datos
+NEO_TILE_ROWS+NEO_TILE_PROGRESSIVE //--------------------Dirección de la transmisión de datos en las sub-matrices de la matriz
);


CRGB color;
int counter;
int lastState;
int currentState;  //---variables de lectura de los botones
int automatic;


TBlendType    blending;    
CRGB purple = CHSV( HUE_PURPLE, 255, 255);
CRGB green  = CHSV( HUE_GREEN, 255, 255);
CRGB red  = CHSV( HUE_RED, 255, 255);           //---Colores
CRGB blue  = CHSV( HUE_BLUE, 255, 255);
CRGB yellow  = CHSV( HUE_YELLOW, 255, 255);
CRGB orange = CRGB::Orange;
CRGB darkOrange = CRGB::DarkOrange;
CRGB cyan = CRGB::Cyan;
CRGB olive = CRGB::Olive;
CRGB magenta  = CRGB::MediumVioletRed;
CRGB lime  = CRGB::Lime;
CRGB fuchsia = CRGB::Fuchsia;
CRGBPalette16 rainbowPalette;  //----Paleta pre-definida
CRGBPalette16 palette; //-----Paleta personalizada



int internalMatrix[16][16]={0};
int newInternalMatrix[16][16]={0}; //----Matrices internas para aplicar y actualizar las reglas del juego

void setup() {
    
  //FastLED.addLeds<Tipo de led, Pin de datos, orden del Color>(leds,leds totales)
  FastLED.addLeds<WS2812,PIN, GRB>(leds,total_Leds);
  
  FastLED.setBrightness(240);
    
  rainbowPalette = RainbowColors_p;  //--------Paleta de colores pre-definida, para los estados consecutivos
  blending = LINEARBLEND;            //------Gradiente lineal  
  palette = CRGBPalette16(
  purple, green, red, blue,
  yellow, orange, darkOrange, cyan,
  olive, magenta, lime, fuchsia,
  purple,purple, red, fuchsia);


  
  pinMode(A1,INPUT);                 //---Declaración de los puertos de los botones en Arduino
  pinMode(A2,INPUT);

  color = palette[random8(0,16)];  

  fillMatrix(0);
  matrix.begin(); 
  FastLED.show(); 
  Serial.begin(9600);
 
}


void loop(){
  
      automatic = digitalRead(A2); //----Botón 2
      initialState();
      counter=0;
      
      while(automatic == HIGH){
        if(counter<20){               //---------- El estado inicial usado es estable en la generación 20.
          showMatrix(color);
          matrix.show();
          FastLED.show();
          delay(1000);
          counter++;
          rules();
          updateMatrix();
          color = palette[random8(0,16)];   
         }else{
          initialState();
          counter = 0;
          }
         automatic = digitalRead(A2);
      }
      
      initialState();   
      counter=0;
       
 
       lastState =digitalRead(A1);
       while(automatic == LOW){
        if(counter<20){
           showMatrix(color);
           matrix.show();
           FastLED.show(); 
           currentState = digitalRead(A1);  
           if(lastState != currentState){
              rules();
              updateMatrix();
              color = palette[random8(0,16)];   
              lastState = currentState; 
              counter++;          
            }
        }else{
          initialState();
          counter = 0;
          }
          automatic = digitalRead(A2);
        }

}

void initialState(){
  for(int x=0;x<16;x++){
    for(int y=0;y<16;y++){
      internalMatrix[x][y] = 0;
    }    
  }
  internalMatrix[6][6] = 1;
  internalMatrix[7][6] = 1;
  internalMatrix[7][7] = 1;
  internalMatrix[8][6] = 1;          //---------Estado inicial del juego
  internalMatrix[8][7] = 1;
  internalMatrix[8][8] = 1;
  internalMatrix[9][7] = 1;
  internalMatrix[9][8] = 1;
  internalMatrix[10][8] = 1;
  
  }

  
void showMatrix(CRGB lifeColor){      //----------Muestra la matriz en la pantalla
  for(int x=0;x<16;x++){
    for(int y=0;y<16;y++){
      if (internalMatrix[x][y] == 1){
         matrix.drawPixel(x,y,lifeColor);  //---Dibuja el pixel vivo
      }
      else {
         matrix.drawPixel(x,y,matrix.Color(0,0,0)); //---Dibuja el pixel muerto
      }
    }    
  }
}


int aroundSum(int x,int y){ //---------Suma los estados (0 ó 1) de las celdas vecinas a (x,y)
  return (internalMatrix[x][y-1] + internalMatrix[x][y+1] + internalMatrix[x-1][y] + internalMatrix[x+1][y] 
  + internalMatrix[x-1][y-1] + internalMatrix[x-1][y+1] + internalMatrix[x+1][y-1]+ internalMatrix[x+1][y+1] );
 
}


void rules(){ //----------Aplica las reglas del juego
   for(int x=1;x<15;x++){
    for(int y=1;y<15;y++){
            int sumStates = aroundSum(x,y);
            if(sumStates>3 or sumStates <=1){  //------Regla de muerte
                newInternalMatrix[x][y] = 0;
            }
            if(sumStates==3 and internalMatrix[x][y]== 0){    //----------Regla de resurrección
                newInternalMatrix[x][y] = 1;
            }
            if((sumStates==3 or sumStates==2) and internalMatrix[x][y]== 1){ //---------Regla de supervivencia
                newInternalMatrix[x][y] = 1;
            }
     }
  }   
}

void updateMatrix(){
  for(int x=0;x<15;x++){
    for(int y=0;y<15;y++){
     internalMatrix[x][y]=newInternalMatrix[x][y];  //----Actualiza la matriz
    }    
  }
  newInternalMatrix[16][16]={0};
  
  
  }

void fillMatrix( uint8_t colorIndex) // Muestra la paleta pre-definida 'Rainbow' al inicio
{
    uint8_t brightness = 200;    
    for( int i = 0; i < total_Leds; i++) {
        leds[i] = ColorFromPalette(rainbowPalette, colorIndex, brightness,blending);
        colorIndex += 3;
    }
    delay(5000);
}








 //---------------------------Ejemplos de de estados iniciales--------------------------
 

//Número pi
 //internalMatrix[6][6] = 1;
  //internalMatrix[6][7] = 1;
  //internalMatrix[6][8] = 1;
  //internalMatrix[6][9] = 1;
  //internalMatrix[6][10] = 1;
  //internalMatrix[9][6] = 1;
  //internalMatrix[9][7] = 1;
  //internalMatrix[9][8] = 1;
  //internalMatrix[9][9] = 1;
  //internalMatrix[9][10] = 1;
  //internalMatrix[9][9] = 1;
  //internalMatrix[7][6] = 1;
  //internalMatrix[8][6] = 1; 

 
  
  
