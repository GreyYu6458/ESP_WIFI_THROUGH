#ifndef _SELF_DETAIL_JSON_H_
#define _SELF_DETAIL_JSON_H_
#include "cJSON.h"
#include "config.h"
#include "esp_log.h"

/* 本机描述JSON */
extern cJSON* device_detail_json;

extern cJSON* json_detail_init();
extern char* get_char_detail();
extern char* get_char_detail_buff(char *buff, int len);

#endif