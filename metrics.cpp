#include "metrics.h"
#include <WiFi.h>

Metric::Metric()
{
}

Metric::Metric(const char *name, int value)
{
    strcpy(this->_name, name);
    this->_intVal = value;
    this->_isInt = 1;
}

Metric::Metric(const char *name, float value)
{
    strcpy(this->_name, name);
    this->_floatVal = value;
    this->_isInt = 0;
}

void Metric::metricWrite(char *out)
{
    if (this->_isInt)
    {
        sprintf(out, "%s %d\n", this->_name, this->_intVal);
    }
    else
    {
        sprintf(out, "%s %f\n", this->_name, this->_floatVal);
    }
}

Metrics::Metrics()
{
    _size = 0;
}

void Metrics::add(Metric metric)
{
    _metrics[_size++] = metric;
}

void Metrics::add(const char *name, int value)
{
    _metrics[_size++] = Metric(name, value);
}

void Metrics::add(const char *name, float value)
{
    _metrics[_size++] = Metric(name, value);
}

int Metrics::size()
{
    return _size;
}

Metric Metrics::get(int index)
{
    return _metrics[index];
}

PrometheusMetricPush::PrometheusMetricPush(const char *host, int port, const char *deviceID)
{
    strcpy(_host, host);
    strcpy(_deviceID, deviceID);
    _port = port;
}

PrometheusMetricPush::PrometheusMetricPush(IPAddress hostIP, int port, const char *deviceID)
{
    _hostIP = hostIP;
    strcpy(_deviceID, deviceID);
    _port = port;
}

char body[2048];
char metric[256];
int PrometheusMetricPush::push(Metrics metrics)
{
    body[0] = '\0';
    for (int i = 0; i < metrics.size(); i++)
    {
        metrics.get(i).metricWrite(metric);
        strcat(body, metric);
    }
   // Serial.println(body);

    WiFiClient client;
    long start = millis();
    while (millis() - start < 10000)
    {
        if (client.connect(_hostIP, _port))
        {
            client.print("POST /metrics/job/");
            client.print(_deviceID);
            client.print(" HTTP/1.1\r\n");
            client.print("Connection: close\r\n");
            client.print("Content-Type: application/x-www-form-urlencoded\r\n");
            client.print("Content-Length: ");
            client.print(strlen(body));
            client.print("\r\n");
            client.print("Host: ");
            client.print(_host);
            client.print(":");
            client.print(_port);
            client.print("\r\n\r\n");
            client.print(body);
            client.flush();
            client.stop();
            return 1;
        } else {
            Serial.println("Could not connect to Prometheus Gateway!");
        }
    }
    return 0;
}