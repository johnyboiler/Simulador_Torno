#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 12 
#define OLED_SCL 14 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define JOY_Y A0    
#define JOY_SW 0    

enum EstadoMenu {
  VER_POSICIONES,
  SELECCIONAR_EJE,
  EDITAR_EJE_Z,
  EDITAR_EJE_X
};

EstadoMenu estadoActual = VER_POSICIONES;
int cursorSeleccion = 0; 

int posZ = 0;
int posX = 0;

unsigned long ultimoMovimiento = 0;
unsigned long ultimoBoton = 0;
const int retardoJoystick = 150; 
const int retardoBoton = 300;    

bool joystickEnReposo = true;

void setup() {
  pinMode(JOY_SW, INPUT_PULLUP); 
  
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) { delay(10); } 
  }
}

void loop() {
  unsigned long tiempoActual = millis();
  
  int lecturaY = analogRead(JOY_Y);
  bool botonPresionado = (digitalRead(JOY_SW) == LOW); 

  // 1. Lógica del Botón
  if (botonPresionado && (tiempoActual - ultimoBoton > retardoBoton)) {
    ultimoBoton = tiempoActual;
    
    if (estadoActual == VER_POSICIONES) {
      estadoActual = SELECCIONAR_EJE;
    } 
    else if (estadoActual == SELECCIONAR_EJE) {
      if (cursorSeleccion == 0) estadoActual = EDITAR_EJE_Z;
      else estadoActual = EDITAR_EJE_X;
    } 
    else {
      estadoActual = VER_POSICIONES;
    }
  }

  // 2. NUEVA ZONA NEUTRAL: Adaptada al centro físico de tu joystick (700-800)
  if (lecturaY > 600 && lecturaY < 850) {
    joystickEnReposo = true;
  }

  // 3. Lógica del Joystick con nuevos umbrales
  if (tiempoActual - ultimoMovimiento > retardoJoystick) {
    
    if (lecturaY > 900) { 
      // Movimiento hacia un lado (Cerca del 1024)
      if (estadoActual == SELECCIONAR_EJE && joystickEnReposo) {
        cursorSeleccion = 0;
        joystickEnReposo = false; 
      } 
      else if (estadoActual == EDITAR_EJE_Z) { posZ++; }
      else if (estadoActual == EDITAR_EJE_X) { posX++; }
      
      ultimoMovimiento = tiempoActual;
    } 
    else if (lecturaY < 500) { 
      // Movimiento hacia el otro lado (Cerca del 6)
      if (estadoActual == SELECCIONAR_EJE && joystickEnReposo) {
        cursorSeleccion = 1;
        joystickEnReposo = false; 
      } 
      else if (estadoActual == EDITAR_EJE_Z) { posZ--; }
      else if (estadoActual == EDITAR_EJE_X) { posX--; }
      
      ultimoMovimiento = tiempoActual;
    }
  }

  // 4. Renderizar Interfaz
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (estadoActual == VER_POSICIONES)
  {
    // Encabezado invertido sólido
    display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK); // Texto negro sobre fondo blanco
    display.setCursor(15, 3);
    display.print("TORNO AUTOMATICO");

    display.setTextColor(SSD1306_WHITE); // Volver a texto blanco

    // Coordenadas en formato grande y dividido
    display.setTextSize(1);
    display.setCursor(0, 22);
    display.print("Z:");
    display.setTextSize(2);
    display.print(posZ);

    display.setTextSize(1);
    display.setCursor(64, 22);
    display.print("X:");
    display.setTextSize(2);
    display.print(posX);

    // Línea separadora
    display.drawLine(0, 42, SCREEN_WIDTH, 42, SSD1306_WHITE);

    // Compensación de herramienta (pronto lo haremos variable)
    display.setTextSize(1);
    display.setCursor(0, 48);
    display.print("Herramienta: 2.5 mm");
  }
  else if (estadoActual == SELECCIONAR_EJE) {
    display.setCursor(0, 0);
    display.println("--- ELEGIR EJE ---");
    
    display.setCursor(10, 20);
    if (cursorSeleccion == 0) display.print(">");
    display.print(" Configurar Eje Z");
    
    display.setCursor(10, 40);
    if (cursorSeleccion == 1) display.print(">");
    display.print(" Configurar Eje X");
  } 
  else if (estadoActual == EDITAR_EJE_Z) {
    display.setCursor(0, 0);
    display.println("--- AJUSTANDO Z ---");
    display.setTextSize(2);
    display.setCursor(20, 30);
    display.print(posZ); display.print(" mm");
  }
  else if (estadoActual == EDITAR_EJE_X) {
    display.setCursor(0, 0);
    display.println("--- AJUSTANDO X ---");
    display.setTextSize(2);
    display.setCursor(20, 30);
    display.print(posX); display.print(" mm");
  }

  display.display();
}