#include <Arduino.h>
#include <WiFi.h>
#include <sensor_msgs/Joy.h>
#include <sensor_msgs/BatteryState.h>

#include "Config.h"
#include "ConfigReg.h"
#include "ROS1.h"
#include "Battery.h"


ConfigGroup configGroupRos1(FST("ROS1"));

ConfigStr configRos1Host(FST("Host"), 32, ROS1_HOST, FST("ROS1 server"), 0, &configGroupRos1);
ConfigUInt16 configRos1Port(FST("Port"), ROS1_PORT, FST("ROS1 server port number"), 0, &configGroupRos1);

WiFiClient ros1WifiClient;
ros::NodeHandle_<Ros1WiFiLink> ros1Node;


sensor_msgs::Joy ros1JoyMsg;
ros::Publisher ros1PublisherJoy(FST("remote_joy"), &ros1JoyMsg);
uint32_t ros1JoyTs_ = 0;
#ifndef ROS1_PUB_JOY_MS
#define ROS1_PUB_JOY_MS 100
#endif

#if BATTERY_PIN >= 0
sensor_msgs::BatteryState ros1BatteryMsg;
ros::Publisher ros1PublisherBattery(FST("remote_battery"), &ros1BatteryMsg);
uint32_t ros1BatteryTs_ = 0;
#ifndef ROS1_PUB_BATTERY_MS
#define ROS1_PUB_BATTERY_MS 10000
#endif
#endif

ros::Subscriber<std_msgs::Empty> ros1Subscriber1(FST("toggle_led"), &ros1Handler1);

uint32_t ros1LastConnectTs_ = 0;
bool ros1IsConnected_ = false;
bool ros1IsAdvertised_ = false;
bool ros1IsReady_ = false;
uint32_t ros1Counter = 0;

void rosTask_(void* parameter ) {
    while (true) {
        ros1Run();
        vTaskDelay(1);
    }
}

void rosInit() {
    xTaskCreate(
    rosTask_,   // Task function
    "ROS",          // String with name of task
    2048,           // Stack size in bytes
    NULL,           // Parameter passed as input of the task
    1,              // Priority of the task.
    NULL);          // Task handle.
}

ros::Time ros1Time(uint32_t ms) {
    ros::Time current_time;
    current_time.sec = ms / 1000; // + sec_offset;
    current_time.nsec = (ms % 1000) * 1000000UL; // + nsec_offset;
    ros::normalizeSecNSec(current_time.sec, current_time.nsec);
    return current_time;
}

bool ros1CheckConnectionState() {
    if (WiFi.status() != WL_CONNECTED) { 
        ros1IsConnected_ = false;
        ros1IsAdvertised_ = false;
        ros1IsReady_ = false;
        return false; 
    }
    uint32_t now = millis();
    if (!ros1IsConnected_) {
        if (ros1LastConnectTs_ != 0 && now < (ros1LastConnectTs_ + 1000)) { return false; }
        if (ros1LastConnectTs_ == 0) { DEBUG_printf(FST("ROS1 Wifi host:%s, port:%d\n"), configRos1Host.get(), configRos1Port.get()); }
        ros1LastConnectTs_ = now;
        if (!ros1WifiClient.connect(configRos1Host.get(), configRos1Port.get())) {
            DEBUG_println(FST("Waiting for ROS1 connection"));
            return false;
        }
        DEBUG_println(FST("ROS1 WIFI client connected"));
        ros1Node.initNode();
        ros1IsConnected_ = true;
    }
    if (!ros1WifiClient.connected()) {
        DEBUG_println(FST("Lost ROS1 WIFI client connection"));
        ros1IsConnected_ = false;
        ros1IsAdvertised_ = false;
        ros1IsReady_ = false;
        return false; 
    }
    if (!ros1IsAdvertised_) {
        ros1JoyMsg.header.frame_id = FST("remote");
        ros1JoyMsg.axes_length = JOY_AXIS_SIZE;
        ros1JoyMsg.axes = joyAxes;
        ros1JoyMsg.buttons_length = JOY_BUTTON_SIZE;
        ros1JoyMsg.buttons = joyButtons;
        ros1Node.advertise(ros1PublisherJoy);
#if BATTERY_PIN >= 0
        ros1BatteryMsg.header.frame_id = FST("remote");
        ros1Node.advertise(ros1PublisherBattery);
#endif
        ros1Node.subscribe(ros1Subscriber1);
        DEBUG_println(FST("ROS1 node is advertised"));
        ros1IsAdvertised_ = true;
    }
    if (ros1IsReady_) {
        if (ros1Node.connected()) { return true; }
        DEBUG_println(FST("Lost ROS1 node connection"));
        ros1IsAdvertised_ = false;
        ros1IsReady_ = false;
        return false;
    }
    if (ros1Node.connected()) {
        DEBUG_println(FST("ROS1 node is ready"));
        ros1IsReady_ = true;
        return true;
    }
    ros1Node.spinOnce();
    return false;
}

void ros1Run() {
    if (!ros1CheckConnectionState()) { return; }
    uint32_t now = millis();
    ros::Time rosNow = ros1Time(now);
    if ((now - ros1JoyTs_) >= ROS1_PUB_JOY_MS) {
        ros1JoyMsg.header.stamp = rosNow;
        ros1PublisherJoy.publish(&ros1JoyMsg);
        ros1JoyTs_ = now;
    }
#if BATTERY_PIN >= 0
    if ((now - ros1BatteryTs_) >= ROS1_PUB_BATTERY_MS) {
        ros1BatteryMsg.header.stamp = rosNow;
        ros1BatteryMsg.voltage = batteryVoltageFiltered;
        ros1BatteryMsg.percentage = (float) batteryChargeLevel;
        ros1PublisherBattery.publish(&ros1BatteryMsg);
        ros1BatteryTs_ = now;
    }
#endif    
    ros1Node.spinOnce();
}

void ros1Handler1(const std_msgs::Empty& toggle_msg) {
    DEBUG_println(FST("Got ROS1 message"));
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // blink the led
}

void Ros1WiFiLink::init() {
    // do your initialization here. this probably includes TCP server/client setup
}

// read a byte from the serial port. -1 = failure
int Ros1WiFiLink::read() {
    // implement this method so that it reads a byte from the TCP connection and returns it
    // you may return -1 is there is an error; for example if the TCP connection is not open
    return ros1WifiClient.read(); //will return -1 when it will works
}

// write data to the connection to ROS
void Ros1WiFiLink::write(uint8_t* data, int length) {
    // implement this so that it takes the arguments and writes or prints them to the TCP connection
    ros1WifiClient.write(data, length);
}
