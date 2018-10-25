#ifndef __KEYS_H__
#define __KEYS_H__

#include <string>

using namespace std;

namespace zc55
{
class Key
{
  #define KEY_START 31
  #define KEY_UPDATE 22
  #define KEY_DESTROY 18
  #define KEY_SELECT_DELAY 1 //ms
public:
  Key(){};
  ~Key(){};

  pthread_t keyInit();
  static void* threadFunc(void *arg);
  static void reportKey();

private: 
  const string keyName = "gpio-keys";
  string eventName;
  int keyNum;
public:
  static volatile int execFlag;
};

} // namespace zc55

#endif