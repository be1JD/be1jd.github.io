#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>

// be1JD FPV Car firmware
// Matches the browser controller commands in FPV_Car/RC_Control_4.3.html.

const char* AP_SSID = "JetRay-FPV-Car";
const char* AP_PASS = "12345678";

const int SERVO_PIN = 26;
const int MOTOR_BACK_PNP = 19;
const int MOTOR_FORWARD_PNP = 18;
const int MOTOR_FORWARD_NPN = 5;
const int MOTOR_BACK_NPN = 21;

const int FORWARD_PWM_CHANNEL = 0;
const int BACK_PWM_CHANNEL = 1;
const int PWM_FREQ = 16000;
const int PWM_RESOLUTION = 8;

int driveSpeed = 128;
int steerCenter = 90;
int steerMin = 30;
int steerMax = 170;
int servoMinUs = 1000;
int servoMaxUs = 2000;
bool reverseSteering = false;

WebServer server(80);
WebSocketsServer webSocket(81);
Servo steeringServo;

void stopMotor() {
  ledcWrite(FORWARD_PWM_CHANNEL, 0);
  ledcWrite(BACK_PWM_CHANNEL, 0);
  digitalWrite(MOTOR_FORWARD_PNP, HIGH);
  digitalWrite(MOTOR_BACK_PNP, HIGH);
  digitalWrite(MOTOR_FORWARD_NPN, LOW);
  digitalWrite(MOTOR_BACK_NPN, LOW);
}

void driveForward() {
  digitalWrite(MOTOR_BACK_PNP, HIGH);
  digitalWrite(MOTOR_BACK_NPN, LOW);
  digitalWrite(MOTOR_FORWARD_PNP, LOW);
  ledcWrite(BACK_PWM_CHANNEL, 0);
  ledcWrite(FORWARD_PWM_CHANNEL, driveSpeed);
}

void driveBackward() {
  digitalWrite(MOTOR_FORWARD_PNP, HIGH);
  digitalWrite(MOTOR_FORWARD_NPN, LOW);
  digitalWrite(MOTOR_BACK_PNP, LOW);
  ledcWrite(FORWARD_PWM_CHANNEL, 0);
  ledcWrite(BACK_PWM_CHANNEL, driveSpeed);
}

void brakeMotor() {
  ledcWrite(FORWARD_PWM_CHANNEL, 0);
  ledcWrite(BACK_PWM_CHANNEL, 0);
  digitalWrite(MOTOR_FORWARD_PNP, LOW);
  digitalWrite(MOTOR_BACK_PNP, LOW);
  digitalWrite(MOTOR_FORWARD_NPN, HIGH);
  digitalWrite(MOTOR_BACK_NPN, HIGH);
}

void setSteering(int angle) {
  angle = constrain(angle, steerMin, steerMax);
  if (reverseSteering) {
    angle = steerMax - (angle - steerMin);
  }
  steeringServo.write(angle);
}

String configPayload() {
  return String(driveSpeed) + "," + String(steerCenter) + "," + String(steerMin) + "," + String(steerMax) +
         ",0," + String(servoMinUs) + "," + String(servoMaxUs) + ",0,0,0," + String(reverseSteering ? 1 : 0) +
         "," + AP_SSID + "," + AP_PASS;
}

void broadcastStatus(const String& message) {
  webSocket.broadcastTXT(message);
}

void handleCommand(String command) {
  command.trim();

  if (command == "hello" || command == "cfg_load") {
    broadcastStatus("cfg_sync:" + configPayload());
    return;
  }

  if (command.startsWith("speed:")) {
    driveSpeed = constrain(command.substring(6).toInt(), 0, 255);
    broadcastStatus("speed:" + String(driveSpeed));
    return;
  }

  if (command.startsWith("steer:")) {
    setSteering(command.substring(6).toInt());
    broadcastStatus("steer:ok");
    return;
  }

  if (command == "forward_start") {
    driveForward();
    broadcastStatus("forward");
    return;
  }

  if (command == "forward_stop" || command == "back_stop" || command == "break_stop") {
    stopMotor();
    setSteering(steerCenter);
    broadcastStatus("stopped");
    return;
  }

  if (command == "back_start") {
    driveBackward();
    broadcastStatus("backward");
    return;
  }

  if (command == "rear_break") {
    brakeMotor();
    broadcastStatus("brake");
    return;
  }

  if (command.startsWith("cfg_speed:")) driveSpeed = constrain(command.substring(10).toInt(), 0, 255);
  else if (command.startsWith("cfg_center:")) steerCenter = constrain(command.substring(11).toInt(), 0, 180);
  else if (command.startsWith("cfg_stmin:")) steerMin = constrain(command.substring(10).toInt(), 0, 180);
  else if (command.startsWith("cfg_stmax:")) steerMax = constrain(command.substring(10).toInt(), 0, 180);
  else if (command.startsWith("cfg_servo_min:")) servoMinUs = constrain(command.substring(14).toInt(), 500, 2500);
  else if (command.startsWith("cfg_servo_max:")) servoMaxUs = constrain(command.substring(14).toInt(), 500, 2500);
  else if (command.startsWith("cfg_reverse:")) reverseSteering = command.substring(12).toInt() == 1;
  else if (command == "cfg_save") broadcastStatus("settings saved for this session");
  else if (command == "cfg_factory") broadcastStatus("factory defaults loaded");

  steeringServo.attach(SERVO_PIN, servoMinUs, servoMaxUs);
  setSteering(steerCenter);
}

void onWebSocketEvent(uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    webSocket.sendTXT(client, "cfg_sync:" + configPayload());
  }

  if (type == WStype_TEXT) {
    handleCommand(String((char*)payload).substring(0, length));
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_FORWARD_PNP, OUTPUT);
  pinMode(MOTOR_BACK_PNP, OUTPUT);
  pinMode(MOTOR_FORWARD_NPN, OUTPUT);
  pinMode(MOTOR_BACK_NPN, OUTPUT);
  stopMotor();

  ledcSetup(FORWARD_PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(BACK_PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_FORWARD_NPN, FORWARD_PWM_CHANNEL);
  ledcAttachPin(MOTOR_BACK_NPN, BACK_PWM_CHANNEL);

  steeringServo.attach(SERVO_PIN, servoMinUs, servoMaxUs);
  setSteering(steerCenter);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("Connect to WiFi: ");
  Serial.println(AP_SSID);
  Serial.print("Open: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", []() {
    server.send(200, "text/html", "<h1>be1JD FPV Car</h1><p>Open the web controller and connect to ws://192.168.4.1:81/</p>");
  });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}