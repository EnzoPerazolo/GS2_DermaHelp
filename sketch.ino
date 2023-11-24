#include <WiFi.h>
#include <ThingSpeak.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

WiFiClient  client;

unsigned long myChannelNumber = 1;

const char * myWriteAPIKey = "JPER8DV6YFPEBKF4";

int freq = 800;
int channel = 0;
int resolution = 8;

const int buttonPin = 16;   // Pino do botão
const int ledPin = 17;      // Pino do LED
const int buzzerPin = 18;   // Pino do buzzer


int buttonState = LOW;       // Variável para armazenar o estado do botão
int lastButtonState = LOW;   // Variável para armazenar o estado anterior do botão
unsigned long lastDebounceTime = 0;  // Variável para o último tempo de debounce
unsigned long debounceDelay = 10;    // Tempo de debounce

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

bool isOn = false;         // Variável para controlar o estado do loop
int statusBotao = 0;
int statusLed = 0;
int statusBuzzer = 0;

void setup() {

  Serial.begin(115200);

  ledcSetup(channel, freq, resolution);
  ledcAttachPin(18, channel);

  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado a: ");
  Serial.println(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("Aguarde 30 segundos para a primeira postagem");

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {

  int reading = digitalRead(buttonPin);  // Lê o estado do botão

  if (reading == LOW){
    statusBotao = 1;
  }else{
    statusBotao = 0;
  }

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        // Inverte o estado do loop ao pressionar o botão
        isOn = !isOn;
        delay(200); // Pequeno delay para estabilizar
      }
    }
  }

  lastButtonState = reading;

  if (isOn) {
    // Lógica para acender o LED e emitir som com o buzzer
    digitalWrite(ledPin, HIGH);
    ledcWriteTone(buzzerPin, 800);
    statusLed = 1;
    statusBuzzer = 1;
    delay(500);
    digitalWrite(ledPin, LOW);
    ledcWrite(buzzerPin, 0);
    delay(500);
  } else {
    // Lógica para apagar o LED e desligar o som do buzzer
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
    statusLed = 0;
    statusBuzzer = 0;
  }

  if ((millis() - lastTime) > timerDelay) {
    
    // Conecta ou reconecta a rede Wi-Fi
    if(WiFi.status() != WL_CONNECTED){
      // Informa mensagem de conexão no Monitor Serial
      Serial.print("Tentando conectar...");
      // Conecta (no início) ou reconecta (se "cair") a rede Wi-Fi a cada 5 segundos
      // Recebe os parâmetros com as credenciais da rede, definidas no cabeçalho do código
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password);
        delay(2000);
      }
      // Informa mensagem de conexão no Monitor Serial
      Serial.println("\nConectado.");
    }

  // Define uma variável "envio", que receberá os parâmetros
  // – Canal do ThingSpeak
  // – Chave de escrita da API do ThingSpeak
  ThingSpeak.setField(1, statusBotao);
  ThingSpeak.setField(2, statusLed);
  ThingSpeak.setField(3, statusBuzzer);
  
  int envio = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  // Caso o ThingSpeak retorne o valor 200, a requisição de envio dos dados foi bem sucedida
  // Escreve no Monitor Serial a mensagem de atualização
  if(envio == 200){
    Serial.println("ThingSpeak atualizado");
    Serial.print("Botão: ");
    Serial.println(statusBotao);
    Serial.print("LED: ");
    Serial.println(statusLed);
    Serial.print("Buzzer: ");
    Serial.println(statusBuzzer);
    Serial.println("");
 }
 // Caso o ThingSpeak retorne outro valor, a requisição de envio não foi bem sucedida
 // Escreve no Monitor Serial a mensagem de erro e o código do erro
  else{
    Serial.println("Problema na atualização do canal. Código do erro HTTP" + String(envio));
    
  }
  // Reinicia o temporizado
  lastTime = millis();
}
}