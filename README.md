# Projeto de Controle de LED com MQTT - Sistemas Embarcados

**Aluno:** JUAN VICTOR SOUZA SILVA

## Descrição do Projeto

Este é o projeto da Unidade 1 da matéria de Sistemas Embarcados.

O objetivo foi usar uma placa ESP32 para controlar o LED da placa através do protocolo MQTT. O código foi feito com base no exemplo `mqtt5` do ESP-IDF.

Como funciona:

1.  A placa ESP32 conecta no Wi-Fi e em um servidor (Broker) MQTT.
2.  Ela fica "escutando" o tópico `/ifpe/ads/embarcados/esp32/led`.
3.  Quando uma mensagem com o valor `1` é enviada para esse tópico, o LED acende.
4.  Quando a mensagem é `0`, o LED apaga.

## Como Executar o Projeto

Para fazer o projeto funcionar, siga os passos abaixo. É necessário já ter o ambiente ESP-IDF configurado na sua máquina.

### 1. Clone o Repositório

Abra um terminal e clone este repositório para o seu computador.

```bash
git clone https://github.com/JuanVictorss/Esp32-Led.git
cd Esp32-Led
```
### COM O AMBIENTE MONTADO, RECOMENDO QUE O PROJETO ESTEJA NA PASTA "Espressif"
### 2. Configure o Wi-Fi e o Broker

Use o menu de configuração do ESP-IDF para colocar os dados da sua rede e do seu broker.

```bash
Aperte F1 e pesquise >ESP-IDF: SDK Configuration Editor (Menuconfi)
```

Dentro do menu que abrir, navegue e altere:

- **Para configurar o Wi-Fi:**

  - Vá em `Example Connection Configuration --->`
  - Preencha o `Wi-Fi SSID` (nome da sua rede) e o `Wi-Fi Password` (senha da sua rede).

- **Para configurar o Broker MQTT:**

  - Vá em `Example Configuration --->`
  - Preencha o `Broker URL`. Exemplo: `broker.hivemq.com`.

  - Vá em `Project Configuration --->`
  - Altere o `LED GPIO Number`. O padrão é `2`.

Depois de configurar, Salve.

### 3. Grave na Placa ESP32

Conecte sua placa no computador para compilar o código e gravar.

```bash
# Lembre-se de trocar a porta pela porta serial correta (exemplo: COM3 no Windows), no vsCode tem uma barra na parte inferior e terá um icone de uma tomada.

# Para Buildar, na mesma barra terá um icone de uma chave de boca.

# E para gravar terá um icone de raio, flash device. Depois escolha a opção UART que aparece na parte superior do vsCode.

```

### 4. Testando o LED

1.  Use um aplicativo MQTT no seu celular ou computador.
2.  Conecte-se no **mesmo broker** que você configurou no passo 2.
3.  Publique (envie) uma mensagem com os seguintes dados:
    - **Tópico:** `/ifpe/ads/embarcados/esp32/led`
    - **Mensagem para Ligar:** `1`
    - **Mensagem para Desligar:** `0`
