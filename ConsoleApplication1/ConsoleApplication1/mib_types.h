#pragma once
#include <crtdbg.h>
#include <cstdint>
#include <climits>
#include <vcruntime_string.h>
#include <stdio.h>




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
        NG_TokenTooLong,
        NG_StackOverFlow,
        NG_SyntaxError,
        NG_InvalidNumber,
        NG_NumberRange,
        NG_UnknownDelimiter,
    };
}
