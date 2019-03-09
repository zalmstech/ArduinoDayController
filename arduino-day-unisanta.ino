/********************************************************
      Arduino Day Unisanta
        Código exemplo para introdução a IOT

     Palestrantes:
      Bruno Raphael
      Daniel Zanutto

    Ultima alteração: 09/03/2019
*********************************************************
*/

////////////////////////////////////////////////////////// INCLUSÃO DE BIBLIOTECAS

#include <DHTesp.h>                 //Biblioteca para leitura do sensor DHT11
#include <ESP8266WiFi.h>            //Biblioteca para tratamento do Wifi
#include <PubSubClient.h>           //Biblioteca para tratamento MQTT

////////////////////////////////////////////////////////// VARIAVEIS MQTT

#define ID_ALUNO          "danielz"
#define MQTT_SUB_TOPIC    "zalms/arduinoday/"ID_ALUNO"/comandos"     //Topico para escuta
#define MQTT_PUB_TOPIC    "zalms/arduinoday/"ID_ALUNO"/feedback"     //Topico para publicar
#define MQTT_SERVER       "IPdoservidor"                            //Endereço do Broker
#define MQTT_SERVER_PORT  1883                                            //Porta do servidor
#define MQTT_USER         "usuario"                                       //Usuario do Broker
#define MQTT_PASSWORD     "huehuebr"                                      //Senha do Broker

////////////////////////////////////////////////////////// VARIAVEIS MQTT

const char* WIFI_SSID     = "nomedowifi";
const char* WIFI_PASSWORD = "senhadowifi";

////////////////////////////////////////////////////////// VARIAVEIS GLOBAIS

DHTesp dht;                     //Instância do sensor DHT
WiFiClient wifiClient;          //Instância do Cliente WiFi
PubSubClient mqttClient(wifiClient);  //Instância do Cliente MQTT

////////////////////////////////////////////////////////// MAIN SETUP

void setup() {
  
  Serial.begin(115200);
  Serial.println("Iniciando Setup");

  initWiFi();
  initMQTT();
  initIO();
  
}

////////////////////////////////////////////////////////// MAIN LOOP

void loop() {

  WiFiMQTTCheck();        //Verifica conexão WiFi e MQTT
  mqttClient.loop();            //Mantém comunicação MQTT

}

////////////////////////////////////////////////////////// ENVIO DO ESTADO DAS LEITURAS
/*  Método: sendStates
    Função: Captura os dados de leituras, atualiza nas variáveis e envia via MQTT
*/
void sendReadings() {

  String leituras = "{temperatura:" + getDHTTemperature() + ",umidade:" + getDHTHumidity() + "}";
  mqttClient.publish("zalms/arduinoday/"ID_ALUNO"/feedback", leituras.c_str());

}

////////////////////////////////////////////////////////// LEITURA DE TEMPERATURA
/*  Método: getDHTTemperature
    Função: Executa leitura da temperatura pelo sensor DHT
*/
String getDHTTemperature() {

  delay(dht.getMinimumSamplingPeriod());
  return String(dht.getTemperature());

}

////////////////////////////////////////////////////////// LEITURA DE UMIDADE
/*  Método: getDHTHumidity
    Função: Executa leitura da humidade pelo sensor DHT
*/
String getDHTHumidity() {

  delay(dht.getMinimumSamplingPeriod());
  return String(dht.getHumidity());

}

////////////////////////////////////////////////////////// WIFI E MQTT CHECK
/*  Método: WiFiMQTTCheck
    Função: Checa as conexões WiFi e MQTT
*/
void WiFiMQTTCheck() {
  
  if (WiFi.status() == WL_CONNECTED) {
    if (mqttClient.connected()) {
      return;
    }
    else {
      Serial.println("Conexão com Broker perdida, reconectando...");
      initMQTT();
    }
  }
  else {
    Serial.println("Conexão com Wi-Fi perdida, reconectando...");
    initWiFi();
  }

}

////////////////////////////////////////////////////////// TRATAMENTO DE COMANDOS
/*  Método: mqtt_callback
    Função: Método executado sempre que uma mensagem chegar no tópico inscrito
*/
void mqtt_callback(char* topic, byte * payload, unsigned int length) {

  String msg;
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.print((String)topic);
  Serial.print(" ");
  Serial.println(msg);

  ////////////////////////////////Receber Leituras
  if (msg.equals("getReadings"))
  {
    sendReadings();
  }
  ////////////////////////////////ESP LED
  if (msg.equals("turnOn"))
  {
    digitalWrite(2, !HIGH);
  }
  if (msg.equals("turnOff"))
  {
    digitalWrite(2, !LOW);
  }

}

////////////////////////////////////////////////////////// INIT METHODS
/*  Método: initWiFi
    Função: Tenta conectar no WiFi com configurações previamente
            estabelecidas.
*/
void initWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado a rede: ");
    Serial.print("\"");
    Serial.print(WiFi.SSID());
    Serial.print("\"");
    Serial.print(" IP local: ");
    Serial.println(WiFi.localIP());
  }
  else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

}

/*  Método: initMQTT
    Função: Configura a conexão MQTT com as configurações previamente
            estabelecidas.
*/
void initMQTT() {

  Serial.println("Conectando ao o Broker MQTT");

  mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqttClient.setCallback(mqtt_callback);

  if (mqttClient.connected()) {
    Serial.print("MQTT");
    Serial.print("\t");
    Serial.print("OK");
  }
  else if (!mqttClient.connected()) {
    if (mqttClient.connect(ID_ALUNO, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Conectado ao o Broker MQTT com sucesso!");
      mqttClient.subscribe(MQTT_SUB_TOPIC);
    }
    else {
      Serial.println("Falha ao conectar com o Broker MQTT");
      delay(500);
    }
  }
  
}

/*  Método: initIO
    Função: inicializa e configura as portas e recursos Digitais In e Out
*/
void initIO() {
  
  pinMode(2, OUTPUT);
  dht.setup(14, DHTesp::DHT11);

}
