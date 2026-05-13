idf_component_register(SRC_DIRS "."
                                "./service/"
                                "./handler/"
                    INCLUDE_DIRS "."
                                 "./include"
                                 "./service/"
                                 "./handler/"
                    REQUIRES esp_wifi lwpkt esp_timer espnow-manager)
