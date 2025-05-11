#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>

#include <Wire.h>
#include <MPU6500_WE.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#define MPU6500_ADDR 0x68

#include <secrets.h>
#include <index.h>

#define MDNS_HOSTNAME "flight-tracker"
#define SDA_PIN 8
#define SCL_PIN 9 // pin is 9 but on lolin_c3_mini SCL is on GPIO 10

MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);

AsyncWebServer server(80);

String request;

WiFiClient client;

// #ifndef LED_BUILTIN
#define LED_BUILTIN 8
// #endif


// Create empty vectors of size 1000
// std::vector<float> gx(200, 0);
// std::vector<float> gy(200, 0);
// std::vector<float> gz(200, 0);
std::vector<xyzFloat> acceleration(0);

std::vector<float> g(600, 0);

uint16_t buff_i = 0;


// String generateCSV(const std::vector<float>& list_x, const std::vector<float>& list_y, const std::vector<float>& list_z) {
//     String csvData = "x;y;z\n";
//     for (size_t i = 0; i < list_x.size(); i++) {
//         csvData += String(list_x[i]) + ";" + String(list_y[i]) + ";" + String(list_z[i]) + "\n";
//     }
//     return csvData;
// }

String generateCSV(const std::vector<float>& list) {
    String csvData = "time (s);acceleration (g)\n";
    for (size_t i = 0; i < list.size(); i++) {
        size_t i_shifted = (buff_i + i) % list.size();
        String g_string = String(list[i_shifted]);
        g_string.replace('.', ',');
        String time = String((float)i/10);
        time.replace('.', ',');
        csvData += time + ";" + g_string + "\n";
    }
    return csvData;
}


