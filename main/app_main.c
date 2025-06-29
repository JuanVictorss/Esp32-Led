/**
 * @file app_main.c
 * @author JUAN VICTOR SOUZA SILVA
 * @date 29 de Junho de 2025
 * @version 1.0
 * @brief Cliente MQTTv5 para controle de LED no ESP32 - Projeto da Unidade 1.
 *
 * Este projeto implementa um cliente MQTT v5 no microcontrolador ESP32.
 * O dispositivo conecta-se a uma rede Wi-Fi e a um broker MQTT,
 * inscrevendo-se no tópico "/ifpe/ads/embarcados/esp32/led".
 * O estado de um LED (cujo pino é configurável via menuconfig) é
 * controlado com base nas mensagens publicadas neste tópico ('1' para ligar, '0' para desligar).
 *
 * Baseado no exemplo "mqtt5" do framework ESP-IDF.
 */

// Inclusões padrão do sistema e de log
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"

// Inclusões para rede e protocolos
#include "protocol_examples_common.h"
#include "mqtt_client.h"

// Inclusão para controle dos pinos de hardware (GPIO)
#include "driver/gpio.h"

/**
 * @brief TAG utilizada para os logs do sistema, facilitando a depuração.
 */
static const char *TAG = "MQTT5_LED_PROJETO";

/**
 * @brief Tópico MQTT no qual o ESP32 se inscreverá para receber os comandos.
 */
#define MQTT_LED_COMMAND_TOPIC "/ifpe/ads/embarcados/esp32/led"


/**
 * @brief Inicializa o pino do LED.
 *
 * Configura o pino do LED, definido em `CONFIG_BLINK_GPIO` via menuconfig,
 * como uma saída digital e o define com o estado inicial desligado.
 */
static void led_init(void)
{
    // O pino do LED é definido pela variável CONFIG_BLINK_GPIO do menuconfig
    gpio_reset_pin(CONFIG_BLINK_GPIO);
    gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_BLINK_GPIO, 0);
}

/**
 * @brief Manipulador de eventos para o cliente MQTT v5.
 *
 * Esta função é o coração da lógica MQTT. Ela é chamada pela biblioteca
 * sempre que um evento de rede ou de protocolo ocorre.
 *
 * @param handler_args Argumentos do manipulador (não utilizado neste projeto).
 * @param base Base do evento.
 * @param event_id ID do evento específico que ocorreu.
 * @param event_data Ponteiro para os dados associados ao evento.
 */
static void mqtt5_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, (long)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED: Conectado ao broker MQTT!");
        // Após conectar, inscreve-se no tópico de comando do LED com QoS 1.
        esp_mqtt_client_subscribe(client, MQTT_LED_COMMAND_TOPIC, 1);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED: Desconectado do broker MQTT.");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED: Inscrição no tópico '%s' confirmada!", MQTT_LED_COMMAND_TOPIC);
        ESP_LOGI(TAG, "Sistema pronto. Aguardando comandos para o LED...");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA: Mensagem recebida!");
        ESP_LOGI(TAG, "TOPICO: %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "DADO: %.*s", event->data_len, event->data);

        // Verifica se a mensagem foi recebida no tópico correto
        if (strncmp(event->topic, MQTT_LED_COMMAND_TOPIC, event->topic_len) == 0) {
            ESP_LOGI(TAG, "Comando para o LED recebido no tópico correto.");
            
            // Verifica o conteúdo da mensagem (payload) para ligar ou desligar o LED
            if (strncmp(event->data, "1", event->data_len) == 0) {
                ESP_LOGI(TAG, "Comando: '1'. Acendendo o LED.");
                gpio_set_level(CONFIG_BLINK_GPIO, 1);
            } else if (strncmp(event->data, "0", event->data_len) == 0) {
                ESP_LOGI(TAG, "Comando: '0'. Apagando o LED.");
                gpio_set_level(CONFIG_BLINK_GPIO, 0);
            } else {
                ESP_LOGW(TAG, "Comando desconhecido recebido: '%.*s'. Nenhuma ação tomada.", event->data_len, event->data);
            }
        } else {
            ESP_LOGW(TAG, "Mensagem recebida em tópico inesperado. Ignorando.");
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGE(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", (int)event_id);
        break;
    }
}

/**
 * @brief Função principal da aplicação (ponto de entrada).
 *
 * Realiza todas as inicializações necessárias em sequência:
 * 1. Log inicial do sistema.
 * 2. Inicialização do hardware (LED).
 * 3. Inicialização da memória não volátil (NVS).
 * 4. Inicialização da pilha de rede (TCP/IP).
 * 5. Criação do loop de eventos padrão.
 * 6. Conexão com a rede Wi-Fi.
 * 7. Configuração e inicialização do cliente MQTTv5.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "[INÍCIO] Inicializando aplicação...");

    led_init();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    ESP_LOGI(TAG, "Iniciando cliente MQTT v5...");
    esp_mqtt_client_config_t mqtt5_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt5_cfg);

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt5_event_handler, NULL));
    
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}