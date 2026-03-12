#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "8484848";
const char* password = "shenshitao00";
const char* mqtt_uid = "38c70e700e8d91b301ff3a0d97f8fa82"; 

// 巴法云标准MQTT参数
const char* mqtt_server = "bemfa.com";
const int mqtt_port = 9501; // 正确端口
const char* topic_led = "led001";  // 控制主题
const char* topic_temp = "temp004"; // 数据主题

WiFiClient espClient;
PubSubClient client(espClient);

// ESP32-S3板载LED
#define LED_PIN 48

long lastMsg = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(115200);
  delay(10);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  Serial.print("WiFi: "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi已连 IP: " + WiFi.localIP().toString());
}

// 接收云端指令（控制LED）
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i=0; i<length; i++) msg += (char)payload[i];
  Serial.print("收到指令: "); Serial.println(msg);

  if (String(topic) == topic_led) {
    if (msg == "on") { digitalWrite(LED_PIN, HIGH); Serial.println("✅ LED开"); }
    if (msg == "off") { digitalWrite(LED_PIN, LOW); Serial.println("✅ LED关"); }
  }
}

// MQTT重连（巴法云标准）
void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT连接...");
    // 巴法云连接格式：clientId=私钥，用户user，密码=私钥
    if (client.connect(mqtt_uid, "user", mqtt_uid)) {
      Serial.println("成功");
      client.subscribe(topic_led); // 订阅控制主题
    } else {
      Serial.print("失败 rc="); Serial.print(client.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    // 模拟温湿度
    int temp = random(20, 30);
    int humi = random(40, 70);
    // 巴法云数据格式：温度,湿度
    String data = String(temp) + "," + String(humi);
    client.publish(topic_temp, data.c_str());
    Serial.print("上传: 温度=" + String(temp) + " 湿度=" + String(humi));
  }
}