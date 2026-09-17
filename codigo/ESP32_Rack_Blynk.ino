#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// =================================================
// CONFIGURAÇÃO WI-FI
// =================================================

char ssid[] = "SUA_REDE_WIFI";
char pass[] = "SUA_SENHA_WIFI";

// =================================================
// CONFIGURAÇÃO BLYNK
// =================================================

#define BLYNK_TEMPLATE_ID "SEU_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "ESP32 control"
#define BLYNK_AUTH_TOKEN "SEU_BLYNK_AUTH_TOKEN"

// =================================================
// DHT11
// =================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =================================================
// HC-SR04
// =================================================

#define TRIG_PIN 5
#define ECHO_PIN 18

// =================================================
// LED E BUZZER
// =================================================

#define LED_PIN 26
#define BUZZER_PIN 25

// =================================================
// LIMITES
// =================================================

#define TEMPERATURA_ALTA 35.0
#define DISTANCIA_PORTA_ABERTA 60

// =================================================
// CONTROLE DO ALERTA DE TEMPERATURA
// =================================================

bool alertaTemperaturaEnviado = false;

// =================================================
// CONTROLE DA PORTA E ALARME
// =================================================

bool portaAberta = false;
bool portaEstavaAberta = false;
bool alarmeAtivo = false;

unsigned long inicioAlarme = 0;

const unsigned long DURACAO_ALARME = 10000;

// =================================================
// CONTROLE DOS BIPS
// =================================================

unsigned long ultimoBip = 0;

bool estadoBip = false;

const unsigned long INTERVALO_BIP = 300;

// =================================================
// BLYNK TIMER
// =================================================

BlynkTimer timer;

// =================================================
// FUNÇÃO PARA MEDIR DISTÂNCIA
// =================================================

float medirDistancia() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracao == 0) {
    return -1;
  }

  return duracao * 0.034 / 2;
}

// =================================================
// CONTROLE DO ALARME
// =================================================

void controlarAlarme() {

  if (!alarmeAtivo) {
    return;
  }

  unsigned long agora = millis();

  // Desliga o alarme após 10 segundos
  if (agora - inicioAlarme >= DURACAO_ALARME) {

    noTone(BUZZER_PIN);

    alarmeAtivo = false;
    estadoBip = false;

    return;
  }

  // Controle dos bips
  if (agora - ultimoBip >= INTERVALO_BIP) {

    ultimoBip = agora;

    estadoBip = !estadoBip;

    if (estadoBip) {

      tone(BUZZER_PIN, 1800);

    } else {

      noTone(BUZZER_PIN);

    }
  }
}

// =================================================
// ENVIO DOS DADOS PARA O BLYNK
// =================================================

void enviarDados() {

  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  // Verifica se o DHT11 respondeu
  if (isnan(umidade) || isnan(temperatura)) {

    Serial.println("Erro ao ler o DHT11");

    return;
  }

  // Mede distância
  float distancia = medirDistancia();

  // =================================================
  // SERIAL MONITOR
  // =================================================

  Serial.println("-----------------------------");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println(" %");

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.println("-----------------------------");

  // =================================================
  // ENVIA DADOS PARA O BLYNK
  // =================================================

  Blynk.virtualWrite(V2, temperatura);
  Blynk.virtualWrite(V3, umidade);
  Blynk.virtualWrite(V6, distancia);

  // =================================================
  // ALERTA DE TEMPERATURA
  // =================================================

  if (temperatura >= TEMPERATURA_ALTA) {

    if (!alertaTemperaturaEnviado) {

      Serial.println("TEMPERATURA ALTA!");

      Blynk.logEvent(
        "temperatura_alta",
        String("Temperatura muito alta: ") +
        String(temperatura) +
        " °C"
      );

      alertaTemperaturaEnviado = true;
    }

  } else {

    alertaTemperaturaEnviado = false;
  }

  // =================================================
  // DETECÇÃO DA PORTA
  // =================================================

  portaAberta = (distancia > DISTANCIA_PORTA_ABERTA);

  // =================================================
  // PORTA ABERTA
  // =================================================

  if (portaAberta) {

    Serial.println("PORTA DO SERVIDOR ABERTA!");

    // LED permanece ligado
    digitalWrite(LED_PIN, HIGH);

    // Envia status para o Blynk
    Blynk.virtualWrite(V7, "PORTA ABERTA");

    // Detecta uma nova abertura
    if (!portaEstavaAberta) {

      alarmeAtivo = true;

      inicioAlarme = millis();

      ultimoBip = millis();

      estadoBip = false;
    }

    portaEstavaAberta = true;
  }

  // =================================================
  // PORTA FECHADA
  // =================================================

  else {

    Serial.println("Porta fechada.");

    // Desliga LED
    digitalWrite(LED_PIN, LOW);

    // Desliga buzzer
    noTone(BUZZER_PIN);

    // Desativa alarme
    alarmeAtivo = false;

    estadoBip = false;

    // Envia status para o Blynk
    Blynk.virtualWrite(V7, "PORTA FECHADA");

    // Sistema pronto para nova abertura
    portaEstavaAberta = false;
  }
}

// =================================================
// CONTROLE MANUAL DO LED PELO BLYNK
// =================================================

BLYNK_WRITE(V4) {

  int estadoLED = param.asInt();

  digitalWrite(LED_PIN, estadoLED);
}

// =================================================
// CONTROLE MANUAL DO BUZZER PELO BLYNK
// =================================================

BLYNK_WRITE(V5) {

  int estadoBuzzer = param.asInt();

  if (estadoBuzzer == 1) {

    tone(BUZZER_PIN, 1000);

  } else {

    noTone(BUZZER_PIN);
  }
}

// =================================================
// SETUP
// =================================================

void setup() {

  Serial.begin(115200);

  // Configuração dos pinos
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // =================================================
  // TESTE INICIAL DO LED E BUZZER
  // =================================================

  digitalWrite(LED_PIN, HIGH);

  tone(BUZZER_PIN, 1000);

  delay(2000);

  digitalWrite(LED_PIN, LOW);

  noTone(BUZZER_PIN);

  // =================================================
  // INICIA DHT11
  // =================================================

  dht.begin();

  // =================================================
  // CONECTA AO BLYNK
  // =================================================

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  // =================================================
  // ATUALIZA OS DADOS A CADA 2 SEGUNDOS
  // =================================================

  timer.setInterval(2000L, enviarDados);

  Serial.println("Sistema iniciado!");
}

// =================================================
// LOOP
// =================================================

void loop() {

  Blynk.run();

  timer.run();

  controlarAlarme();
}
