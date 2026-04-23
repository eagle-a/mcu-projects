#include "almanac.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "ALMANAC";
static char s_last_error[128] = "";

typedef struct {
    char *buf;
    int len;
    int cap;
} http_resp_buf_t;

static void set_last_error(const char *msg)
{
    snprintf(s_last_error, sizeof(s_last_error), "%s", msg ? msg : "unknown");
}

const char *almanac_last_error(void)
{
    return s_last_error;
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    http_resp_buf_t *resp = (http_resp_buf_t *)evt->user_data;
    if (!resp) {
        return ESP_OK;
    }

    if (evt->event_id == HTTP_EVENT_ON_DATA && evt->data && evt->data_len > 0) {
        int need = resp->len + evt->data_len + 1;
        if (need > resp->cap) {
            int new_cap = resp->cap == 0 ? 1024 : resp->cap * 2;
            while (new_cap < need) {
                new_cap *= 2;
            }
            char *new_buf = (char *)realloc(resp->buf, new_cap);
            if (!new_buf) {
                set_last_error("oom while receiving http body");
                return ESP_FAIL;
            }
            resp->buf = new_buf;
            resp->cap = new_cap;
        }
        memcpy(resp->buf + resp->len, evt->data, evt->data_len);
        resp->len += evt->data_len;
        resp->buf[resp->len] = '\0';
    }

    return ESP_OK;
}

static const char *skip_ws(const char *p)
{
    while (p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
        p++;
    }
    return p;
}

static const char *find_key(const char *json, const char *key)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    return strstr(json, pattern);
}

static int extract_json_string(const char *json, const char *key, char *dst, size_t dst_size)
{
    const char *k = find_key(json, key);
    if (!k) {
        dst[0] = '\0';
        return -1;
    }

    const char *colon = strchr(k, ':');
    if (!colon) {
        dst[0] = '\0';
        return -1;
    }
    const char *p = skip_ws(colon + 1);
    if (!p || *p != '"') {
        dst[0] = '\0';
        return -1;
    }
    p++;

    size_t n = 0;
    while (*p && *p != '"') {
        if (*p == '\\' && p[1] != '\0') {
            p++;
        }
        if (n + 1 < dst_size) {
            dst[n++] = *p;
        }
        p++;
    }
    dst[n] = '\0';
    return n > 0 ? 0 : -1;
}

static void extract_json_array_joined(const char *json, const char *key, char *dst, size_t dst_size, int max_items)
{
    const char *k = find_key(json, key);
    dst[0] = '\0';
    if (!k) {
        return;
    }

    const char *colon = strchr(k, ':');
    if (!colon) {
        return;
    }
    const char *p = skip_ws(colon + 1);
    if (!p || *p != '[') {
        return;
    }
    p++;

    int count = 0;
    while (*p && *p != ']' && count < max_items) {
        p = skip_ws(p);
        if (*p != '"') {
            p++;
            continue;
        }
        p++;

        char item[64] = {0};
        size_t n = 0;
        while (*p && *p != '"') {
            if (*p == '\\' && p[1] != '\0') {
                p++;
            }
            if (n + 1 < sizeof(item)) {
                item[n++] = *p;
            }
            p++;
        }
        item[n] = '\0';

        if (item[0] != '\0') {
            if (dst[0] != '\0') {
                strncat(dst, "、", dst_size - strlen(dst) - 1);
            }
            strncat(dst, item, dst_size - strlen(dst) - 1);
        }

        if (*p == '"') {
            p++;
        }
        count++;
    }
}

bool almanac_fetch(almanac_data_t *out)
{
    if (!out) {
        set_last_error("invalid output ptr");
        return false;
    }

    memset(out, 0, sizeof(*out));
    set_last_error("");

    http_resp_buf_t resp = {0};
    esp_http_client_config_t cfg = {
        .url = ALMANAC_API_URL,
        .timeout_ms = 10000,
        .event_handler = http_event_handler,
        .user_data = &resp,
    };

    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        set_last_error("http client init failed");
        return false;
    }

    esp_http_client_set_header(client, "User-Agent", "ESP32-Calendar/1.0");
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        set_last_error("http perform failed");
        ESP_LOGE(TAG, "http perform error: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        free(resp.buf);
        return false;
    }

    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    if (status != 200 || !resp.buf || resp.len == 0) {
        set_last_error("http status/body invalid");
        free(resp.buf);
        return false;
    }

    extract_json_string(resp.buf, "日期", out->date, sizeof(out->date));
    extract_json_string(resp.buf, "农历", out->lunar, sizeof(out->lunar));
    extract_json_string(resp.buf, "星期", out->weekday, sizeof(out->weekday));
    extract_json_string(resp.buf, "今日节气", out->jieqi, sizeof(out->jieqi));
    extract_json_string(resp.buf, "季节", out->season, sizeof(out->season));
    extract_json_string(resp.buf, "生肖冲煞", out->chongsha, sizeof(out->chongsha));
    extract_json_string(resp.buf, "星座", out->constellation, sizeof(out->constellation));
    extract_json_array_joined(resp.buf, "宜", out->suitable, sizeof(out->suitable), 12);
    extract_json_array_joined(resp.buf, "忌", out->avoid, sizeof(out->avoid), 12);

    free(resp.buf);

    out->is_valid = out->date[0] != '\0';
    if (!out->is_valid) {
        set_last_error("missing date field");
        return false;
    }

    ESP_LOGI(TAG, "almanac fetched: %s %s", out->date, out->weekday);
    return true;
}
