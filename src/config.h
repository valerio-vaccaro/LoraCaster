#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <SPIFFS.h>

#define FORMAT_SPIFFS_IF_FAILED true

bool defaultConfig();
bool loadConfig(int* conf_b, int* conf_r, int* conf_w, int* conf_f);
bool saveConfig(int conf_b, int conf_r, int conf_w, int conf_f);

#endif /* CONFIG_H */
