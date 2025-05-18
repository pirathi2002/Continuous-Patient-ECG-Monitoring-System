#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Display constants
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)

// Initialize display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Potentiometer pin
int potPin = 34;

// Heart rate parameters
int heart_rate = 60;  // Default heart rate (BPM)
float cycle_length;    // Cycle length based on heart rate (in milliseconds)

// Timing for ECG segments based on heart rate
float P_DURATION, PR_SEGMENT, QRS_DURATION, ST_SEGMENT, T_DURATION, TP_SEGMENT;

// Amplitude settings for ECG waveform (flipped)
#define BASELINE 0
#define P_AMPLITUDE -0.2
#define Q_AMPLITUDE 0.3
#define R_AMPLITUDE -1.0
#define S_AMPLITUDE 0.5
#define T_AMPLITUDE -0.3
#define PI 3.14159265359

// Current time for waveform generation
float current_time = 0;

// Scaling factors
float PT_SCALING = 0.85; // P-T interval moderately shortens with heart rate
float TP_SCALING = 2.5;  // T-P segment shortens significantly with high heart rate

// WiFi and MQTT setup
const char* ssid = "Wokwi-GUEST"; // Your WiFi SSID
const char* password = ""; // Your WiFi password
const char* mqtt_server = "broker.hivemq.com"; // MQTT broker address
const char* mqtt_topic = "Warning/msg/bpm"; // Topic for warnings

WiFiClient espClient;
PubSubClient client(espClient);

// Heart rate threshold values
const int normalHeartRateLow = 60;  // Minimum heart rate
const int normalHeartRateHigh = 100; // Maximum heart rate

// Function to calculate cycle lengths based on heart rate
void update_cycle_length() {
    cycle_length = 60000.0 / heart_rate;  // Time for one cycle in ms
    
    // Shorten T-P interval significantly at high BPM
    TP_SEGMENT = cycle_length * 0.4 * (1 - (heart_rate - 40) / 140.0) * TP_SCALING;
    TP_SEGMENT = fmax(TP_SEGMENT, 3);  // Minimum T-P duration for very high BPMs
    
    // Adjust P-T interval moderately with BPM
    float PT_duration = cycle_length - TP_SEGMENT;
    P_DURATION = PT_duration * 0.1 * PT_SCALING;
    PR_SEGMENT = PT_duration * 0.12 * PT_SCALING;
    QRS_DURATION = PT_duration * 0.15; // Keep QRS stable
    ST_SEGMENT = PT_duration * 0.15 * PT_SCALING;
    T_DURATION = PT_duration * 0.23 * PT_SCALING;
}

// Function to generate ECG signal
float ecg_signal(float t) {
    // Normalize time to within one cycle
    t = fmod(t, cycle_length);
    
    // P wave
    if (t < P_DURATION) {
        float phase = t / P_DURATION;
        return P_AMPLITUDE * sin(PI * phase);
    }
    
    // PR segment (baseline)
    else if (t < P_DURATION + PR_SEGMENT) {
        return BASELINE;
    }
    
    // QRS complex
    else if (t < P_DURATION + PR_SEGMENT + QRS_DURATION) {
        float phase = (t - (P_DURATION + PR_SEGMENT)) / QRS_DURATION;
        
        if (phase < 0.25) return Q_AMPLITUDE * (phase / 0.25);         // Q wave (sharp descent)
        else if (phase < 0.5) return R_AMPLITUDE * (1 - 2 * (phase - 0.25)); // R wave (sharp peak)
        else return S_AMPLITUDE * ((phase - 0.5) / 0.5);               // S wave (sharp ascent)
    }
    
    // ST segment (baseline)
    else if (t < P_DURATION + PR_SEGMENT + QRS_DURATION + ST_SEGMENT) {
        return BASELINE;
    }
    
    // T wave
    else if (t < P_DURATION + PR_SEGMENT + QRS_DURATION + ST_SEGMENT + T_DURATION) {
        float phase = (t - (P_DURATION + PR_SEGMENT + QRS_DURATION + ST_SEGMENT)) / T_DURATION;
        return T_AMPLITUDE * sin(PI * phase);
    }
    
    // T-P segment (baseline)
    else {
        return BASELINE;
    }
}

// Helper function to map the signal to display coordinates
uint8_t map_to_display(float signal_value, uint8_t screen_height) {
    return (uint8_t)((signal_value + 1.0) * screen_height / 2.0);
}

// Function to draw the ECG frame
void draw_ecg_frame(uint8_t* buffer, int buffer_size, float start_time) {
    for (int i = 0; i < buffer_size; i++) {
        float t = start_time + (float)i * cycle_length / buffer_size;
        float signal = ecg_signal(t);
        buffer[i] = map_to_display(signal, SCREEN_HEIGHT);
    }
}

// Function to send a warning message via MQTT
void send_warning(const char* message) {
    if (client.publish(mqtt_topic, message)) {
        Serial.println("Warning sent: " + String(message));
    } else {
        Serial.println("Warning failed to send");
    }
}

// Function to connect to WiFi
void setupWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" WiFi connected");
}

// Function to reconnect to MQTT broker
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client")) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    
    // Update the cycle lengths initially
    update_cycle_length();

    // Connect to WiFi and set up MQTT
    setupWiFi();
    client.setServer(mqtt_server, 1883); // Default MQTT port
}

void loop() {
    // Ensure MQTT client stays connected
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    
    // Read potentiometer and map to heart rate (40 BPM to 180 BPM)
    heart_rate = map(analogRead(potPin), 0, 4095, 40, 180);
    
    // Update cycle lengths based on the new heart rate
    update_cycle_length();
    
    display.clearDisplay();
    
    // Buffer for ECG frame
    uint8_t display_buffer[SCREEN_WIDTH];
    draw_ecg_frame(display_buffer, SCREEN_WIDTH, current_time);
    
    // Draw ECG line point by point
    for (int i = 1; i < SCREEN_WIDTH; i++) {
        display.drawLine(i - 1, display_buffer[i - 1], i, display_buffer[i], SSD1306_WHITE);
    }

    // Display the current heart rate
    display.setCursor(0, 0);
    display.setTextSize(1.5);
    display.print("HR: ");
    display.print(heart_rate);
    display.print(" BPM");

    display.display();
    
    // Check heart rate for warnings
    if (heart_rate < normalHeartRateLow) {
        send_warning("Warning: Low Heart Rate!");
    } else if (heart_rate > normalHeartRateHigh) {
        send_warning("Warning: High Heart Rate!");
    }

    // Advance time
    current_time += 10;
    delay(20);
}
