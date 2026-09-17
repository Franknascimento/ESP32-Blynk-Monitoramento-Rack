

#define BLYNK_TEMPLATE_ID "TMPL2QEnuxnkl"
#define BLYNK_TEMPLATE_NAME "ESP32 control"
#define BLYNK_AUTH_TOKEN "iQM8nNSyyzmT1zm_jEEbD3lIK0AOWDmr"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// =====================================================
// CONFIGURAÇÃO DO WIFI - WOKWI
// =====================================================

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// =====================================================
// SENSOR DHT11
// =====================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =====================================================
// SENSOR ULTRASSÔNICO HC-SR04
// =====================================================

#define TRIG_PIN 5
#define ECHO_PIN 18

// =====================================================
// CONFIGURAÇÕES DE ALERTA
// =====================================================

#define TEMPERATURA_ALTA 35.0

#define DISTANCIA_PORTA_ABERTA 60

bool alertaTemperaturaEnviado = false;

// =====================================================
// LED
// =====================================================

#define LED_PIN 26

// =====================================================
// BUZZER
// =====================================================

#define BUZZER_PIN 25

// =====================================================
// CONTROLE DA PORTA
// =====================================================

bool portaAberta = false;
bool portaEstavaAberta = false;

bool alarmeAtivo = false;

unsigned long inicioAlarme = 0;

const unsigned long DURACAO_ALARME = 10000;

// =====================================================
// CONTROLE DO BIP
// =====================================================

unsigned long ultimoBip = 0;

bool estadoBip = false;

const unsigned long INTERVALO_BIP = 300;

// =====================================================
// TIMER DO BLYNK
// =====================================================

BlynkTimer timer;

// =====================================================
// MEDIR DISTÂNCIA
// =====================================================

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

// =====================================================
// CONTROLE DO ALARME
// =====================================================

void controlarAlarme() {

  if (!alarmeAtivo) {
    return;
  }

  unsigned long agora = millis();

  // Após 10 segundos, desliga o alarme
  if (agora - inicioAlarme >= DURACAO_ALARME) {

    noTone(BUZZER_PIN);

    alarmeAtivo = false;

    estadoBip = false;

    return;
  }

  // Faz o buzzer piscar/intercalar
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

// =====================================================
// ENVIAR DADOS PARA O BLYNK
// =====================================================

void enviarDados() {

  float umidade = dht.readHumidity();

  float temperatura = dht.readTemperature();

  // Verifica erro do DHT11
  if (isnan(umidade) || isnan(temperatura)) {

    Serial.println("Erro ao ler o DHT11");

    return;
  }

  // Mede distância
  float distancia = medirDistancia();

  // ===================================================
  // SERIAL MONITOR
  // ===================================================

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

  // ===================================================
  // ENVIA DADOS PARA O BLYNK
  // ===================================================

  Blynk.virtualWrite(V2, temperatura);

  Blynk.virtualWrite(V3, umidade);

  Blynk.virtualWrite(V6, distancia);

  // ===================================================
  // ALERTA DE TEMPERATURA
  // ===================================================

  if (temperatura >= TEMPERATURA_ALTA) {

    if (!alertaTemperaturaEnviado) {

      Serial.println("⚠️ TEMPERATURA ALTA!");

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

  // ===================================================
  // DETECÇÃO DA PORTA
  // ===================================================

  portaAberta = (distancia > DISTANCIA_PORTA_ABERTA);

  // ===================================================
  // PORTA ABERTA
  // ===================================================

  if (portaAberta) {

    // Envia status para V7
    Blynk.virtualWrite(V7, "PORTA ABERTA");

    Serial.println("🚪 PORTA DO SERVIDOR ABERTA!");

    // Acende LED
    digitalWrite(LED_PIN, HIGH);

    // Se acabou de abrir, inicia o alarme
    if (!portaEstavaAberta) {

      alarmeAtivo = true;

      inicioAlarme = millis();

      ultimoBip = millis();

      estadoBip = false;
    }

    portaEstavaAberta = true;
  }

  // ===================================================
  // PORTA FECHADA
  // ===================================================

  else {

    // Envia status para V7
    Blynk.virtualWrite(V7, "PORTA FECHADA");

    Serial.println("Porta fechada.");

    // Apaga LED
    digitalWrite(LED_PIN, LOW);

    // Desliga buzzer
    noTone(BUZZER_PIN);

    // Reseta alarme
    alarmeAtivo = false;

    estadoBip = false;

    // Rearma o sistema
    portaEstavaAberta = false;
  }
}

// =====================================================
// CONTROLE DO LED PELO BLYNK
// =====================================================

BLYNK_WRITE(V4) {

  int estadoLED = param.asInt();

  digitalWrite(LED_PIN, estadoLED);
}

// =====================================================
// CONTROLE DO BUZZER PELO BLYNK
// =====================================================

BLYNK_WRITE(V5) {

  int estadoBuzzer = param.asInt();

  if (estadoBuzzer == 1) {

    tone(BUZZER_PIN, 1000);

  } else {

    noTone(BUZZER_PIN);
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // Configura pinos
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

  // Inicializa DHT
  dht.begin();

  // Conecta ao Blynk
  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  // Executa enviarDados a cada 2 segundos
  timer.setInterval(2000L, enviarDados);

  Serial.println("Sistema iniciado!");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  Blynk.run();

  timer.run();

  controlarAlarme();
}