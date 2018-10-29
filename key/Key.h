#ifndef __KEYS_H__
#define __KEYS_H__

#include <string>

using namespace std;


// pressed: 1 key press 0 k

namespace zc55
{

  typedef enum{
  KEY_NONE_STATUS = 0,
  KEY_PRESSED ,
  KEY_RELEASED 
  }KeyStatus;

  #define KEY_START 31
  #define KEY_UPDATE 22
  #define KEY_DESTROY 18

  typedef void (*KeyCB)(unsigned char keyCode, KeyStatus pressed);

  
class Key
{  

  #define KEY_SELECT_DELAY_S 1 //ms
  #define KEY_SELECT_DELAY_US 0 //ms
public:
  Key(string name);
  ~Key(){};

  pthread_t startKeyMonitor();
  void stopKeyMonitor();
  static void* threadFunc(void *arg);
  static void reportKey();
  static void keyCBDummy(unsigned char keyCode, KeyStatus pressed);
  void setKeyCB(KeyCB cb);

private: 
  string keyName;
  string eventName;
  int keyNum;
public:
  static volatile int execFlag;
  static KeyCB keyCB;
};

} // namespace zc55

#endif