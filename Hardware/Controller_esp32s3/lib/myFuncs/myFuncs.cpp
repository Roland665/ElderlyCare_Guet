
#include <stdint.h>
#include "myFuncs.h"

/* 将字符转换成整数在16进制中对应的数 0-0 A-10 F-15 */
uint8_t calc_charTonumber(char data){
  if(data <= '9' && data >= '0')
    return data-'0';
  if(data <= 'f' && data >= 'a')
    return data-'a'+10;
  if(data <= 'F' && data >= 'A')
    return data-'A'+10;
  return 0;
}

/* 将整数在16进制中对应的数转换成字符 0-0 10-A 15-F */
char calc_numberTochar(uint8_t data){
  if(data <= 9 && data >= 0)
    return data+'0';
  if(data <= 15 && data >= 10)
    return data-10+'A';
  return 0;
}
