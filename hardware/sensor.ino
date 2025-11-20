#include <WiFi.h>
#include <PubSubClient.h>

// ---------------- Criação de objetos ----------------
WiFiClient espClient;                 // Cliente Wi-Fi
PubSubClient MQTT(espClient);         // Cliente MQTT

#define Microphone 34  // pino analógico do microfone
int iniSample = 2048; // Valor mínimo do microfone

#define PIR_PIN 18

// ---------------- Configurações de rede e MQTT ----------------
const char* SSID = "Wokwi-GUEST";            // Nome da rede Wi-Fi
const char* PASSWORD = "";                   // Senha da rede Wi-Fi
const char* BROKER_MQTT = "20.46.254.134";   // Endereço IP do broker MQTT
const int BROKER_PORT = 1883;                // Porta do broker MQTT

// Tópicos MQTT de publicação
const char* DEVICE_ID = "sensor001";
const char* TOPICO_PUBLISH_PRESENCE = "/TEF/sensor001/attrs/presence";
const char* TOPICO_PUBLISH_NOISE = "/TEF/sensor001/attrs/noise";

const char* ID_MQTT = "fiware_001";          // ID de identificação do cliente MQTT

// ---------------- Funções auxiliares ----------------

// Inicializa a comunicação serial
void initSerial() {
  Serial.begin(115200);
}

// Conecta o ESP32 à rede Wi-Fi
void initWiFi() {
  Serial.println("------ Conectando ao Wi-Fi ------");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Configura o cliente MQTT
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

// Tenta reconectar ao broker MQTT caso a conexão seja perdida
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar ao Broker MQTT...");
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker!");
    } else {
      Serial.print("Falha na conexão. Código de erro: ");
      Serial.println(MQTT.state());
      Serial.println("Tentando novamente em 2 segundos...");
      delay(2000);
    }
  }
}

// Garante que Wi-Fi e MQTT estão conectados
void VerificaConexoesWiFIEMQTT() {
  if (!MQTT.connected()) {
    reconnectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
}

int microphone() {
  int sample = analogRead(Microphone);     // leitura analógica (0–4095)
  int delta = abs(sample - iniSample);     // diferença em relação ao silêncio

  // converte a variação para "decibéis" aproximados (escala simulada)
  int dB = map(delta, 0, 1024, 0, 100);
  if (dB > 100) dB = 100;

  // mostra os valores no Serial Monitor
  Serial.print("Sample: ");
  Serial.print(sample);
  Serial.print(" | Delta: ");
  Serial.print(delta);
  Serial.print(" | dB: ");
  Serial.println(dB);

  return dB;

}

bool salaOcupada() {
  int estado = digitalRead(PIR_PIN);
  Serial.print("PIR: ");
  Serial.println(estado);
  return estado == HIGH;
}

void publicarBarulho(int barulho) {
  String mensagem = String(barulho);
  Serial.print("Publicando barulho: ");
  Serial.println(mensagem);
  MQTT.publish(TOPICO_PUBLISH_NOISE, mensagem.c_str());
}

void publicarPresenca(bool presenca) {
  String mensagem = String(presenca);
  Serial.print("Publicando presenca: ");
  Serial.println(mensagem);
  MQTT.publish(TOPICO_PUBLISH_PRESENCE, mensagem.c_str());
}

void publicar() {

  int barulho = microphone();
  bool presencaPIR = salaOcupada();

  bool presenca = presencaPIR || (barulho > 0);

  publicarBarulho(barulho);
  publicarPresenca(presenca);

}

// ---------------- Setup e Loop principal ----------------
void setup() {
  
  initSerial();
  initWiFi();
  initMQTT();

  pinMode(PIR_PIN, INPUT);

}

void loop() {
  // Garante que Wi-Fi e MQTT estejam sempre conectados
  VerificaConexoesWiFIEMQTT();
  MQTT.loop();

  publicar();

  delay(100);

}