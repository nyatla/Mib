// RpSolver.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#define TEST
#include "../../../src/rpsolver/rpsolver.h"
int main()
{
    //計算のテスト
    struct local {
        static void parse(const char* s, const char* a) {
            RpSolver rp;
            RawTokenIterator rti(s);
            auto r = rp.parse(rti);
            const char* m = rp.sdump();
            if (r!=RpSolver::Result::OK) {
                if (a == NULL) {
                    printf("[OK] ERROR=%d OUT: %s\n", r, m);
                }
                else if(memcmp(a,m,strlen(a))==0){
                    printf("[OK] ERROR=%d IN:%s -> OUT: %s\n", r,s, m);
                }
                else {
                    printf("[NG] ERROR=%d IN: %s OUT: %s test=%s\n", r, s, m, a);
                }
            }
            else {
                printf("[%s] IN: %s -> OUT: %s\n", memcmp(a, m, strlen(a)) == 0 ? "OK" : "NG", s, m);
            }
        }
        static void test1()
        {
            //INT32 TEST
            local::parse("(9)", "9");
            local::parse("(9+(-3)*2)", "3");
            local::parse("-(9+(-3)*2)+2", "-1");
            local::parse("1+127+32767+2147483647", "-2147450754");
            local::parse("-1-128-32768-2147483648", "2147450751");
            local::parse("1+(2+3+(4+5+6)+7)", "28");
            local::parse("-1+(-2+-3+(-4+-5+-6)+-7)", "-28");
            local::parse("(1)", "1");
            local::parse("10/(3+2+1)", "1");
            local::parse("-10/(-2+-3-8)", "0");
            local::parse("-10%(-2+-3)", "0");
            local::parse("5*(3+3)", "30");
            local::parse("1+(2+3)", "6");
            local::parse("(3+3)*5", "30");
            local::parse("((3+3)*5)", "30");
            local::parse("((2*3+-3)*5)", "15");
            local::parse("(2+(2*3+-3)*5)", "17");
            local::parse("(2+5+3+(2*3+-3)*5-3)", "22");
            local::parse("2+5+3+(2*3+-3)*5-3", "22");
            local::parse("-1+-2", "-3");
            local::parse("1+2", "3");
            local::parse("-1+-2*-3", "5");
            local::parse("1+2*3", "7");
            local::parse("-1+-2*-3+-4", "1");
            local::parse("1+2*3+4", "11");
            local::parse("Not 1", "-2");
            local::parse("Not -2", "1");
            local::parse("Not 2", "-3");
            local::parse("- Not 2", "3");
            local::parse("-1+(-2+-3)", "-6");
            local::parse("1+(2+3)", "6");
            local::parse("1*-(2*3)", "-6");
            local::parse("1*--(2*3)", "6");
            local::parse("-1+(-2+-3)", "-6");
            local::parse("-1*+(-2+-3)", "5");
            local::parse("-10/(-2+-3)", "2");
            local::parse("-10%(-2+-3)", "0");
            local::parse("2-*10", NULL);
            local::parse("2*-10", "-20");
        }
        static void test2() {
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
            local::parse("(3)+4", "7");
            local::parse("-(3)+4", "1");
            local::parse("(-3+1)+4", "2");
            local::parse("-(-1)+4", "5");
        }
        static void test3() {
            //String test
            local::parse("\"ABCDE\"+(2*5-3)", "\"ABCDE7\"");//OK
            local::parse("\"ABCDE\"+\"FG\"", "\"ABCDEFG\"");
            local::parse("\"ABCDE\"+1+2-3", "\"ABCDE12-3\"");
            local::parse("\"ABCDE\"+1+(2+3)", "\"ABCDE15\"");
            local::parse("\"AB\"+1+(2+3+4)", "\"AB19\"");
            local::parse("-\"ABCDE\"+1+(2+3)", NULL);
            local::parse("\"ABCDE\"-(2+3)", "\"ABCDE\" 5");//ERROR

            local::parse("1+(2+\"AB\"+3)", "\"12AB3\"");
            local::parse("1+(2+\"AB\"+3*-2)", "\"12AB-6\"");
            local::parse("1<3", "TRUE");
            local::parse("3<3", "FALSE");
            local::parse("3<3+1", "TRUE");
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
        }
        static void test4() {
            local::parse("1,(1,2)", "1 , 1 , 2");
            local::parse("1,(1,2+3)", "1 , 1 , 5");
            local::parse("(1,2+3),1,", "1 , 5 , 1");
            local::parse("1,(3+5,2)", "1 , 8 , 2");
            local::parse("1,(3+\"A\",2)", "1 , \"3A\" , 2");

            local::parse("1,(2+5+3)", "1 , 10");
            local::parse("1,(1,2+3)", "1 , 1 , 5");
            local::parse("1,(3+5,2)", "1 , 8 , 2");
            local::parse(",(3+5,2)", ", 8 , 2");
            local::parse("4,(3+5,(2==2),2)", "4 , 8 , TRUE , 2");
            local::parse("1,-(1,2)", NULL);
            local::parse("1,(1-,2)", NULL);
            local::parse("(-1,1),4", "-1 , 1 , 4");
            local::parse("(1,-1),4", "1 , -1 , 4");
            local::parse("(1+5,-1*3),4", "6 , -3 , 4");
            local::parse("(1+5,-1*3),4", "6 , -3 , 4");
            local::parse("(-(-8*2)+4,((4),2))", "20 , 4 , 2");

        }

    };
    local::test1();
    local::test2();
    local::test3();
    local::test4();
    return 0;
}

