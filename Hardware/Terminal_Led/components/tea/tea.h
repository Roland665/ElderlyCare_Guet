#ifndef _TEA_H_  
#define _TEA_H_
#include "stm32f10x.h"
/********************************************************************* 
 *                           函数 
 * **********************************************************************/  
/********************************************************************* 
 *tea加密 
*参数:v:要加密的数据,长度为8字节
*     k:加密用的key,长度为16字节
* **********************************************************************/  
static void tea_encrypt(u32 *v,u32 *k);  
/********************************************************************* *                           
 * tea解密 
 * *参数:v:要解密的数据,长度为8字节 
 * *     k:解密用的key,长度为16字节 
 * **********************************************************************/  
static void tea_decrypt(u32 *v,u32 *k);  
/********************************************************************* *                           
 * 加密算法 *参数:src:源数据,所占空间必须为8字节的倍数.加密完成后密文也存放在这 *     
 * size_src:源数据大小,单位字节 *     
 * key:密钥,16字节 
 * *返回:密文的字节数 
 * **********************************************************************/  
uint16_t encrypt(uint8_t *src,uint16_t size_src,uint8_t *key);  
/********************************************************************* *                           
 * 解密算法 *参数:src:源数据,所占空间必须为8字节的倍数.解密完成后明文也存放在这 
 * *     size_src:源数据大小,单位字节 
 * *     key:密钥,16字节 
 * *返回:明文的字节数,如果失败,返回0 
 * **********************************************************************/  
uint16_t decrypt(uint8_t *src,uint16_t size_src,uint8_t *key);  
#endif  
