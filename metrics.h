#ifndef _METRICS_H_
#define _METRICS_H_

#include <WiFi.h>

class Metric
{
public:
    Metric();
    Metric(const char *name, int val);
    Metric(const char *name, float val);
    void metricWrite(char *out);

private:
    char _name[50];
    float _floatVal;
    int _intVal;
    int _isInt;
};

class Metrics
{
public:
    Metrics();
    void add(Metric metric);
    void add(const char *name, int value);
    void add(const char *name, float value);
    int size();
    Metric get(int index);

private:
    int _size;
    Metric _metrics[50];
};

class PrometheusMetricPush
{
public:
    PrometheusMetricPush(const char *host, int port, const char *deviceID);
    PrometheusMetricPush(IPAddress hostIP, int port, const char *deviceID);
    int push(Metrics metrics);

private:
    char _host[50];
    IPAddress _hostIP;
    int _port;
    char _deviceID[50];
};

#endif