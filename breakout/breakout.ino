/***********************************************************\
              IFPB - Curso de Engenharia Elétrica

Descrição do projeto:
Jogo Breakout controlado via Bluetooth através do 
sensor de orientação de um dispositivo móvel com
sistema operacional Android utilizando o aplicativo
Joystick BT

Desenvolvido por:
Marieliton Mendes Barbosa (marielitonmb@gmail.com)

Abril/2012
\***********************************************************/

/*
Funcionamento do aplicativo Joystick BT:
- valerá 0o quando o dispositivo estiver plano em relação ao
solo, estando nesta forma o Arduino não recebe nenhum dado
- varia até 90o quando o lado direito do dispositivo estiver
erguido, ou seja, a tela apontando para a esquerda, estando
nesta forma o Arduino receberá o caracter 'e'
- diminui até -90o quando o lado esquerdo do dispositivo estiver
erguido, ou seja, a tela apontando para a direita, estando
nesta forma o Arduino receberá o caracter 'd'
*/

// Bibliotecas-----------------------------------------------------------
#include <TVout.h>
#include <fontALL.h>
#include "TVOlogo.h"

TVout TV;

// Tamanho da tela-------------------------------------------------------
#define Tela_x 120
#define Tela_y 96

// Parâmetros para o jogo------------------------------------------------
#define Lin_Bloc 40  // Número de linhas de blocos
#define Tam_Raq 10  // Tamanho da raquete

// Controle do jogo------------------------------------------------------
int posRaq;         // Posição da raquete
int xBola, yBola;   // Posição da bola
int dxBola, dyBola; // Velocidade da bola
int nBlocos;        // Número de blocos a apagar
int pontos = 0;     // Pontuação do jogo
int vidas = 5;      // Quantidade de vidas no jogo
int buzzer = 5;     // Buzzer responsável pelos sons do jogo

// Variável para recepção dos dados--------------------------------------
char rx;

// Leitura dos dados via bluetooth---------------------------------------
void EntraDados() {
  if (Serial.available() > 0) {
    rx = Serial.read();
  }
}

// Logomarca do TVout----------------------------------------------------
void Intro() {
  unsigned char w, l, wb;
  int index;
  w = pgm_read_byte(TVOlogo);
  l = pgm_read_byte(TVOlogo+1);
  if (w&7)
    wb = w/8 + 1;
  else
    wb = w/8;
  index = wb*(l-1) + 2;
  for ( unsigned char i = 1; i < l; i++ ) {
    TV.bitmap((TV.hres() - w)/2,0,TVOlogo,index,w,i);
    index-= wb;
    TV.delay(50);
  }
  for (unsigned char i = 0; i < (TV.vres() - l)/2; i++) {
    TV.bitmap((TV.hres() - w)/2,i,TVOlogo);
    TV.delay(50);
  }
  TV.delay(5000);
  TV.clear_screen();
}

// Tela inicial----------------------------------------------------------
void Apresentacao() {
  Intro();
  TV.select_font(font8x8ext);
  TV.println(27,45,"BREAKOUT");
  delay(10000);
  TV.clear_screen();
  TV.select_font(font4x6);
  TV.println(25,45,"Desenvolvido por:");
  TV.print(10,55,"Marieliton Mendes Barbosa");
  delay(10000);
  TV.clear_screen();
}

// Coloca uma nova bola em jogo------------------------------------------
void NovaBola() {
  if (vidas > 0) {
    yBola = Tela_y - 3;
    xBola = random (1, Tela_x-1);
    dxBola = 1;
    dyBola = -1;
    TV.set_pixel(xBola, yBola, 1);
    TV.set_pixel(xBola+1, yBola, 1);
    TV.set_pixel(xBola, yBola+1, 1);
    TV.set_pixel(xBola+1, yBola+1, 1);
  }
}

// Inicia um novo jogo---------------------------------------------------
void NovoJogo() {
  TV.clear_screen();
  int x = 0;
  int y = 0;
  pontos = 0;
  for (y = 0; y < Lin_Bloc; y++)
    for (x = 0; x < Tela_x; x++)
      if (((y & 3) != 3) && ((x & 3) != 3))
        TV.set_pixel (x, y, 1);
  posRaq = (Tela_x - Tam_Raq) / 2;
  for (x = 0; x < Tam_Raq; x++)
    TV.set_pixel (posRaq+x, Tela_y - 1, 1);
  nBlocos = Lin_Bloc * Tela_x / 16;
  NovaBola();
}

