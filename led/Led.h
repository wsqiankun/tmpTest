#ifndef __LED_H__
#define __LED_H__

#define BLINK_1HZ 500
#define BLINK_2HZ 250
#define BLINK_5HZ 100

#include <string>

using namespace std;

namespace zc55
{
class Led
{
public:
    Led(string ledName);
    ~Led(){};
    int lightOn();
    int lightOff();
    int startBlink(int hz);
    int startBlink(int delayOn, int delayOff);
    int stopBlink();
    

private:
    const string LEDPATH = "/sys/class/leds/";
    string ledFullName;
    string ledBtightnessFile;
    string ledTriggerFile;
    string ledDelayOnFile;
    string ledDelayOffFile;
};
} // namespace zc55

#endif