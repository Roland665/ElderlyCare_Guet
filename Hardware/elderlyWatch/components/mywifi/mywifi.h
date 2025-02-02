#pragma once

#include "esp_err.h"

#define DEFAULT_WIFI_SSID "HUAWEI Nova 11 SE"
#define DEFAULT_WIFI_PSWD "roland66"

extern char *wifi_ssid;
extern char *wifi_pswd;

void wifi_init(void);
// esp_err_t wifi_connect(void);
void wifi_shutdown(void);
