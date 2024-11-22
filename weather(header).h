#ifndef WEATHER_H
#define WEATHER_H

typedef struct {
    char city[100];
    char country[100];
    double temperature;
    int humidity;
} WeatherData;

int get_weather_data(const char *url, char *response);
int parse_json(const char *json_data, WeatherData *data);
void write_to_file(const char *filename, const WeatherData *data);
void check_alerts(const WeatherData *data);

#endif // WEATHER_H
