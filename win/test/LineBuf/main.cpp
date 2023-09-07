// LineBuf.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//


#define TEST

#include "../../../src/linebuf/linebuf.h"
#include <stdarg.h>
using namespace MIB;

static void test_LineBuf()
{
    struct local {
        static void read(LineBuffer<20>& g, int line, ILineBuffer::Result ret, const char* text, int size) {
            const char* b = NULL;
            int s = 0;

            auto r = g.read(line, b, s);    //1 9 空に追加 
            auto f = r == ILineBuffer::Result::OK ? (memcmp(text, b, s) == 0 && s == size && ret == r) : (ret == r);

            if (f) {
                if (r == ILineBuffer::Result::OK) { printf("[OK] true %s %d\n", text, size); }
                else { printf("[OK]read   false\n"); }
            }
            else {
                if (r == ILineBuffer::Result::OK) { printf("[NG] %d->%d %s->%s %d->%d\n", ret, r, text, b, size, s); }
                else { printf("[NG]read   false\n"); }
            }
        }
        static void update(LineBuffer<20>& g, int line, const char* text, ILineBuffer::Result ret) {
            const char* b = NULL;
            int s = 0;

            auto r = g.update(line, text, (int)strlen(text));    //1 9 空に追加 
            auto f = (ret == r);
            printf("[%s]update  %d\n", (ret == r) ? "OK" : "NG", r);
        }

        static void remove(LineBuffer<20>& g, int line, ILineBuffer::Result ret) {
            const char* b = NULL;
            int s = 0;

            auto r = g.remove(line);    //1 9 空に追加 
            auto f = (ret == r);
            printf("[%s]remove  %d\n", (ret == r) ? "OK" : "NG", r);
        }
        static void buftest(LineBuffer<20>& g, const char* testpatt) {
            static char tmp[256];
            memset(tmp, 0, sizeof(tmp));

            MIB_UINT16 line = 0;
            const char* text = NULL;
            int n = 0;
            int size;
            for (int idx = 0;;) {
                idx = g.iter(idx, line, text, size);
                if (idx < 0) {
                    break;
                }
                n += sprintf(&tmp[n], "%d:%.*s,", line, size, text);
            }
            bool f = memcmp(testpatt, tmp, n) == 0 && n == strlen(testpatt);
            printf("[%s]buftest %d (%d) %s %s\n", f ? "OK" : "NG", n, g.freeSize(), testpatt, tmp);
        }

    };
    {
        LineBuffer<20> inst;
        local::read(inst, 1, ILineBuffer::Result::NG_NO_LINE, NULL, -1);
        local::remove(inst, 1, ILineBuffer::Result::NG_NO_LINE);
    }
    {
        LineBuffer<20> inst;
        local::update(inst, 1, "11", ILineBuffer::Result::OK);        //新規追加
        local::buftest(inst, "1:11,");
        local::update(inst, 2, "22", ILineBuffer::Result::OK);        //追記
        local::buftest(inst, "1:11,2:22,");
        local::update(inst, 5, "55", ILineBuffer::Result::OK);        //挿入
        local::buftest(inst, "1:11,2:22,5:55,");
        local::update(inst, 3, "33", ILineBuffer::Result::OK);        //臨界挿入
        local::buftest(inst, "1:11,2:22,3:33,5:55,");
        local::update(inst, 6, "A", ILineBuffer::Result::NG_OOM);    //OOM
        local::remove(inst, 2, ILineBuffer::Result::OK);                 //未定義削除
        local::buftest(inst, "1:11,3:33,5:55,");
        local::remove(inst, 2, ILineBuffer::Result::NG_NO_LINE);
        local::buftest(inst, "1:11,3:33,5:55,");
        local::update(inst, 3, "3333", ILineBuffer::Result::OK);        //定義済み更新1
        local::buftest(inst, "1:11,3:3333,5:55,");
        local::update(inst, 3, "3", ILineBuffer::Result::OK);        //定義済み更新2
        local::buftest(inst, "1:11,3:3,5:55,");
        local::update(inst, 3, "33333333", ILineBuffer::Result::NG_OOM);//定義済み更新3(OOM)
        local::buftest(inst, "1:11,3:3,5:55,");
        local::remove(inst, 1, ILineBuffer::Result::OK);
        local::remove(inst, 3, ILineBuffer::Result::OK);
        local::buftest(inst, "5:55,");
        local::update(inst, 0, "000", ILineBuffer::Result::OK);
        local::buftest(inst, "0:000,5:55,");
        local::update(inst, 9, "99", ILineBuffer::Result::OK);
        local::buftest(inst, "0:000,5:55,9:99,");
        local::remove(inst, 9, ILineBuffer::Result::OK);
        local::buftest(inst, "0:000,5:55,");
        local::update(inst, 5, "", ILineBuffer::Result::OK);
        local::buftest(inst, "0:000,5:,");
        local::update(inst, 0, "0", ILineBuffer::Result::OK);
        local::update(inst, 1, "1", ILineBuffer::Result::OK);
        local::update(inst, 4, "4", ILineBuffer::Result::OK);
        local::update(inst, 3, "3", ILineBuffer::Result::OK);
        local::buftest(inst, "0:0,1:1,3:3,4:4,5:,");
        local::remove(inst, 4, ILineBuffer::Result::OK);
        local::buftest(inst, "0:0,1:1,3:3,5:,");
        local::remove(inst, 0, ILineBuffer::Result::OK);
        local::buftest(inst, "1:1,3:3,5:,");
        local::remove(inst, 1, ILineBuffer::Result::OK);
        local::buftest(inst, "3:3,5:,");
        local::remove(inst, 3, ILineBuffer::Result::OK);
        local::buftest(inst, "5:,");
        local::remove(inst, 5, ILineBuffer::Result::OK);
        local::buftest(inst, "");

    }
}


