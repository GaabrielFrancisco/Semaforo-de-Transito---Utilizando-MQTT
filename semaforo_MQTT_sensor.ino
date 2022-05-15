/* Headers */ 
/* Autor : Gabriel Francisco Barbosa Silva */
#include <WiFi.h> /* Header para uso das funcionalidades de wi-fi do ESP32 */
#include <PubSubClient.h>  /*  Header para uso da biblioteca PubSubClient */
 
/* Defines do MQTT */
/*
 * 
 */
/* IMPORTANTE: recomendamos fortemente alterar os nomes
               desses tópicos. Caso contrário, há grandes
               chances de você enviar e receber mensagens de um ESP32
               de outra pessoa.
*/
/* Tópico MQTT para recepção de informações do broker MQTT para ESP32 */
#define TOPICO_SUBSCRIBE "SEMAFORO_recebe_informacao"   
/* Tópico MQTT para envio de informações do ESP32 para broker MQTT */
#define TOPICO_PUBLISH   "SEMAFORO_envia_informacao"  
/* id mqtt (para identificação de sessão) */
/* IMPORTANTE: este deve ser único no broker (ou seja, 
               se um client MQTT tentar entrar com o mesmo 
               id de outro já conectado ao broker, o broker 
               irá fechar a conexão de um deles).
*/
#define ID_MQTT  "INCB_Cliente_MQTT"     
/*  Variáveis e constantes globais */
/* SSID / nome da rede WI-FI que deseja se conectar */
const char* SSID = "ROILIZ_2G"; 
/*  Senha da rede WI-FI que deseja se conectar */
const char* PASSWORD = "knn1056c"; 
  
/* URL do broker MQTT que deseja utilizar */
const char* BROKER_MQTT = "broker.hivemq.com"; 
/* Porta do Broker MQTT */
int BROKER_PORT = 1883;
 
unsigned long tempo_atual = 0;
int tempo = 0;
String estado = "vermelho", msg = "normal";
bool status_yellow_led = 0;

#define red_led 14    //Declara o led vermelho no pino 14 do ESP32
#define yellow_led 12 //Declara o led vermelho no pino 12 do ESP32
#define green_led 13  //Declara o led vermelho no pino 13 do ESP32

/* Variáveis e objetos globais */
WiFiClient espClient;      //Instancia o objeto espClient na Classe WiFiClient da biblioteca WiFi.h
PubSubClient MQTT(espClient); //Instancia o objeto MQTT na Classe PubSubClient da biblioteca PubSubClient.h
  
//Prototypes
void init_serial(void);
void init_wifi(void);
void init_mqtt(void);
void reconnect_wifi(void); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void verifica_conexoes_wifi_mqtt(void);
 
/* 
 *  Implementações das funções
 */
void setup() 
{
    init_serial();
    init_wifi();
    init_mqtt();
    //Captura o valor do millis para fazer a lógica de tempo do semáforo
    tempo_atual = millis();
    //Definição das portas do ESP32 como de saída
    pinMode(red_led, OUTPUT);
    pinMode(yellow_led, OUTPUT);
    pinMode(green_led, OUTPUT);
}
  
/* Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
*          o que está acontecendo.
* Parâmetros: nenhum
* Retorno: nenhum
*/
void init_serial() 
{
    Serial.begin(115200); //Taxa de transmissão da comunicação serial
}
 
/* Função: inicializa e conecta-se na rede WI-FI desejada
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void init_wifi(void) 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconnect_wifi();
}
  
/* Função: inicializa parâmetros de conexão MQTT(endereço do  
 *         broker, porta e seta função de callback)
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void init_mqtt(void) 
{
    /* informa a qual broker e porta deve ser conectado */
    MQTT.setServer(BROKER_MQTT, BROKER_PORT); 
    /* atribui função de callback (função chamada quando qualquer informação do 
    tópico subescrito chega) */
    MQTT.setCallback(mqtt_callback);            
}
  
/* Função: função de callback 
 *          esta função é chamada toda vez que uma informação de 
 *          um dos tópicos subescritos chega)
 * Parâmetros: nenhum
 * Retorno: nenhum
 * */
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    msg = "";
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    Serial.print("[MQTT] Mensagem recebida: ");
    Serial.println(msg);
    if (msg == "alerta"){
      Serial.println("Semaforo em alerta, piscando amarelo.");
      digitalWrite(red_led, HIGH);
      digitalWrite(yellow_led, HIGH);
      digitalWrite(green_led, HIGH);
    }
}
  
/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 *          em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnect_mqtt(void) 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
/* Função: reconecta-se ao WiFi
 * Parâmetros: nenhum
 * Retorno: nenhum
*/
void reconnect_wifi() 
{
    /* se já está conectado a rede WI-FI, nada é feito. 
       Caso contrário, são efetuadas tentativas de conexão */
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void verifica_conexoes_wifi_mqtt(void)
{
    /* se não há conexão com o WiFI, a conexão é refeita */
    reconnect_wifi(); 
    /* se não há conexão com o Broker, a conexão é refeita */
    if (!MQTT.connected()) 
        reconnect_mqtt(); 
} 
 
/* programa principal */
void loop() 
{   
    /* garante funcionamento das conexões WiFi e ao broker MQTT */
    verifica_conexoes_wifi_mqtt();
    /* Envia frase ao broker MQTT */
    bool ir_sensor = digitalRead(27);
    //Serial.println(ir_sensor);
    tempo = millis() - tempo_atual;

    //Liga o LED vermelho quando o tempo de 11 segundos estourar
    if ((tempo > 11000)and(estado == "amarelo")and(msg == "normal")and(ir_sensor == 1)){
      estado = "vermelho";
      MQTT.publish(TOPICO_PUBLISH, "Semaforo Vermelho.");
      digitalWrite(red_led, LOW);
      digitalWrite(yellow_led, HIGH);
      digitalWrite(green_led, HIGH);
      tempo_atual = millis();
    }

    //Liga o LED amarelo quando o tempo de 8 segundos estourar
    else if ((tempo > 8000)and(estado == "verde")and(msg == "normal")and(ir_sensor == 1)){
      estado = "amarelo";
      MQTT.publish(TOPICO_PUBLISH, "Semaforo Amarelo.");
      digitalWrite(red_led, HIGH);
      digitalWrite(yellow_led, LOW);
      digitalWrite(green_led, HIGH);
    }

    //Liga o LED verde quando o tempo de 4 segundos estourar
    else if ((tempo > 4000)and(estado == "vermelho")and(msg == "normal")and(ir_sensor == 1)){
      estado = "verde";
      MQTT.publish(TOPICO_PUBLISH, "Semaforo Verde.");
      digitalWrite(red_led, HIGH);
      digitalWrite(yellow_led, HIGH);
      digitalWrite(green_led, LOW);
    }

    //Se receber a mensagem "alerta" via MQTT o semáforo ficará com o Led Amarelo piscando
    if ((msg == "alerta") or (ir_sensor == 0)){
      MQTT.publish(TOPICO_PUBLISH, "Semaforo em ALERTA.");
      digitalWrite(red_led, HIGH);
      digitalWrite(green_led, HIGH);
      status_yellow_led = !status_yellow_led;
      digitalWrite(yellow_led, status_yellow_led);
      tempo_atual = millis();
    }
    
    /* keep-alive da comunicação com broker MQTT */    
    MQTT.loop();
    /* Agurda 1 segundo para próximo envio */
    delay(1000);   
}
