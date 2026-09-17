# ESP32-Blynk-Monitoramento-Rack

Sistema IoT para monitoramento de rack/ambiente de servidor, desenvolvido com **ESP32**, **DHT11**, **HC-SR04** e **Blynk IoT**.

## 📌 Objetivo

Monitorar temperatura, umidade e abertura da porta do rack/servidor. O sistema envia os dados para o Blynk e utiliza LED e buzzer como sinalização local.

## ⚙️ Funcionamento

- 🌡️ **DHT11:** mede temperatura e umidade.
- 📏 **HC-SR04:** mede a distância e identifica a abertura da porta.
- 💡 **LED:** permanece aceso enquanto a porta está aberta.
- 🔊 **Buzzer:** emite bipes intermitentes durante 10 segundos quando uma nova abertura é detectada.
- 📱 **Blynk IoT:** exibe temperatura, umidade, distância e status da porta.
- 🔔 **Alerta:** envia uma notificação quando a temperatura atinge **35 °C ou mais**.

## 🔌 Pinagem

| Componente | ESP32 |
|---|---:|
| DHT11 DATA | GPIO 4 |
| HC-SR04 TRIG | GPIO 5 |
| HC-SR04 ECHO | GPIO 18 |
| LED | GPIO 26 |
| Buzzer | GPIO 25 |

## 📊 Datastreams do Blynk

| Virtual Pin | Função |
|---|---|
| V2 | Temperatura |
| V3 | Umidade |
| V4 | Controle do LED |
| V5 | Controle do buzzer |
| V6 | Distância |
| V7 | Status da porta |

## 🧪 Desenvolvimento

O projeto foi inicialmente prototipado no **Wokwi** e posteriormente implementado em um **ESP32 físico** utilizando a **Arduino IDE**.

A comunicação e o monitoramento remoto foram realizados utilizando o **Blynk IoT**.

## 🚨 Lógica do sistema

Quando a distância medida pelo HC-SR04 ultrapassa o limite configurado:

1. O sistema identifica a porta como aberta.
2. O LED é acionado.
3. O buzzer emite bipes intermitentes durante 10 segundos.
4. Após os 10 segundos, o buzzer é desligado.
5. O LED permanece ligado enquanto a porta continuar aberta.
6. Quando a porta é fechada, o LED é desligado e o sistema fica pronto para detectar uma nova abertura.

## 🌡️ Monitoramento de temperatura

O sistema utiliza o DHT11 para realizar a leitura da temperatura.

Quando a temperatura atinge **35 °C ou mais**, o ESP32 envia um evento para o Blynk, que pode gerar uma notificação no dispositivo cadastrado.

## 🔐 Configuração

Antes de gravar o código no ESP32, configure sua rede Wi-Fi e as credenciais do Blynk.

**Não publique senhas ou tokens reais no GitHub.**

Exemplo:

```cpp
char ssid[] = "SUA_REDE_WIFI";
char pass[] = "SUA_SENHA_WIFI";

#define BLYNK_TEMPLATE_ID "SEU_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "ESP32 control"
#define BLYNK_AUTH_TOKEN "SEU_BLYNK_AUTH_TOKEN"
## 📁 Estrutura do repositório

- 📂 **[codigo](codigo/)** — código-fonte principal do ESP32.
- 📂 **[simulacao](simulacao/)** — arquivos da simulação no Wokwi, incluindo código, circuito e documentação.
- 📄 **[README da simulação](simulacao/README.md)** — descrição da montagem e funcionamento da simulação.

## 🔗 Acesso rápido

- [Código do ESP32](codigo/ESP32_Rack_Blynk.ino)
- [Simulação no Wokwi](simulacao/)
- [README da simulação](simulacao/README.md)

## 👨‍💻 Autor

**Frank Nascimento**

Projeto acadêmico de IoT para monitoramento de ambiente/rack de servidor.
