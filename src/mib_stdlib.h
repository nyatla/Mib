#pragma once
#include <crtdbg.h>
#include <cstdint>
#include <climits>
#include <vcruntime_string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




namespace MIB{
    #define MIB_ASSERT(e) _ASSERT((e))

    typedef char MIB_INT8;
    typedef unsigned char MIB_UINT8;
    typedef short MIB_INT16;
	typedef unsigned short MIB_UINT16;
    typedef int MIB_INT32;
    typedef unsigned int MIB_UINT32;




    enum class ParserResult {
        OK,
        NG,
        NG_EOF,
        NG_IO,
        NG_TokenTooLong,
        NG_StackOverFlow,
        NG_SyntaxError,
        NG_InvalidNumber,
        NG_NumberRange,
        NG_UnknownDelimiter,
        NG_UnknownToken,
    };

    class ErrorMessage {
    public:
        const ParserResult eid;
        const char* message;
    public:
        ErrorMessage(ParserResult eid_, const char* message_) :eid(eid_), message(message_) {}
    };


}



/********************************************************************************
* 
* Functions
* 
********************************************************************************/
namespace MIB {

    static char* itoa(int num, char* str, int base) {
        if (base < 2 || base > 36) {
            return NULL;
        }

        int i = 0;
        int isNegative = (num < 0);

        // 数値を文字列に変換して逆順に格納
        do {
            int rem = abs(num) % base;
            str[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'A');
        } while (num /= base);

        if (isNegative) {
            str[i++] = '-';
        }

        str[i] = '\0';

        // 文字列を反転
        int start = 0;
        int end = i - 1;
        while (start < end) {
            char temp = str[start];
            str[start++] = str[end];
            str[end--] = temp;
        }

        return str;
    }








    
    inline void int2bytes(MIB_INT32 s, void* buf){
        auto b = (unsigned char*)buf;
        *b = (s >> 24) & 0xff;
        *(b + 1) = (s >> 16) & 0xff;
        *(b + 2) = (s >> 8) & 0xff;
        *(b + 3) = s & 0xff;
    }

    inline void short2bytes(MIB_INT16 s, void* buf){
        auto b = (unsigned char*)buf;
        *b = (s >> 8) & 0xff;
        *(b + 1) = s & 0xff;
    }
    inline void ushort2bytes(MIB_UINT16 s, void* buf) {
        auto b = (unsigned char*)buf;
        *b = (s >> 8) & 0xff;
        *(b + 1) = s & 0xff;
    }




    inline MIB_INT16 bytes2short(const void* p){
        auto b = (const unsigned char*)p;
        return (MIB_INT16)(((MIB_UINT16)b[0]) << 8) | b[1];

    }
    inline MIB_INT32 bytes2int(const void* p){
        auto b = (const unsigned char*)p;
        return (MIB_INT32)(((MIB_UINT32)b[0]) << 24) | (((MIB_UINT32)b[1]) << 16) | (((MIB_UINT32)b[2]) << 8) | b[3];
    }
    inline MIB_UINT16 bytes2ushort(const void* p) {
        auto b = (const unsigned char*)p;
        return (MIB_UINT16)(((MIB_UINT16)b[0]) << 8) | b[1];

    }


    /// <summary>
    /// 整数を文字列にしたときの桁数（符号付き）を返します。
    /// Generated ChatGPT 
    /// </summary>
    /// <param name="number"></param>
    /// <returns></returns>
    int countDigits(int number) {
        int count = 0;

        // Handle the case when the number is 0 separately
        if (number == 0) {
            return 1;
        }

        // Handle negative numbers by making them positive
        if (number < 0) {
            // 末尾桁を気にしながら正の値に
            number = (number == INT32_MIN) ? INT32_MAX : -number;
            count++; // Add one more digit for the negative sign
        }

        // Determine the number of digits without using division
        while (number > 0) {
            number /= 10;
            count++;
        }
        return count; // Add one more digit for the last non-zero digit
    }









}