int main2() {
    {
        RpQueue<> rq;
        rq.pushInt(1);
        rq.pushInt(1);
        rq.pop(1);
        rq.pushInt(300);
        rq.pop(1);
        rq.pushInt(70000);
        rq.pop(1);
        rq.pushKeyword("012", 3);
        rq.pop(1);
        rq.pushBool(true);
        rq.pop(1);
        rq.pushStr("0123456789", 10);
        rq.pop(1);
        rq.pushStr("0123456789012345678901234567890123456789012345678901234567890123456789", 70);
        rq.pop(1);
        //1が１つ残ってればOK
    }
    {
        struct local {
            static void peekTest(RpQueue<>& q, int idx, int val) {
                int t = 0;
                if (!q.peekInt(idx, t) || val != t) {
                    printf("[NG] test=%d,got=%d\n",val,t);
                }
                else {
                    printf("[OK] test = % d, got = % d\n",val,t);
                }
            }
            static void peekTest(RpQueue<>& q, int idx, const char* val) {
                const char* t=NULL;
                int s=0;
                if (!q.peekStr(idx, t,s)) {
                    printf("[NG] test=%s\n", val);
                }
                else {
                    if (memcmp(val, t, s) == 0) {
                        printf("[OK] test=%s,got=%.*s\n", val, s, t);
                    }
                    else {
                        printf("[NG] test=%s,got=%.*s\n", val, s, t);
                    }
                }
            }

        };


        RpQueue<> rq;
        rq.pushInt(1);
        local::peekTest(rq, -1, 1);
        rq.pushInt(256);
        local::peekTest(rq, -1, 256);
        rq.pushInt(3);
        rq.remove(1);
        local::peekTest(rq, -1, 3);
        local::peekTest(rq, -2, 1);
        //local::peekTest(rq, -3, 1);//ERROR 
        rq.pop(1);
        local::peekTest(rq, -1, 1);
        rq.pop(1);
        rq.pushStr("123", 3);
        rq.pushStr("4567", 4);
        local::peekTest(rq, -1, "4567");
        local::peekTest(rq, -2, "123");
        rq.pushStr("89", 2);
        rq.remove(0);
        local::peekTest(rq, -1, "89");
        local::peekTest(rq, -2, "4567");
        return 0;

    }


}