static void test_DictBuf()
{
    class DictBufferDbg :public DictBuffer<20>{
    public:
        DictBufferDbg() :DictBuffer<20>(){
        }
        bool findByName(const char* name, int name_len, int& index, struct MemBlock*& mb, int start_index = 0) {
            return DictBuffer<20>::findByName(name, name_len, index, mb, start_index);
        }
        bool getMemBlockSize(int index, int& size) {
            return DictBuffer<20>::getMemBlockSize(index,size);

        }

    };
    struct local {
        static bool read(DictBufferDbg& g, const char* name, IDictBuffer::Result ret, const char* text, int size) {
            const char* b = NULL;
            int s = 0;
            const IDictBuffer::MemBlock* tmp;
            auto r = g.read(name,tmp);
            if (r!= ret) {
                //戻り値が異なる。
                printf("[NG]read invalid result:%d\n", r);
                return false;
            }
            if (r != IDictBuffer::Result::OK) {
                printf("[OK]read NG status:%d\n", r);
                return true;
            }
            if (tmp->type != IDictBuffer::TYPE_SHORT_TEXT) {
                printf("[NG]read Invalid type:%d\n",tmp->type);
                return false;
            }
            if (strlen(name) != tmp->name_len || memcmp(name, &tmp->data0, tmp->name_len) != 0) {
                printf("[NG]read Name missmatch\n");
                return false;
            }
            if (!tmp->asText(b, s)) {
                printf("[NG]read asText\n");
                return false;
            }
            if (size!=s || memcmp(b,text,size) != 0) {
                printf("[NG]read value missmatch\n");
                return false;
            }
            printf("[OK]read\n");
            return true;
        }
        static bool read(DictBufferDbg& g, const char* name, IDictBuffer::Result ret, int v) {
            int b = 0;
            const IDictBuffer::MemBlock* tmp;
            auto r = g.read(name, tmp);
            if (r != ret) {
                //戻り値が異なる。
                printf("[NG]read invalid result:%d\n", r);
                return false;
            }
            if (r != IDictBuffer::Result::OK) {
                printf("[OK]read NG status:%d\n", r);
                return true;
            }
            if (tmp->type != IDictBuffer::TYPE_INT32) {
                printf("[NG]read Invalid type:%d\n", tmp->type);
                return false;
            }
            if (strlen(name) != tmp->name_len || memcmp(name, &tmp->data0, tmp->name_len) != 0) {
                printf("[NG]read Name missmatch\n");
                return false;
            }
            if (!tmp->asInt32(b)) {
                printf("[NG]read asInt32\n");
                return false;
            }
            if (b != v) {
                printf("[NG]read value missmatch\n");
                return false;
            }
            printf("[OK]read\n");
            return true;
        }
        static bool read(DictBufferDbg& g, const char* name, IDictBuffer::Result ret, bool v) {
            bool b;
            const IDictBuffer::MemBlock* tmp;
            auto r = g.read(name, tmp);
            if (r != ret) {
                //戻り値が異なる。
                printf("[NG]read invalid result:%d\n", r);
                return false;
            }
            if (r != IDictBuffer::Result::OK) {
                printf("[OK]read NG status:%d\n", r);
                return true;
            }
            if (tmp->type != IDictBuffer::TYPE_BOOL_TRUE && tmp->type != IDictBuffer::TYPE_BOOL_FALSE) {
                printf("[NG]read Invalid type:%d\n", tmp->type);
                return false;
            }
            if (strlen(name) != tmp->name_len || memcmp(name, &tmp->data0, tmp->name_len) != 0) {
                printf("[NG]read Name missmatch\n");
                return false;
            }
            if (!tmp->asBool(b)) {
                printf("[NG]read asInt32\n");
                return false;
            }
            if (b != v) {
                printf("[NG]read value missmatch\n");
                return false;
            }
            printf("[OK]read\n");
            return true;
        }
        static bool update(DictBufferDbg& g, const char* name, const char* text, IDictBuffer::Result ret) {
            const char* b = NULL;
            int s = 0;
            //auto last_free=g.freeSize();

            auto r = g.update(name, text,strlen(text));    //1 9 空に追加
            if (r != ret) {
                //戻り値が異なる。
                printf("[NG]update invalid result:%d\n", r);
                return false;
            }
            if (r != IDictBuffer::Result::OK) {
                printf("[OK]read NG status:%d\n", r);
                return true;
            }
            ////消費サイズ確認
            //if (last_free - (2+strlen(name)+1+strlen(text))!=g.freeSize()) {
            //    printf("[NG]update invalid memory size\n");
            //    return false;

            //}
            //読出し確認
            IDictBuffer::Result r2 = {};
            if (!read(g, name, r2,text,strlen(text))) {
                printf("[NG]update read failed\n");
                return false;
            }
            printf("[OK]update\n");
            return true;
        }
        static bool update(DictBufferDbg& g, const char* name, int value, IDictBuffer::Result ret) {
            const char* b = NULL;
            int s = 0;

            auto r = g.update(name, value);    //1 9 空に追加
            if (r != ret) {
                //戻り値が異なる。
                printf("[NG]update invalid result:%d\n", r);
                return false;
            }
            if (r != IDictBuffer::Result::OK) {
                printf("[OK]read NG status:%d\n", r);
                return true;
            }
            //読出し確認
            IDictBuffer::Result r2 = {};
            if (!read(g, name, r2, value)) {
                printf("[NG]update read failed\n");
                return false;
            }
            printf("[OK]update\n");
            return true;
        }
        static bool update(DictBufferDbg& g, const char* name, bool value, IDictBuffer::Result ret) {
            const char* b = NULL;
            int s = 0;

            auto r = g.update(name, value);    //1 9 空に追加
            if (r != ret) {
                //戻り値が異なる。
                printf("[NG]update invalid result:%d\n", r);
                return false;
            }
            if (r != IDictBuffer::Result::OK) {
                printf("[OK]read NG status:%d\n", r);
                return true;
            }
            //読出し確認
            IDictBuffer::Result r2 = {};
            if (!read(g, name, r2, value)) {
                printf("[NG]update read failed\n");
                return false;
            }
            printf("[OK]update\n");
            return true;
        }
        static bool remove(DictBufferDbg& g, const char* name, IDictBuffer::Result ret) {
            const char* b = NULL;
            int s = 0;
            if (ret != IDictBuffer::Result::OK) {
                auto r = g.remove(name);    //1 9 空に追加 
                if (r != ret) {
                    //戻り値が異なる。
                    printf("[NG]remove invalid result:%d\n", r);
                    return false;
                }
                else {
                    printf("[OK]remove NG status:%d\n", r);
                    return true;
                }
            }
            else {
                auto last_free = g.freeSize();
                const IDictBuffer::MemBlock* tmp;
                if (IDictBuffer::Result::OK != g.read(name, tmp)) {
                    printf("[NG]update preread failed\n");
                    return false;
                }
                IDictBuffer::MemBlock* tmp2;
                int idx=0;
                g.findByName(name, strlen(name), idx, tmp2, 0);
                auto pre_block_size = 0;
                if (!g.getMemBlockSize(idx, pre_block_size)) {
                    printf("[NG]update blocksize failed\n");
                    return false;
                }
                auto r = g.remove(name);    //1 9 空に追加 
                if (r != IDictBuffer::Result::OK) {
                    printf("[OK]update NG status:%d\n", r);
                    return false;
                }

                //消費サイズ確認
                if (last_free + pre_block_size != g.freeSize()) {
                    printf("[NG]update invalid memory size\n");
                    return false;
                }
            }
            printf("[OK]remove\n");
            return true;
        }
        static void buftest(DictBufferDbg& g, const char* testpatt) {
            static char tmp[256];
            memset(tmp, 0, sizeof(tmp));
            const IDictBuffer::MemBlock* mb_tmp;

            MIB_UINT16 line = 0;
            const char* text = NULL;
            int n = 0;
            int size=0;
            for (int idx = 0;;) {
                idx = g.iter(idx,mb_tmp);
                if (idx < 0) {
                    break;
                }
                n += sprintf(&tmp[n], "%.*s:", mb_tmp->name_len, &mb_tmp->data0);
                switch (mb_tmp->type) {
                case IDictBuffer::TYPE_SHORT_TEXT:
                {
                    const char* c;
                    int l;
                    mb_tmp->asText(c, l);
                    n += sprintf(&tmp[n], "%.*s,", l, c);
                }
                break;
                case IDictBuffer::TYPE_INT32:
                {
                    int v;
                    mb_tmp->asInt32(v);
                    n += sprintf(&tmp[n], "i%d,", v);
                }
                break;
                case IDictBuffer::TYPE_BOOL_TRUE:
                {
                    bool v;
                    mb_tmp->asBool(v);
                    n += sprintf(&tmp[n], "b%s,", v ? "true" : "false");
                }
                break;
                default:
                    n += sprintf(&tmp[n], "ERROR,");
                }
            }
            int n1 = strlen(tmp);
            int n2 = strlen(testpatt);
            if (n1 == n2 && memcmp(tmp, testpatt, n2)==0) {
                printf("[OK] buftest %s\n", tmp);
            }
            else {
                printf("[NG]  '%s' != '%s'\n", tmp, testpatt);
            }
        }

    };
    {
        //正常系1
        if(true){
            DictBufferDbg inst;
            local::update(inst, "k1", "str1", IDictBuffer::Result::OK);
            local::buftest(inst, "k1:str1,");//2+2+1+4=9byte
            local::update(inst, "k2", "str2", IDictBuffer::Result::OK);
            local::buftest(inst, "k1:str1,k2:str2,");
            local::update(inst, "k2", true, IDictBuffer::Result::OK);
            local::buftest(inst, "k1:str1,k2:btrue,");
            local::update(inst, "k1", 123, IDictBuffer::Result::OK);
            local::buftest(inst, "k1:i123,k2:btrue,");
            local::remove(inst, "k1", IDictBuffer::Result::OK);
            local::buftest(inst, "k2:btrue,");
        }
        //正常系2
        if (true) {
            DictBufferDbg inst;
            local::update(inst, "a1", "str1", IDictBuffer::Result::OK);
            local::buftest(inst, "a1:str1,");//2+2+1+4=9byte
            local::update(inst, "b2", "str2", IDictBuffer::Result::OK);
            local::buftest(inst, "a1:str1,b2:str2,");
            local::update(inst, "b2", true, IDictBuffer::Result::OK);
            local::buftest(inst, "a1:str1,b2:btrue,");
            local::update(inst, "a1", 123, IDictBuffer::Result::OK);
            local::buftest(inst, "a1:i123,b2:btrue,");
            local::remove(inst, "a1", IDictBuffer::Result::OK);
            local::buftest(inst, "b2:btrue,");
        }
        //正常系3
        if (true) {
            DictBufferDbg inst;
            local::update(inst, "b1", "str1", IDictBuffer::Result::OK);
            local::buftest(inst, "b1:str1,");//2+2+1+4=9byte
            local::update(inst, "a2", "str2", IDictBuffer::Result::OK);
            local::buftest(inst, "a2:str2,b1:str1,");
            local::update(inst, "a2", true, IDictBuffer::Result::OK);
            local::buftest(inst, "a2:btrue,b1:str1,");
            local::update(inst, "b1", 123, IDictBuffer::Result::OK);
            local::buftest(inst, "a2:btrue,b1:i123,");
            local::remove(inst, "b1", IDictBuffer::Result::OK);
            local::buftest(inst, "a2:btrue,");
        }
        //異常系たち
        if (true) {
            DictBufferDbg inst;
            local::update(inst, "b1", "str1", IDictBuffer::Result::OK);//2+2+1+4=9
            local::buftest(inst, "b1:str1,");//2+2+1+4=9byte
            local::read(inst, "b1", IDictBuffer::Result::OK, "str1", 4);
            local::read(inst, "a1", IDictBuffer::Result::NG_NO_KEYWORD,NULL,-1);
            local::update(inst, "c1", "012345", IDictBuffer::Result::OK);//2+2+1+6=11
            local::remove(inst, "c1", IDictBuffer::Result::OK);
            local::update(inst, "c1", "012345", IDictBuffer::Result::OK);//2+2+1+6=11
            local::update(inst, "c1", "012345", IDictBuffer::Result::OK);//2+2+1+6=11
            local::remove(inst, "c1", IDictBuffer::Result::OK);
            local::update(inst, "c1", "0123456", IDictBuffer::Result::NG_OOM);//2+2+1+6=11
            local::buftest(inst, "b1:str1,");//9
            local::update(inst, "c1", "012345", IDictBuffer::Result::OK);//2+2+1+6=11
        }
        //色々な型
        if (true) {
            DictBufferDbg inst;
            local::update(inst, "b1", 1, IDictBuffer::Result::OK);//2+2+4=6
            local::buftest(inst, "b1:i1,");//2+2+1+4=9byte
            local::read(inst, "b1", IDictBuffer::Result::OK, 1);
            local::remove(inst, "b1", IDictBuffer::Result::OK);
            local::read(inst, "b1", IDictBuffer::Result::NG_NO_KEYWORD, 1);

            local::update(inst, "b1", 1, IDictBuffer::Result::OK);//2+2+4=6
            local::buftest(inst, "b1:i1,");//2+2+1+4=9byte
            local::update(inst, "b1", "h", IDictBuffer::Result::OK);//2+2+1+3+1=6
            local::buftest(inst, "b1:h,");
            local::update(inst, "b2", "012345678", IDictBuffer::Result::OK);//2+2+1+9=14

        }


    }



}


int main()
{
    //GapVector<10>::test(); //テストして
    //LineBuffer<>::test_CacheTable();
    test_DictBuf();
}

