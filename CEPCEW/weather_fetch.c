#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <syslog.h>
#include "weather(header).h"

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((char *)userp)[size * nmemb] = '\0'; // Null-terminate the received string
    strcat((char *)userp, contents);
    return size * nmemb;
}

// Fetching weather data using CURL
int get_weather_data(const char *url, char *response) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "Error initializing curl\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Curl error: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}

// Parsing JSON data and updating the WeatherData struct
int parse_json(const char *json_data, WeatherData *data) {
    struct json_object *parsed_json;
    struct json_object *sys;
    struct json_object *current;
    struct json_object *temp;
    struct json_object *humid;
    struct json_object *city;
    struct json_object *country;

    parsed_json = json_tokener_parse(json_data);
    if (parsed_json == NULL) {
        fprintf(stderr, "Error parsing JSON\n");
        return 1;
    }

    city = json_object_object_get(parsed_json, "name");
    sys = json_object_object_get(parsed_json, "sys");
    country = json_object_object_get(sys, "country");
    current = json_object_object_get(parsed_json, "main");
    temp = json_object_object_get(current, "temp");
    humid = json_object_object_get(current, "humidity");

    strcpy(data->city, json_object_get_string(city));
    strcpy(data->country, json_object_get_string(country));
    data->temperature = json_object_get_double(temp);
    data->humidity = json_object_get_int(humid);

    return 0;
}

// Writing weather data to a file
void write_to_file(const char *filename, const WeatherData *data) {
    FILE *file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "City: %s, Country: %s, Temperature: %.2f°C, Humidity: %d%%\n",
                data->city, data->country, data->temperature, data->humidity);
        fclose(file);
    } else {
        perror("Error opening file");
    }
}

// Checking alerts, logging to syslog, and sending desktop notifications
void check_alerts(const WeatherData *data) {
    openlog("WeatherAlert", LOG_PID | LOG_CONS, LOG_USER);

    char command[256]; // Buffer to hold the command string

    if (data->temperature > 30.0) {
        syslog(LOG_ALERT, "High temperature alert in %s! Temperature: %.2f°C", data->city, data->temperature);
        snprintf(command, sizeof(command), "notify-send 'Weather Alert' 'High temperature in %s! Temperature: %.2f°C'", data->city, data->temperature);
        system(command);
    }

    if (data->humidity < 20) {
        syslog(LOG_WARNING, "Low humidity alert in %s! Humidity: %d%%", data->city, data->humidity);
        snprintf(command, sizeof(command), "notify-send 'Weather Alert' 'Low humidity in %s! Humidity: %d%%'", data->city, data->humidity);
        system(command);
    }

    closelog();
}

int main() {
    char *response = (char *)malloc(2048 * sizeof(char));
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    WeatherData *data = (WeatherData *)malloc(sizeof(WeatherData));
    if (data == NULL) {
        fprintf(stderr, "Memory allocation for data failed\n");
        free(response);
        return 1;
    }

    const char *url = "http://api.openweathermap.org/data/2.5/weather?q=Karachi,PK&units=metric&appid=833e18e3d3df702326c6f5e1b57b3701";

    if (get_weather_data(url, response) == 0) {
        if (parse_json(response, data) == 0) {
            // Print only the required fields
            printf("Country: %s\n", data->country);
            printf("City: %s\n", data->city);
            printf("Temperature: %.2f°C\n", data->temperature);
            printf("Humidity: %d%%\n", data->humidity);

            write_to_file("weather_data.txt", data);
            check_alerts(data);
        } else {
            fprintf(stderr, "Error parsing JSON data\n");
        }
    } else {
        fprintf(stderr, "Error fetching data from API\n");
    }

    free(response);
    free(data);

    return 0;
}