// Fim do jogo-----------------------------------------------------------
void FimdeJogo() {
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.print(42,35,"FIM");
  TV.print(46,45,"DE");
  TV.print(38,55,"JOGO");
  TV.select_font(font4x6);
  TV.print(0,90,"Pontos= ");
  TV.print(29,90,pontos);
  digitalWrite(buzzer, LOW);
}

// Completou o jogo------------------------------------------------------
void CompletouJogo() {
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.print(25,35,"PARABENS,");
  TV.print(15,45,"ARREBENTOU");
  TV.print(33,55,"GERAL!");
  TV.select_font(font4x6);
  TV.print(0,90,"Pontos= ");
  TV.print(30,90,pontos);
  digitalWrite(buzzer, LOW);
}

// Apaga um bloco--------------------------------------------------------
void ApagaBloco(int x, int y) {
  // Determina o ponto que bateu
  if (dyBola == 1)
    y++;
  if (dxBola == 1)
    x++;
  // Determina o canto superior esquerdo do bloco
  x = x & ~3;
  y = y & ~3;
  // Apaga
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      TV.set_pixel(x+i, y+j, 0);
}

// Testa se bateu em um bloco--------------------------------------------
int BateuBloco(int x, int y) {
  return TV.get_pixel(x, y) || TV.get_pixel(x+1, y) ||
  TV.get_pixel(x, y+1) || TV.get_pixel(x+1, y+1);
}

// Move a bola (retorna 'true' se a bola saiu da tela)-------------------
int MoveBola() {
  int nx, ny;
  nx = xBola + dxBola;
  ny = yBola + dyBola;
  TV.set_pixel(xBola, yBola, 0);
  TV.set_pixel(xBola+1, yBola, 0);
  TV.set_pixel(xBola, yBola+1, 0);
  TV.set_pixel(xBola+1, yBola+1, 0);
  if (nx < 0) {
    nx = 0;
    dxBola = 1;
  }
  else if (nx == Tela_x-1) {
    nx--;
    dxBola = -1;
  }
  if (ny < 0) {
    ny = 0;
    dyBola = 1;
  }
  else if (ny == Tela_y-1) {
    // Saída de som
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
    delay(100);
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
    return true; // A bola escapou
  }
  if (ny == (Tela_y-2)) {
    #ifdef TESTE
    if (1)
    #else
    if (TV.get_pixel(nx, ny+1))
    #endif {
      // A bola bateu na raquete
      ny -= 2;
      dyBola = -1;
    }
  }
  else if (BateuBloco(nx, ny)) {
    // A bola bateu em um bloco
    ApagaBloco (nx, ny);
    nBlocos--;
    if (nBlocos == 0) {
      CompletouJogo();
    }
    // Saída de som
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    pontos = pontos + 10;
    dyBola = -dyBola;
    nx = xBola;
    ny = yBola;
  }
  xBola = nx;
  yBola = ny;
  TV.set_pixel (xBola, yBola, 1);
  TV.set_pixel (xBola+1, yBola, 1);
  TV.set_pixel (xBola, yBola+1, 1);
  TV.set_pixel (xBola+1, yBola+1, 1);
  return false;
}

// Movimentação da raquete---------------------------------------------
void MoveRaquete() {
  if ((rx == 'e') && (posRaq > 0)) {
    posRaq--; // A raquete vai para a esquerda
    TV.set_pixel(posRaq, Tela_y-1, 1);
    TV.set_pixel(posRaq+Tam_Raq, Tela_y-1, 0);
  }
  else if ((rx == 'd') && (posRaq < (Tela_x - Tam_Raq))) {
    TV.set_pixel(posRaq, Tela_y-1, 0);
    TV.set_pixel(posRaq+Tam_Raq, Tela_y-1, 1);
    posRaq++; // A raquete vai para a direita
  }
}

// Programa principal--------------------------------------------------
void setup() {
  TV.begin(PAL,120,96);
  TV.select_font(font6x8); // Tamanho da fonte na tela
  TV.delay(2000);
  Serial.begin(9600); // Taxa de transmissão de dados
  pinMode(buzzer, OUTPUT);
  Apresentacao();
  NovoJogo();
}

void loop() {
  EntraDados();
  MoveRaquete();
  delay (50);
  MoveRaquete();
  delay (50);
  MoveRaquete();
  //delay(50); // Se retirar este delay a bola se moverá mais rápida
  if (MoveBola()) {
    vidas--; // Decrementa 1 vida
    delay(3000);
    NovaBola();
    if (vidas == 0) FimdeJogo();
  }
}