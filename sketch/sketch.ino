#include <WiFi.h>
#include <HTTPClient.h>

// Definição dos pinos para controle de LEDs
#define led_verde 41     // Pino para o LED verde
#define led_vermelho 40  // Pino para o LED vermelho
#define led_amarelo 10   // Pino para o LED amarelo

// Definição do pino para o botão
const int buttonPin = 18; // Pino para o botão
int buttonState = 0;      // Variável para armazenar o estado do botão
int lastButtonState = LOW; // Último estado do botão
unsigned long lastDebounceTime = 0; // Último tempo de debounce
const unsigned long debounceDelay = 50; // Atraso para debounce

// Contador de pressões do botão
int buttonPressCount = 0;

// Definição do pino e threshold do sensor LDR
const int ldrPin = 4;  // Pino para o sensor LDR
int threshold = 600;   // Limite para determinar se está claro ou escuro

// Temporizadores para os modos
unsigned long lastTime = 0;
unsigned long interval = 0;

// Estado atual
bool isNightMode = false;

void setup() {
  pinMode(led_amarelo, OUTPUT);
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  

  pinMode(buttonPin, INPUT);

  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);

  Serial.begin(9600);
  WiFi.begin("Wokwi-GUEST", "");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi!");
}

void loop() {
  // Leitura do sensor LDR
  int ldrStatus = analogRead(ldrPin);
  unsigned long currentTime = millis();

  // Modo noturno
  if (ldrStatus <= threshold) {
    if (!isNightMode) {
      isNightMode = true;
      Serial.println("Entrando no modo noturno.");
    }
    if (currentTime - lastTime >= 1000) {
      lastTime = currentTime;
      digitalWrite(led_amarelo, !digitalRead(led_amarelo)); // Pisca o LED amarelo
    }
  } else {
    if (isNightMode) {
      isNightMode = false;
      Serial.println("Entrando no modo convencional.");
      digitalWrite(led_amarelo, LOW); // Desliga o LED amarelo
    }

    // Alternância de LEDs no modo convencional
    if (currentTime - lastTime >= interval) {
      lastTime = currentTime;

      if (digitalRead(led_verde) == HIGH) {
        digitalWrite(led_verde, LOW);
        digitalWrite(led_amarelo, HIGH);
        interval = 2000; // 2 segundos para amarelo
      } else if (digitalRead(led_amarelo) == HIGH) {
        digitalWrite(led_amarelo, LOW);
        digitalWrite(led_vermelho, HIGH);
        interval = 5000; // 5 segundos para vermelho
      } else {
        digitalWrite(led_vermelho, LOW);
        digitalWrite(led_verde, HIGH);
        interval = 3000; // 3 segundos para verde
      }
    }
  }

  // Leitura e debounce do botão
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = currentTime;
  }
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        buttonPressCount++;
        Serial.print("Botão pressionado! Contagem: ");
        Serial.println(buttonPressCount);

        // Abre o semáforo após 1 segundo no modo fechado
        if (digitalRead(led_vermelho) == HIGH) {
          delay(1000);
          digitalWrite(led_vermelho, LOW);
          digitalWrite(led_verde, HIGH);
        }

        // Envia requisição após 3 pressões
        if (buttonPressCount == 3) {
          buttonPressCount = 0; // Reinicia contador
          if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            http.begin("http://www.google.com.br/");
            int httpResponseCode = http.GET();
            Serial.print("Requisição HTTP enviada. Código: ");
            Serial.println(httpResponseCode);
            http.end();
          }
        }
      }
    }
  }
  lastButtonState = reading;
}