# Simulação no Wokwi

A simulação do sistema foi desenvolvida utilizando a plataforma Wokwi.

## Componentes utilizados

- ESP32
- DHT11
- HC-SR04
- LED
- Buzzer

## Funcionamento

O ESP32 realiza a leitura da temperatura e umidade através do DHT11 e utiliza o sensor ultrassônico HC-SR04 para identificar a abertura da porta do rack/ambiente de servidor.

Quando a porta é aberta, o LED permanece aceso e o buzzer emite bipes intermitentes durante 10 segundos.

Os dados de temperatura, umidade, distância e status da porta são enviados para o Blynk IoT.

Em caso de temperatura igual ou superior a 35 °C, o sistema envia uma notificação de alerta.

## Plataforma

Wokwi – ambiente utilizado para prototipagem e testes do circuito antes da implementação física.