void setup()
{
    // initialize LED digital pin as an output.
    // pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    Wire.setPins(SDA_PIN, SCL_PIN);
    Wire.begin();  // Initialize I2C


    Serial.println("MPU6500 test!");

    // Try to initialize!
    if (!myMPU6500.init()) { // default address is 0x68
        Serial.println("Failed to find MPU6500 chip");
        // TODO: add warning in webserver
        while (1) {
            delay(100);
            Serial.println("Failed to find MPU6500 chip.");
        }
    }
    Serial.println("MPU6500 is connected!");
    Serial.println("Position you MPU6500 flat and don't move it.");
    delay(2000);
    Serial.println("Calibrating...");
    myMPU6500.autoOffsets();
    Serial.println("Done");

    /*  You can enable or disable the digital low pass filter (DLPF). If you disable the DLPF, you
     *  need to select the bandwidth, which can be either 8800 or 3600 Hz. 8800 Hz has a shorter delay,
     *  but higher noise level. If DLPF is disabled, the output rate is 32 kHz.
     *  MPU9250_BW_WO_DLPF_3600
     *  MPU9250_BW_WO_DLPF_8800
     */
    myMPU6500.enableGyrDLPF();
    //myMPU6500.disableGyrDLPF(MPU9250_BW_WO_DLPF_8800); // bandwidth without DLPF

    /*  Digital Low Pass Filter for the gyroscope must be enabled to choose the level.
     *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7
     *
     *  DLPF    Bandwidth [Hz]   Delay [ms]   Output Rate [kHz]
     *    0         250            0.97             8
     *    1         184            2.9              1
     *    2          92            3.9              1
     *    3          41            5.9              1
     *    4          20            9.9              1
     *    5          10           17.85             1
     *    6           5           33.48             1
     *    7        3600            0.17             8
     *
     *    You achieve lowest noise using level 6
     */
    myMPU6500.setGyrDLPF(MPU6500_DLPF_5);

    /*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
     *  Sample rate = Internal sample rate / (1 + divider)
     *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
     *  Divider is a number 0...255
     */
    myMPU6500.setSampleRateDivider(99);

    /*  MPU9250_GYRO_RANGE_250       250 degrees per second (default)
     *  MPU9250_GYRO_RANGE_500       500 degrees per second
     *  MPU9250_GYRO_RANGE_1000     1000 degrees per second
     *  MPU9250_GYRO_RANGE_2000     2000 degrees per second
     */
    myMPU6500.setGyrRange(MPU6500_GYRO_RANGE_250);

    /*  MPU9250_ACC_RANGE_2G      2 g
     *  MPU9250_ACC_RANGE_4G      4 g
     *  MPU9250_ACC_RANGE_8G      8 g
     *  MPU9250_ACC_RANGE_16G    16 g
     */
    myMPU6500.setAccRange(MPU6500_ACC_RANGE_2G);

    /*  Enable/disable the digital low pass filter for the accelerometer
     *  If disabled the the bandwidth is 1.13 kHz, delay is 0.75 ms, output rate is 4 kHz
     */
    myMPU6500.enableAccDLPF(true);

    /*  Digital low pass filter (DLPF) for the accelerometer, if enabled
     *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7
     *   DLPF     Bandwidth [Hz]      Delay [ms]    Output rate [kHz]
     *     0           460               1.94           1
     *     1           184               5.80           1
     *     2            92               7.80           1
     *     3            41              11.80           1
     *     4            20              19.80           1
     *     5            10              35.70           1
     *     6             5              66.96           1
     *     7           460               1.94           1
     */
    myMPU6500.setAccDLPF(MPU6500_DLPF_5);
    delay(200);

    Serial.print("Connecting to: ");
    Serial.println(WIFI_SSID);
    // WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }
    Serial.print("\n");
    Serial.print("Connected to Wi-Fi ");
    Serial.println(WiFi.SSID());
    delay(1000);
    server.begin(); /* Start the HTTP web Server */
    Serial.print("Connect to IP Address: ");
    Serial.print("http://");
    Serial.println(WiFi.localIP());
    // Initialize mDNS
    if (!MDNS.begin(MDNS_HOSTNAME)) {   // Set the hostname to "esp32.local"
        while(1) {
        Serial.println("Error setting up MDNS responder! Please restart the controller");
        delay(1000);
        }
    }
    Serial.print("mDNS responder started: http://");
    Serial.print(MDNS_HOSTNAME);
    Serial.print(".local");

    // Serve the HTML page from the file
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("ESP32 Web Server: New request received:");  // for debugging
        Serial.println("GET /");                                    // for debugging

        request->send(200, "text/html", webpage);
    });

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("ESP32 Web Server: Download request received");
        // String csvData = generateCSV(gx, gy, gz);
        String csvData = generateCSV(g);
        request->send(200, "text/csv", csvData);
    });

    // Start the server
    server.begin();
}

void loop()
{
    xyzFloat accRaw = myMPU6500.getAccRawValues();
    xyzFloat accCorrRaw = myMPU6500.getCorrectedAccRawValues();
    xyzFloat gValue = myMPU6500.getGValues();
    float resultantG = myMPU6500.getResultantG(gValue);
    float temp = myMPU6500.getTemperature();

    if (buff_i >= g.size()) {
        buff_i = 0;
        Serial.println("Reseting buffer index.");
    }
    // gx[buff_i] = gValue.x;
    // gy[buff_i] = gValue.y;
    // gz[buff_i] = gValue.z;

    g[buff_i] = resultantG;

    buff_i += 1;

    Serial.println("Raw acceleration values (x,y,z):");
    Serial.print(accRaw.x);
    Serial.print("   ");
    Serial.print(accRaw.y);
    Serial.print("   ");
    Serial.println(accRaw.z);
    Serial.println("Corrected ('calibrated') acceleration values (x,y,z):");
    Serial.print(accCorrRaw.x);
    Serial.print("   ");
    Serial.print(accCorrRaw.y);
    Serial.print("   ");
    Serial.println(accCorrRaw.z);
    Serial.println("g values (x,y,z):");
    Serial.print(gValue.x);
    Serial.print("   ");
    Serial.print(gValue.y);
    Serial.print("   ");
    Serial.println(gValue.z);
    Serial.print("Resultant g: ");
    Serial.println(resultantG); // should always be 1 g if only gravity acts on the sensor.
    Serial.print("Temperature in °C: ");
    Serial.println(temp);
    Serial.println();

    delay(100);
}
