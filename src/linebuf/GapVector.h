// LineBuf.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#pragma once
#include "./linebuf_imports.h"

#define TEST


namespace MIB {

    /// <summary>
    /// 最大でint16maxのサイズを持つGapVectorです。
    /// </summary>
    /// <typeparam name="SIZE"></typeparam>
    template<int SIZE> class GapVector {
    private:
        MIB_INT16 _begin;
        MIB_INT16 _end;
        MIB_UINT8 _buf[SIZE] = {};
    public:
        GapVector() :_begin(0), _end(SIZE) {
        }
        void clear() {
            this->_begin = 0;
            this->_end = SIZE;
        }
        /// <summary>
        /// 格納されているデータサイズ
        /// </summary>
        /// <returns></returns>
        int size()const {
            return SIZE - (this->_end - this->_begin);
        }
        int freeSize()const {
            return (this->_end - this->_begin);
        }

        /// <summary>
        /// ギャップを挟んで文字列を移動する
        /// </summary>
        /// <typeparam name="SIZE"></typeparam>
        bool move(int len)
        {
            auto& buf = this->_buf;
            if (len > 0 && this->_begin >= len) {
                memmove(&buf[this->_end - len], &buf[this->_begin - len], len);
                this->_begin -= len;
                this->_end -= len;
                return true;
            }
            if (len < 0 && -len <= SIZE - this->_end) {
                memmove(&buf[this->_begin], &buf[this->_end], -len);
                this->_begin += -len;
                this->_end += -len;
                return true;
            }
            if (len == 0) {
                return true;
            }
            return false;
        }
        /// <summary>
        /// バッファのidxのポインタを、lenサイズだけ連続して読みだせるようにして返します。
        /// </summary>
        /// <param name="idx"></param>
        /// <param name="len"></param>
        /// <param name="out">falseの場合、更新しない</param>
        /// <returns></returns>
        bool ptr(int idx, int len, const void*& out) {
            MIB_ASSERT(idx >= 0);
            MIB_ASSERT(len >= 0);
            auto begin = this->_begin;
            auto end = this->_end;

            auto gp = end - begin;
            auto idx_e = idx + len;
            if (idx_e +gp > SIZE) {
                //範囲外
                return false;
            }

            if (idx_e <= begin) {
                out = &this->_buf[idx];
                return true;
            }
            if (idx+gp >= end) {
                out = &this->_buf[idx+gp];
                return true;
            }
            //左にまとめる
            if (idx < begin && end <= idx_e+gp) {
                move(-(idx+len - begin));
                out=&this->_buf[idx];
                return true;
            }
            return false;
        }




        bool append(const char* src) {
            return this->insert(this->size(), src, (int)strlen(src));
        }
        bool append(const void* src, int len) {
            return this->insert(this->size(), src, len);
        }
        bool insert(int idx, const char* src) {
            return this->insert(idx, src, (int)strlen(src));
        }
        bool insert(int idx, const void* src, int len) {
            char* buf;
            if (!this->reserve(idx, len,(const char*&)buf)) {
                return false;
            }
            memmove(buf, src, len);
            return true;


            //MIB_ASSERT(idx >= 0);
            //MIB_ASSERT(len >= 0);
            //auto& buf = this->_buf;
            //auto begin = this->_begin;
            //auto end = this->_end;
            //auto gp = end - begin;
            ////挿入位置と挿入後サイズの確認
            //if (len - gp > 0) {
            //    return false;
            //}



            //if (idx <= begin)
            //{   //begin側挿入
            //    move(begin - idx);
            //    memmove(&buf[idx], src, len);
            //    this->_begin += len;
            //    return true;
            //}
            //if (end < idx + gp) {
            //    //end側挿入
            //    move(-(idx + gp - end));
            //    memmove(&buf[this->_end - len], src, len);
            //    this->_end -= len;
            //    return true;
            //}
            //return false;
        }
        bool reserve(int idx, int len, void*& reserbedbuf) {
            MIB_ASSERT(idx >= 0);
            MIB_ASSERT(len >= 0);
            auto& buf = this->_buf;
            auto begin = this->_begin;
            auto end = this->_end;
            auto gp = end - begin;
            //挿入位置と挿入後サイズの確認
            if (len - gp > 0) {
                return false;
            }
            if (idx <= begin)
            {   //begin側挿入
                move(begin - idx);
                reserbedbuf = &buf[idx];
                this->_begin += len;
                return true;
            }
            if (end < idx + gp) {
                //end側挿入
                move(-(idx + gp - end));
                reserbedbuf = &buf[this->_end - len];
                this->_end -= len;
                return true;
            }
            return false;
        }

        bool remove(int idx, int len) {

            MIB_ASSERT(idx >= 0);
            MIB_ASSERT(len >= 0);

            auto begin = this->_begin;
            auto end = this->_end;
            auto gp = end - begin;
            //操作位置が範囲内にあるか確認
            auto idx_e = idx + len; //終端位置
            if (idx_e > (SIZE - gp)) {
                return false;
            }
            //左辺内
            if (idx_e <= begin) {
                //左辺の残りを移動
                move(begin - idx_e);
                this->_begin = idx;
                return true;
            }
            //右辺内
            if (end - gp <= idx) {
                move(-(gp + idx - end));
                this->_end = gp + idx_e;
                return true;
            }
            //両方にまたがる
            if (idx < begin && end <= idx_e+gp) {
                this->_begin = idx;
                this->_end = gp + idx_e;
                return true;
            }
            //え？
            return false;
        }
    private:
#ifdef TEST

        GapVector(const char* l, const char* r) {
            auto ll = (MIB_INT16)strlen(l);
            auto rl = (MIB_INT16)strlen(r);
            MIB_ASSERT(ll + rl <= SIZE);
            memcpy(&this->_buf[0], l, ll);
            memcpy(&this->_buf[SIZE - rl], r, rl);
            this->_begin = ll;
            this->_end = SIZE - rl;
        }
        /// <summary>
        /// Lバッファ:Rバッファ:gapの文字列。
        /// </summary>
        /// <returns></returns>
        const char* dump(char* buf)
        {
            auto n = sprintf(buf, "%.*s:%.*s:%d",
                this->_begin, &this->_buf[0],
                SIZE - this->_end, &this->_buf[0] + this->_end,
                this->_end - this->_begin
            );
            return buf;
        }
    public:
        static void test() {
            struct local {
                static void append(GapVector<10>& g, const char* text, bool ret, const char* result) {
                    auto r = g.append(text);    //1 9 空に追加 
                    char t1[256];
                    const char* dump = g.dump(t1);
                    printf("[%s] %s %s\n", (strcmp(result, dump) == 0 && r == ret) ? "OK" : "NG", r ? "true" : "false", dump);
                }
                static void insert(GapVector<10>& g, int idx, const char* text, bool ret, const char* result) {
                    char t1[256];
                    auto r = g.insert(idx, text);    //1 9 空に追加 
                    const char* dump = g.dump(t1);
                    printf("[%s] %s %s\n", (strcmp(result, dump) == 0 && r == ret) ? "OK" : "NG", r ? "true" : "false", dump);
                }
                static void remove(GapVector<10>& g, int idx, int len, bool ret, const char* result) {
                    char t1[256];
                    auto r = g.remove(idx, len);    //1 9 空に追加 
                    const char* dump = g.dump(t1);
                    printf("[%s] %s %s\n", (strcmp(result, dump) == 0 && r == ret) ? "OK" : "NG", r ? "true" : "false", dump);
                }
                static void ptr(GapVector<10>& g, int idx, int len, bool ret, const char* result,const char* frgmnt) {
                    char t1[256];
                    const void* out=0;
                    auto r = g.ptr(idx, len,out);    //1 9 空に追加 
                    const char* dump = g.dump(t1);
                    printf("[%s] %s %s\n", (strcmp(result, dump) == 0 && r == ret) ? "OK" : "NG", r ? "true" : "false", dump);
                    if (r) {
                        printf("\t%.*s==%s\n",len,out, frgmnt);
                    }
                }

            };
            {   GapVector<10> g;
            local::insert(g, 0, "1", true, "1::9");    //空に追加
            }
            {   GapVector<10> g("1", "");
            local::insert(g, 1, "23", true, "123::7");   //左辺に単純追記
            }
            {   GapVector<10> g("123", "");
            local::insert(g, 0, "0", true, "0:123:6");   //左辺先頭に挿入に挿入
            }
            {   GapVector<10> g("0123", "");
            local::insert(g, 1, "B", true, "0B:123:5");   //左辺途中に挿入 
            }
            {   GapVector<10> g("0B", "");
            local::insert(g, 1, "AAAAAAAAA", false, "0B::8");  //左辺に過剰挿入 
            }
            {   GapVector<10> g("0B", "");
            local::insert(g, 1, "AAAAAAAA", true, "0AAAAAAAA:B:0");  //左辺に充足挿入
            }

            {   GapVector<10> g("", "0");
            local::insert(g, 0, "1", true, "1:0:8");    //空に追加
            }
            {   GapVector<10> g("1", "0");
            local::insert(g, 1, "23", true, "123:0:6");   //左辺に単純追記
            }
            {   GapVector<10> g("123", "0");
            local::insert(g, 0, "0", true, "0:1230:5");   //左辺先頭に挿入に挿入
            }
            {   GapVector<10> g("0123", "0");
            local::insert(g, 1, "B", true, "0B:1230:4");   //左辺途中に挿入 
            }
            {   GapVector<10> g("0B", "123");
            local::insert(g, 1, "AAAAAA", false, "0B:123:5");  //左辺に過剰挿入 
            }
            {   GapVector<10> g("0B", "123");
            local::insert(g, 1, "AAAAA", true, "0AAAAA:B123:0");  //左辺に充足挿入
            }
            {   GapVector<10> g("1", "2");
            local::insert(g, 2, "AB", true, "12:AB:6");   //右辺に単純追記
            }
            {   GapVector<10> g("123", "45");
            local::insert(g, 3, "0", true, "1230:45:4");   //右辺先頭に挿入に挿入
            }
            {   GapVector<10> g("0123", "456");
            local::insert(g, 5, "B", true, "01234:B56:2");   //右辺途中に挿入 
            }

            {   GapVector<10> g("0B", "12");
            local::insert(g, 3, "AAAAAAA", false, "0B:12:6");  //右辺に過剰挿入 
            }
            {   GapVector<10> g("0B", "12");
            local::insert(g, 2, "AAAAAA", true, "0BAAAAAA:12:0");  //左辺に充足挿入
            }
            {   GapVector<10> g("0B", "12");
            local::insert(g, 1, "AAAAAA", true, "0AAAAAA:B12:0");  //左辺に充足挿入
            }



            {   GapVector<10> g("012", "345");
            local::remove(g, 0, 1, true, ":12345:5");  //左辺内で削除
            }
            {   GapVector<10> g("012", "345");
            local::remove(g, 1, 2, true, "0:345:6");  //左辺内で削除
            }

            {   GapVector<10> g("012", "345");
            local::remove(g, 3, 1, true, "012:45:5");  //右辺内で削除
            }
            {   GapVector<10> g("012", "345");
            local::remove(g, 4, 1, true, "0123:5:5");  //右辺内で削除
            }
            {   GapVector<10> g("012", "345");
            local::remove(g, 4, 2, true, "0123::6");  //右辺内で削除
            }

            {   GapVector<10> g("01234", "56789");
            local::remove(g, 2, 2, true, "01:456789:2");  //右辺内で削除
            }

            {   GapVector<10> g("01234", "56789");
            local::remove(g, 4, 3, true, "0123:789:3");  //境界跨ぎで削除
            }
            {   GapVector<10> g("0123", "6789");
            local::remove(g, 2, 3, true, "01:789:5");  //境界跨ぎで削除
            }
            {   GapVector<10> g("0123", "6789");
            local::remove(g, 0, 5, true, ":789:7");  //境界跨ぎで削除
            }
            {   GapVector<10> g("0123", "6789");
            local::remove(g, 3, 5, true, "012::7");  //境界跨ぎで削除
            }

            //local::insert(g, 6, "DE", true,"0AB123:DE:2");   //0AB123 3 DE    右辺末尾に追加
            //local::insert(g, 7, "F", true,"0AB123D:FE:1");   //0AB123ED 2 FE    右辺中央に追加
            //local::insert(g, 7, "GH", false,"0AB123D:FE:1");   //    領域超過
            //local::insert(g, 2, "G", true, "0AG:B123DFE:0");   //0AG 2 B123EDFE    ラス１

            {   GapVector<10> g("0123", "6789");
                local::ptr(g, 1, 2, true, "0123:6789:2","12");
            }
            {   GapVector<10> g("0123", "6789");
                local::ptr(g, 2, 2, true, "0123:6789:2","23");
            }
            {   GapVector<10> g("0123", "6789");
                local::ptr(g, 4, 2, true, "0123:6789:2","67");
            }
            {   GapVector<10> g("0123", "6789");
                local::ptr(g, 6, 2, true, "0123:6789:2","89");
            }
            {   GapVector<10> g("0123", "6789");
                local::ptr(g, 7, 2, false, "0123:6789:2",NULL);
            }
            {   GapVector<10> g("0123", "6789");
                local::ptr(g, 6, 3, false, "0123:6789:2", NULL);
            }
            {   GapVector<10> g("0123456", "");
                local::ptr(g, 5, 3, false, "0123456::3", NULL);
            }
            //Gapまたぎ
            {   GapVector<10> g("0123", "6789");
                local::ptr(g, 3, 2, true, "01236:789:2","36");
            }
            //Gapまたぎ
            {   GapVector<10> g("01234", "56789");
                local::ptr(g, 2, 5, true, "0123456:789:0","23456");
            }

        }
#endif
    };

}
