#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "weather.h"

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((char *)userp)[size * nmemb] = '\0'; // Null-terminate the received string
    strcat((char *)userp, contents);
    return size * nmemb;
}

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
        return 1;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    // Print received data for debugging
    printf("Received Data: %s\n", response);
    return 0;
}

int parse_json(const char *json_data, EnvironmentalData *data) {
    struct json_object *parsed_json;
    struct json_object *location;
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

    location = json_object_object_get(parsed_json, "location");
    current = json_object_object_get(parsed_json, "current");

    city = json_object_object_get(location, "name");
    country = json_object_object_get(location, "country");
    temp = json_object_object_get(current, "temperature");
    humid = json_object_object_get(current, "humidity");

    strcpy(data->city, json_object_get_string(city));
    strcpy(data->country, json_object_get_string(country));
    data->temperature = json_object_get_double(temp);
    data->humidity = json_object_get_int(humid);

    // Print parsed data for debugging
    printf("City: %s\n", data->city);
    printf("Country: %s\n", data->country);
    printf("Temperature: %.2f°C\n", data->temperature);
    printf("Humidity: %d%%\n", data->humidity);

    return 0;
}

void write_to_file(const char *filename, const EnvironmentalData *data) {
    FILE *file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "City: %s, Country: %s, Temperature: %.2f°C, Humidity: %d%%\n",
                data->city, data->country, data->temperature, data->humidity);
        fclose(file);
    } else {
        perror("Error opening file");
    }
}

void check_alerts(const EnvironmentalData *data) {
    if (data->temperature > 30.0) {
        printf("Alert: High temperature in %s!\n", data->city);
    }
    if (data->humidity < 20) {
        printf("Alert: Low humidity in %s!\n", data->city);
    }
}

int main() {
    char response[2048] = "";  // Buffer to store API response
    EnvironmentalData data;

    // API URL with your WeatherStack API key, specify the country for unambiguous city
    const char *url = "http://api.weatherstack.com/current?access_key=1545f58ef63ff2b6020ccf135fe6af19&query=sydney";

    if (get_weather_data(url, response) == 0) {
        if (parse_json(response, &data) == 0) {
            write_to_file("environmental_data.txt", &data);
            check_alerts(&data);
        } else {
            fprintf(stderr, "Error parsing JSON data\n");
        }
    } else {
        fprintf(stderr, "Error fetching data from API\n");
    }

    // Wait for user input before closing
    printf("Press any key to exit...\n");
    getchar();

    return 0;
}
