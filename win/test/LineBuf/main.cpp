// LineBuf.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//


#define TEST

#include "../../../src/linebuf/linebuf.h"
#include <stdarg.h>
using namespace MIB;


int main()
{
    ///     GapVector<10>::test();
    ///     LineBuffer<>::test_CacheTable();
    LineBuffer<3*(10)>::test();
}

