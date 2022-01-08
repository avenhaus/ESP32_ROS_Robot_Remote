#ifndef _ROS1_H_
#define _ROS1_H_

#include <Arduino.h>
#include <ros.h>
#include <std_msgs/Empty.h>

void rosInit();
void ros1Run();

ros::Time ros1Time(uint32_t ms);

class Ros1WiFiLink {
  public:
    Ros1WiFiLink() {};
    void init();
    int read();
    void write(uint8_t* data, int length);
    unsigned long time() { return millis(); }
};

extern char ros1Host[32];
extern uint16_t ros1Port;

void ros1Handler1(const std_msgs::Empty& toggle_msg);

#endif  // _ROS1_H_