#ifndef espnow_manager_h
#define espnow_manager_h

#define ESPNOW_MANAGER_MAC_LEN 6
#define ESPNOW_MANAGER_MAX_LABEL_LENGTH 32
#define ESPNOW_MANAGER_MAX_PEER_DEVICES 10
#define ESPNOW_MANAGER_MAX_RECEIVE_MESSAGE_DATA_LENGTH 1044

typedef struct
{
    uint8_t src_mac[ESPNOW_MANAGER_MAC_LEN];
    uint8_t dest_mac[ESPNOW_MANAGER_MAC_LEN];
    int rssi;
    uint8_t data[ESPNOW_MANAGER_MAX_RECEIVE_MESSAGE_DATA_LENGTH];
    size_t data_length;
} espnow_manager_message_t;

typedef void espnow_manager_receive_message_callback_t(espnow_manager_message_t *message);

typedef enum
{
    ESPNOW_MANAGER_OK = 0,
    ESPNOW_MANAGER_ERROR_INIT,
    ESPNOW_MANAGER_ERROR_DEINIT,
    ESPNOW_MANAGER_ERROR_NOT_INIT,
    ESPNOW_MANAGER_ERROR_LABEL_GET,
    ESPNOW_MANAGER_ERROR_LABEL_SET,
    ESPNOW_MANAGER_ERROR_PEER_EXISTS,
    ESPNOW_MANAGER_ERROR_PEER_NOT_FOUND,
    ESPNOW_MANAGER_ERROR_PEER_FULL,
    ESPNOW_MANAGER_ERROR_PEER_ADD,
    ESPNOW_MANAGER_ERROR_PEER_COUNT_FIND,
    ESPNOW_MANAGER_ERROR_PEER_GET,
    ESPNOW_MANAGER_ERROR_PEER_DEL,
    ESPNOW_MANAGER_ERROR_ENABLE_MAC,
    ESPNOW_MANAGER_ERROR_DISABLE_MAC,
    ESPNOW_MANAGER_ERROR_SEND,
    ESPNOW_MANAGER_ERROR_RECV,
    ESPNOW_MANAGER_ERROR_SCAN,
    ESPNOW_MANAGER_ERROR_NVS,
    ESPNOW_MANAGER_ERROR_INVALID_ARG,
    ESPNOW_MANAGER_ERROR_LOCK,
    ESPNOW_MANAGER_ERROR_TIMEOUT,
} espnow_manager_error_t;

typedef struct
{
    char *label;
    espnow_manager_receive_message_callback_t *receive_message_callback;
} espnow_manager_config_t;

espnow_manager_error_t espnow_manager_init(espnow_manager_config_t *config);
espnow_manager_error_t espnow_manager_deinit();

espnow_manager_error_t espnow_manager_task_start();
espnow_manager_error_t espnow_manager_task_stop();

espnow_manager_error_t espnow_manager_get_mac(uint8_t *mac);
espnow_manager_error_t espnow_manager_get_label(char *label, uint8_t label_length);
espnow_manager_error_t espnow_manager_set_label(char *label, uint8_t label_length);

espnow_manager_error_t espnow_manager_add_peer_mac(uint8_t *mac, char *label);
espnow_manager_error_t espnow_manager_del_peer_mac(uint8_t *mac);
espnow_manager_error_t espnow_manager_get_peer_mac_count(uint8_t *count, size_t *max_label_length);
espnow_manager_error_t espnow_manager_get_peer_mac(uint8_t index, uint8_t *mac, char *label, bool *is_enable);

espnow_manager_error_t espnow_manager_temporary_add_peer_mac(uint8_t *mac);
espnow_manager_error_t espnow_manager_temporary_del_peer_mac(uint8_t *mac);

espnow_manager_error_t espnow_manager_enable_mac(uint8_t *mac);
espnow_manager_error_t espnow_manager_disable_mac(uint8_t *mac);

espnow_manager_error_t espnow_manager_send_to_mac(uint8_t *mac, uint8_t *data, size_t data_length);
espnow_manager_error_t espnow_manager_send_to_enable_mac(uint8_t *data, size_t data_length);

#endif
