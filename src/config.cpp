#include "config.h"

bool loadConfig(int* conf_b, int* conf_r, int* conf_w, int* conf_f) {
  File configFile = SPIFFS.open("/config.json", "r");

  if (!configFile) {
    Serial.println("Failed to open config file");
    defaultConfig();
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  *conf_b = atoi(json["conf_b"]);
  *conf_r = atoi(json["conf_r"]);
  *conf_w = atoi(json["conf_w"]);
  *conf_f = atoi(json["conf_f"]);

  return true;
}

bool saveConfig(int conf_b, int conf_r, int conf_w, int conf_f) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  char conf_b_str[4];
  sprintf(conf_b_str, "%d", conf_b);
  char conf_r_str[4];
  sprintf(conf_r_str, "%d", conf_r);
  char conf_w_str[4];
  sprintf(conf_w_str, "%d", conf_w);
  char conf_f_str[4];
  sprintf(conf_f_str, "%d", conf_f);
  json["conf_b"] = conf_b_str;
  json["conf_r"] = conf_r_str;
  json["conf_w"] = conf_w_str;
  json["conf_f"] = conf_f_str;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}

bool defaultConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["conf_b"] = "100";
  json["conf_r"] = "0";
  json["conf_w"] = "0";
  json["conf_f"] = "7";

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}
