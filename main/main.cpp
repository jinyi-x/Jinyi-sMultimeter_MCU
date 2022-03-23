#include <nvs_flash.h>
#include "Network.hpp"

extern "C" void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
}
