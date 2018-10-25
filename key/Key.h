#ifndef __KEYS_H__
#define __KEYS_H__

#include <string>


#define KEY_START 31
#define KEY_UPDATE 22
#define KEY_DESTROY 18
using namespace std;

namespace zc55
{
class Key
{
public:
  Key(){};
  ~Key(){};

  int keyInit();
  static void* threadFunc(void *arg);
  static void reportKey();

private: 
  const string keyName = "gpio-keys";
  string eventName;
  int keyNum;
public:
  static volatile int threadFlag;
};

} // namespace zc55

#endif