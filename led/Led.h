#ifndef __LED_H__
#define __LED_H__


#define ERR_LED_NAME "error-led"

#include <string>
#include "../common/common.h"


using namespace std;

namespace zc55
{
class Led
{
public:
    Led(string ledName);
    ~Led(){};
    int ledInit();
    int lightOn();
    int lightOff();
    int startBlink(int hz);
    int startBlink(int delayOn, int delayOff);
    int stopBlink();
    

private:
    const string LEDPATH = "/sys/class/leds/";
    // const string LEDPATH = "/opt/leds/";
    string ledName;
    string ledFullName;
    string ledBtightnessFile;
    string ledTriggerFile;
    string ledDelayOnFile;
    string ledDelayOffFile;
};
} // namespace zc55

#endif