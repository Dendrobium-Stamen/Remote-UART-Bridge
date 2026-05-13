idf_component_register(SRC_DIRS "."
                                "./handler"
    INCLUDE_DIRS "."
                 "./include"
                 "./handler"
    EMBED_FILES "./html/espnow_html.html"
                "./html/monitor_html.html"
    REQUIRES esp_http_server message-manager espressif__cjson)
