#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <vector>
#include <algorithm>

#define MESH_PREFIX "MeshName"
#define MESH_PASSWORD "SuperSecretPassword"
#define MESH_PORT 5555

Scheduler userScheduler;
painlessMesh mesh;

const double TARGET_LAT = 40.785091;
const double TARGET_LON = -73.968285;

std::vector<std::pair<uint32_t, double>> distanceList; // Node ID and distance
std::vector<std::pair<uint32_t, float>> humidityList; // Node ID and humidity

void receivedCallback(uint32_t from, String &msg);
double calculateDistance(double lat1, double lon1, double lat2, double lon2);
float readHumidity();
void broadcastData();
void updateDistanceList(uint32_t nodeId, double distance);
void updateHumidityList(uint32_t nodeId, float humidity);
#include <WiFi.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* host = "DatabaseComputerIP";
const uint16_t port = 65432;

void sendViaWiFi(String data) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    WiFiClient client;
    if (!client.connect(host, port)) {
        Serial.println("Connection to host failed");
        return;
    }
    Serial.println("Connected to server successful");
    
    client.print(data);
    client.stop();
    WiFi.disconnect();
}

// Modify your existing logic to determine the closest sensor
// and call sendViaWiFi() with your data for the closest sensor

Task taskBroadcastData(TASK_SECOND * 10, TASK_FOREVER, &broadcastData);

void setup() {
    Serial.begin(115200);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    userScheduler.addTask(taskBroadcastData);
    taskBroadcastData.enable();
}

void loop() {
  mesh.update();
}

void broadcastData() {
    double distanceToTarget = calculateDistance(0, 0, TARGET_LAT, TARGET_LON); // Example values
    float humidity = readHumidity(); // Implement this function based on your sensor

    StaticJsonDocument<200> doc;
    doc["nodeId"] = mesh.getNodeId();
    doc["distance"] = distanceToTarget;
    doc["humidity"] = humidity;

    String message;
    serializeJson(doc, message);
    mesh.sendBroadcast(message);

    updateDistanceList(mesh.getNodeId(), distanceToTarget);
    updateHumidityList(mesh.getNodeId(), humidity);
}

void receivedCallback(uint32_t from, String &msg) {
    StaticJsonDocument<200> doc;
    auto error = deserializeJson(doc, msg);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    uint32_t nodeId = doc["nodeId"];
    double distance = doc["distance"];
    float humidity = doc["humidity"];

    updateDistanceList(nodeId, distance);
    updateHumidityList(nodeId, humidity);
}

void updateDistanceList(uint32_t nodeId, double distance) {
    // Explicitly declare the iterator type
    std::vector<std::pair<uint32_t, double>>::iterator it = std::find_if(distanceList.begin(), distanceList.end(),
                                                                         [nodeId](const std::pair<uint32_t, double>& pair) { return pair.first == nodeId; });
    if (it != distanceList.end()) {
        it->second = distance;
    } else {
        distanceList.push_back({nodeId, distance});
    }
}

void updateHumidityList(uint32_t nodeId, float humidity) {
    // Explicitly declare the iterator type
    std::vector<std::pair<uint32_t, float>>::iterator it = std::find_if(humidityList.begin(), humidityList.end(),
                                                                        [nodeId](const std::pair<uint32_t, float>& pair) { return pair.first == nodeId; });
    if (it != humidityList.end()) {
        it->second = humidity;
    } else {
        humidityList.push_back({nodeId, humidity});
    }
}


double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    // Dummy implementation of the Haversine formula
    return 1000.0; // Placeholder return value, implement Haversine formula here
}

float readHumidity() {
    // Dummy humidity value, replace with actual sensor reading
    return 50.0;
}
