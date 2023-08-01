// RpSolver.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#define TEST
#include "../../../src/rpsolver/rpsolver.h"
int main()
{
    struct local {
        static void parse(const char* s, const char* a) {
            RpSolver rp;
            RawTokenIterator rti(s);
            auto r = rp.parse(rti);
            const char* m = rp.sdump();
            if (r == RpSolver::Result::OK) {
                printf("%s -> %s(%s)\n", s, m, memcmp(a, m, strlen(a)) == 0 ? "OK" : "NG");
            }
            else {
                printf("ERR %d %s\n", r, m);
            }
        }
    };
    //INT32 TEST
    local::parse("1+127+32767+2147483647", "-2147450754");
    local::parse("-1-128-32768-2147483648", "2147450751");
    local::parse("1+(2+3+(4+5+6)+7)", "28");
    local::parse("-1+(-2+-3+(-4+-5+-6)+-7)", "-28");
    local::parse("-1+-2", "-3");
    local::parse("1+2", "3");
    local::parse("-1+-2*-3", "5");
    local::parse("1+2*3", "7");
    local::parse("-1+-2*-3+-4", "1");
    local::parse("1+2*3+4", "11");
    local::parse("-1+(-2+-3)", "-6");
    local::parse("1+(2+3)", "6");
    local::parse("1*-(2*3)", "-6");
    local::parse("1*--(2*3)", "6");
    local::parse("-1+(-2+-3)", "-6");
    local::parse("-1*+(-2+-3)", "5");
    local::parse("-10/(-2+-3)", "2");
    local::parse("-10%(-2+-3)", "0");

    ////chatGPT generated test
    local::parse("3+4*2", "11");         // 3 + 4 * 2 = 11
    local::parse("(7+3)*2", "20");       // (7 + 3) * 2 = 20
    local::parse("1+2+3+4+5", "15");     // 1 + 2 + 3 + 4 + 5 = 15
    local::parse("8*4-6/2", "29");       // 8 * 4 - 6 / 2 = 16 (修正)
    local::parse("(1+2)*(3+4)", "21");   // (1 + 2) * (3 + 4) = 21
    local::parse("-2*4+3", "-5");        // -2 * 4 + 3 = -5
    local::parse("-(3+4)*2", "-14");     // -(3 + 4) * 2 = -14
    local::parse("-2+3*4", "10");        // -2 + 3 * 4 = 10
    local::parse("-(2+3*4)", "-14");     // -(2 + 3 * 4) = -14
    local::parse("---2", "-2");          // -(-(-2)) = -2
    local::parse("--2*3", "6");          // -(-2) * 3 = 6
    local::parse("(((2+3*3)*4))", "44");   // (((2 + 3) * 4)) = 20
    local::parse("((2+3)*4+(5-2))", "23"); // ((2 + 3) * 4 + (5 - 2)) = 21
    local::parse("(2+3)*(4+5)", "45");   // (2 + 3) * (4 + 5) = 45
    local::parse("((2+3)*(4+5))*(3-1)", "90"); // ((2 + 3) * (4 + 5)) * (3 - 1) = 90

    local::parse("\"ABCDE\"+\"FG\"", "\"ABCDEFG\"");
    local::parse("\"ABCDE\"+1+2-3", "\"ABCDE12-3\"");
    local::parse("\"ABCDE\"+1+(2+3)", "\"ABCDE15\"");
    local::parse("\"AB\"+1+(2+3+4)", "\"AB19\"");
    local::parse("1+(2+\"AB\"+3)", "\"12AB3\"");
    local::parse("1+(2+\"AB\"+3*-2)", "\"12AB-6\"");
    local::parse("1<3", "TRUE");
    local::parse("3<3", "FALSE");
    local::parse("3<3+1", "TRUE");
    //////        local::parse("(3<3)+1", "TRUE");
    local::parse("1<=3", "TRUE");
    local::parse("3<=3", "TRUE");
    local::parse("3<=4", "TRUE");
    local::parse("1>3", "FALSE");
    local::parse("3>3", "FALSE");
    local::parse("3>4", "FALSE");
    local::parse("1>=3", "FALSE");
    local::parse("3>=3", "TRUE");
    local::parse("3>=4", "FALSE");
    local::parse("3==4", "FALSE");
    local::parse("3==3", "TRUE");
    local::parse("3!=4", "TRUE");
    local::parse("3<>4", "TRUE");
    local::parse("9 And(4+1)", "1");
    local::parse("8 Or (4-1)", "11");
    local::parse("1 Xor 0", "1");
    local::parse("3+(1 << 1)", "5");
    local::parse("2-(2 >> 1)", "1");
    local::parse("4-2 << 1", "4");
    local::parse("Not 0", "-1");
    local::parse("Not (1==1)", "FALSE");
    local::parse("(1!=1) And (1==1)", "FALSE");
    local::parse("(2==2) And (1==1)", "TRUE");
    local::parse("(1!=1) Or (1==1)", "TRUE");
    return 0;
}

