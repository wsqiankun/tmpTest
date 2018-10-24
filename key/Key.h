#ifndef __KEY_H__
#define __KEY_H__

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
  // Key(){};
  // ~Key(){};

  int keyInit();
  static void* threadFunc(void *arg);
  static void reportKey();

private: 
  const string keyName = "gpio-keys";
  string eventName;
public:
  static volatile int threadFlag;
  int a;
};

} // namespace zc55

#endif