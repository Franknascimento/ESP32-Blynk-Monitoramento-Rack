#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ===============================
// CONFIGURAÇÃO WI-FI
// ===============================

char ssid[] = "SUA_REDE_WIFI";
char pass[] = "SUA_SENHA_WIFI";

// ===============================
// CONFIGURAÇÃO BLYNK
// ===============================

#define BLYNK_TEMPLATE_ID "SEU_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "ESP32 control"
#define BLYNK_AUTH_TOKEN "SEU_BLYNK_AUTH_TOKEN"

// ===============================
// SENSOR DHT11
// ===============================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ===============================
// SENSOR HC-SR04
// ===============================

#define TRIG_PIN 5
#define ECHO_PIN 18

// ===============================
// LED E BUZZER
// ===============================

#define LED_PIN 26
#define BUZZER_PIN 25

// ===============================
// LIMITES DO SISTEMA
// ===============================

#define TEMPERATURA_ALTA 35.0
#define DISTANCIA_PORTA_ABERTA 60

// ===============================
// CONTROLE DO ALERTA DE TEMPERATURA
// ===============================

bool alertaTemperaturaEnviado = false;

// ===============================
// CONTROLE DA PORTA
// ===============================

bool portaAberta = false;
bool portaEstavaAberta = false;

// ===============================
// CONTROLE DO ALARME
// ===============================

bool alarmeAtivo = false;

unsigned long inicioAlarme = 0;
const unsigned long DURACAO_ALARME = 10000;

unsigned long ultimoBip = 0;
bool estadoBip = false;

const unsigned long INTERVALO_BIP = 300;

// ===============================
// TIMER BLYNK
// ===============================

BlynkTimer timer;

// ===============================
// MEDIR DISTÂNCIA
// ===============================

float medirDistancia() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracao == 0)
    return -1;

  return duracao * 0.034 / 2;
}

// ===============================
// CONTROLE DO ALARME
// ===============================

void controlarAlarme() {

  if (!alarmeAtivo)
    return;

  unsigned long agora = millis();

  if (agora - inicioAlarme >= DURACAO_ALARME) {

    noTone(BUZZER_PIN);

    alarmeAtivo = false;
    estadoBip = false;

    return;
  }

  if (agora - ultimoBip >= INTERVALO_BIP) {

    ultimoBip = agora;

    estadoBip = !estadoBip;

    if (estadoBip)
      tone(BUZZER_PIN, 1800);
    else
      noTone(BUZZER_PIN);
  }
}

// ===============================
// ENVIO DOS DADOS PARA O BLYNK
// ===============================

void enviarDados() {

  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if (isnan(umidade) || isnan(temperatura)) {

    Serial.println("Erro ao ler o DHT11");

    return;
  }

  float distancia = medirDistancia();

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

  // Envia dados para o Blynk

  Blynk.virtualWrite(V2, temperatura);
  Blynk.virtualWrite(V3, umidade);
  Blynk.virtualWrite(V6, distancia);

  // ===============================
  // ALERTA DE TEMPERATURA
  // ===============================

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

  // ===============================
  // DETECÇÃO DA PORTA
  // ===============================

  portaAberta = (distancia > DISTANCIA_PORTA_ABERTA);

  if (portaAberta) {

    Serial.println("PORTA DO SERVIDOR ABERTA!");

    digitalWrite(LED_PIN, HIGH);

    Blynk.virtualWrite(V7, "PORTA ABERTA");

    // Detecta uma nova abertura

    if (!portaEstavaAberta) {

      alarmeAtivo = true;

      inicioAlarme = millis();

      ultimoBip = millis();

      estadoBip = false;
    }

    portaEstavaAberta = true;

  } else {

    Serial.println("Porta fechada.");

    digitalWrite(LED_PIN, LOW);

    noTone(BUZZER_PIN);

    alarmeAtivo = false;

    estadoBip = false;

    portaEstavaAberta = false;

    Blynk.virtualWrite(V7, "PORTA FECHADA");
  }
}

// ===============================
// CONTROLE MANUAL DO LED
// ===============================

BLYNK_WRITE(V4) {

  int estadoLED = param.asInt();

  digitalWrite(LED_PIN, estadoLED);
}

// ===============================
// CONTROLE MANUAL DO BUZZER
// ===============================

BLYNK_WRITE(V5) {

  int estadoBuzzer = param.asInt();

  if (estadoBuzzer == 1)
    tone(BUZZER_PIN, 1000);
  else
    noTone(BUZZER_PIN);
}

// ===============================
// CONFIGURAÇÃO INICIAL
// ===============================

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Teste inicial do LED e buzzer

  digitalWrite(LED_PIN, HIGH);

  tone(BUZZER_PIN, 1000);

  delay(2000);

  digitalWrite(LED_PIN, LOW);

  noTone(BUZZER_PIN);

  // Inicializa o DHT11

  dht.begin();

  // Conecta ao Blynk

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  // Atualização dos dados a cada 2 segundos

  timer.setInterval(2000L, enviarDados);

  Serial.println("Sistema iniciado!");
}

// ===============================
// LOOP PRINCIPAL
// ===============================

void loop() {

  Blynk.run();

  timer.run();

  controlarAlarme();
}
