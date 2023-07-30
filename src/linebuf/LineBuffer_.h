#pragma once
#include "./_linebuf_imports.h"
#include "./GapVector.h"
using namespace MIB;
namespace MIB {

    /// <summary>
    /// 行単位のテキストバッファアクセサです。
    /// </summary>
    class ILineBuffer {
    public:
        enum class Result {
            OK,
            ERR,
            ERR_NO_LINE,
            ERR_OOM
        };
    public:
        /// <summary>
        /// 指定した行の文字列を返します。
        /// </summary>
        /// <param name="line"></param>
        /// <param name="text"></param>
        /// <param name="size"></param>
        /// <returns></returns>
        virtual Result read(MIB_UINT16 line, const char*& buf, int& size) = 0;
        /// <summary>
    /// 指定した行を削除します。
    /// </summary>
    /// <returns></returns>
        virtual Result remove(MIB_UINT16 line) = 0;

        /// <summary>
        /// 行を差し替えます。
        /// </summary>
        /// <param name="line"></param>
        /// <param name="buf"></param>
        /// <param name="size"></param>
        /// <returns></returns>
        virtual Result update(MIB_UINT16 line, const char* buf, int size) = 0;
    };


    template<int BUFSIZE = 64> class LineBuffer :public ILineBuffer {
    private:
        /// <summary>
        /// メモリブロックの参照型。1バイト境界で読みだされるのでメンバ変数はUINT8であること。
        /// マジックナンバーで3を使ってるから注意な！
        /// </summary>
        struct MemBlockHeader {
            MIB_UINT8 line_l; //ライン番号L
            MIB_UINT8 line_h; //ライン番号H
            MIB_UINT8 size;   //データサイズ(ヘッダ長含まず)
            void set(MIB_UINT16 l, MIB_UINT8 s) {
                this->size = s;
                this->line_l = (MIB_UINT8)(l & 0xff);
                this->line_h = (MIB_UINT8)((l >> 8) & 0xff);
            }
            int line()const {
                return (((int)this->line_h) << 8) | this->line_l;
            }
        };
    private:
        template <int TBLSIZE = 8> class CacheTable {
        public:
            struct Item {
                MIB_UINT16 line;  //行番号
                MIB_UINT16 index; //バッファ上のオフセット
            } items[TBLSIZE] = {};      //昇順追記
            MIB_INT8 start = 0;   //テーブルの先頭位置
        public:
            /// <summary>
            /// lineをキーにして、lineを超えない最もlineに近いアイテムを返す。
            /// </summary>
            const struct Item* getEqualLess(MIB_UINT16 line)const {
                auto& items = this->items;
                const struct Item* nearest = NULL;

                //昇順探索。0は無効値
                for (auto i = 0;i < TBLSIZE;i++) {
                    auto p = (i + this->start) % TBLSIZE;
                    auto tline = items[p].line;
                    if (tline == line) {
                        return &items[p];
                    }
                    if (tline < line) {
                        if (nearest == NULL || (nearest->line < tline)) {
                            nearest = &items[p];
                        }
                    }
                    if (tline == 0) {
                        break;
                    }
                }
                return nearest;
            }
            /// <summary>
            /// lineが存在しなければテーブルに追記する。
            /// </summary>
            /// <typeparam name="CTS"></typeparam>
            void pushIfNotExist(MIB_UINT16 line, MIB_UINT16 index) {
                auto& items = this->items;
                auto el = this->getEqualLess(line);
                if (el != NULL && el->line == line) {
                    return;
                }
                //未登録なら逆順で登録
                auto nidx = (this->start + TBLSIZE - 1) % TBLSIZE;
                this->start = (MIB_INT8)nidx;
                items[nidx].line = line;
                items[nidx].index = index;
            }
            void clear() {
                memset(this->items, 0, sizeof(items));
            }
#ifdef TEST
        public:
            const char* dump() {
                static char t[256];
                int p = 0;
                for (int i = 0;i < TBLSIZE;i++) {
                    p = p + sprintf(&t[p], "[%d,%d]", this->items[i].line, this->items[i].index);
                }
                return t;
            }
            static void test() {
                {
                    CacheTable<3> inst;
                    inst.pushIfNotExist(1, 1);
                    printf("%s\n", inst.dump());
                    auto b = inst.getEqualLess(1);
                    MIB_ASSERT(b->line == 1 && b->index == 1);   //一致
                    auto c = inst.getEqualLess(2);
                    MIB_ASSERT(b->line == 1 && b->index == 1);//直前
                }
                {
                    CacheTable<3> inst;
                    inst.pushIfNotExist(2, 2);
                    printf("%s\n", inst.dump());
                    auto b = inst.getEqualLess(1);
                    MIB_ASSERT(b == NULL);//なし
                }
                {
                    CacheTable<3> inst;
                    inst.pushIfNotExist(2, 2);
                    inst.pushIfNotExist(3, 3);
                    inst.pushIfNotExist(4, 4);
                    printf("%s\n", inst.dump());
                    auto b = inst.getEqualLess(0);
                    MIB_ASSERT(b == NULL);//なし
                }
            }
#endif
        };
        CacheTable<> ct;
#ifdef TEST
    public:
        static void test_CacheTable() {
            CacheTable<>::test();
        }
        static void test() {
            struct local {
                static void read(LineBuffer<20>& g, int line, Result ret, const char* text, int size) {
                    const char* b = NULL;
                    int s = 0;

                    auto r = g.read(line, b, s);    //1 9 空に追加 
                    auto f = r == Result::OK ? (memcmp(text, b, s) == 0 && s == size && ret == r) : (ret == r);

                    if (f) {
                        if (r == Result::OK) { printf("[OK] true %s %d\n", text, size); }
                        else { printf("[OK]read   false\n"); }
                    }
                    else {
                        if (r == Result::OK) { printf("[NG] %d->%d %s->%s %d->%d\n", ret, r, text, b, size, s); }
                        else { printf("[NG]read   false\n"); }
                    }
                }
                static void update(LineBuffer<20>& g, int line, const char* text, Result ret) {
                    const char* b = NULL;
                    int s = 0;

                    auto r = g.update(line, text, strlen(text));    //1 9 空に追加 
                    auto f = (ret == r);
                    printf("[%s]update  %d\n", (ret == r) ? "OK" : "NG", r);
                }

                static void remove(LineBuffer<20>& g, int line, Result ret) {
                    const char* b = NULL;
                    int s = 0;

                    auto r = g.remove(line);    //1 9 空に追加 
                    auto f = (ret == r);
                    printf("[%s]remove  %d\n", (ret == r) ? "OK" : "NG", r);
                }
                static void buftest(LineBuffer<20>& g, const char* testpatt) {
                    static char tmp[256];

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
                local::read(inst, 1, Result::ERR_NO_LINE, NULL, -1);
                local::remove(inst, 1, Result::ERR_NO_LINE);
            }
            {
                LineBuffer<20> inst;
                local::update(inst, 1, "11", Result::OK);        //新規追加
                local::buftest(inst, "1:11,");
                local::update(inst, 2, "22", Result::OK);        //追記
                local::buftest(inst, "1:11,2:22,");
                local::update(inst, 5, "55", Result::OK);        //挿入
                local::buftest(inst, "1:11,2:22,5:55,");
                local::update(inst, 3, "33", Result::OK);        //臨界挿入
                local::buftest(inst, "1:11,2:22,3:33,5:55,");
                local::update(inst, 6, "A", Result::ERR_OOM);    //OOM
                local::remove(inst, 2, Result::OK);                 //未定義削除
                local::buftest(inst, "1:11,3:33,5:55,");
                local::remove(inst, 2, Result::ERR_NO_LINE);
                local::buftest(inst, "1:11,3:33,5:55,");
                local::update(inst, 3, "3333", Result::OK);        //定義済み更新1
                local::buftest(inst, "1:11,3:3333,5:55,");
                local::update(inst, 3, "3", Result::OK);        //定義済み更新2
                local::buftest(inst, "1:11,3:3,5:55,");
                local::update(inst, 3, "33333333", Result::ERR_OOM);//定義済み更新3(OOM)
                local::buftest(inst, "1:11,3:3,5:55,");
                local::remove(inst, 1, Result::OK);
                local::remove(inst, 3, Result::OK);
                local::buftest(inst, "5:55,");
                local::update(inst, 0, "000", Result::OK);
                local::buftest(inst, "0:000,5:55,");
                local::update(inst, 9, "99", Result::OK);
                local::buftest(inst, "0:000,5:55,9:99,");
                local::remove(inst, 9, Result::OK);
                local::buftest(inst, "0:000,5:55,");
                local::update(inst, 5, "", Result::OK);
                local::buftest(inst, "0:000,5:,");
                local::update(inst, 0, "0", Result::OK);
                local::update(inst, 1, "1", Result::OK);
                local::update(inst, 4, "4", Result::OK);
                local::update(inst, 3, "3", Result::OK);
                local::buftest(inst, "0:0,1:1,3:3,4:4,5:,");
                local::remove(inst, 4, Result::OK);
                local::buftest(inst, "0:0,1:1,3:3,5:,");

            }



        }

#endif

    private:
        GapVector<BUFSIZE> _buf;

        /// <summary>
        /// ブロックヘッダのインデクスと、サイズ(ヘッダを含む)を返します。
        /// </summary>
        /// <param name="line"></param>
        /// <param name="mbh"></param>
        /// <returns></returns>
        bool _indexOfBlockHeader(MIB_UINT16 line, int& index, int& size)
        {
            const struct MemBlockHeader* mbh;
            int start = 0;

            //キャッシュから取得
            auto item = this->ct.getEqualLess(line);
            if (item != NULL) {
                if (item->line == line)
                {   //一致した場合
                    index = item->index;
                    if (!this->_buf.ptr(index, 3, (const void*&)mbh)) {
                        return false;
                    }
                    size = 3 + mbh->size;
                    return true;
                }
                else
                {   //一致しない場合はlessthan行から探索
                    MIB_ASSERT(item->line < line);
                    //開始点を指定
                    start = item->index;
                }
            }
            //ブロックヘッダの探索            
            for (int idx = start;this->_buf.ptr(idx, 3, (const void*&)mbh);) {
                auto target = mbh->line();
                if (target < line) {
                    idx += 3 + mbh->size;
                    continue;
                }
                if (target == line) {
                    //found
                    index = idx;
                    size = 3 + mbh->size;
                    return true;
                }
                break;
            }
            return false;
        }
    public:
        Result read(MIB_UINT16 line, const char*& buf, int& size) {
            int index;
            int s;
            const void* t_buf;
            if (!this->_indexOfBlockHeader(line, index, s)) {
                return Result::ERR_NO_LINE;
            }
            if (!this->_buf.ptr(index + 3, s - 3, t_buf)) {
                return Result::ERR_NO_LINE;
            }
            //登録
            this->ct.pushIfNotExist(line, index);
            buf = (const char*)t_buf;
            size = s;
            return Result::OK;
        }

        /// <summary>
        /// 指定した行を削除します。
        /// </summary>
        /// <returns></returns>
        Result remove(MIB_UINT16 line) {
            int index;
            int s;
            if (!this->_indexOfBlockHeader(line, index, s)) {
                return Result::ERR_NO_LINE;
            }
            if (!this->_buf.remove(index, s)) {
                return Result::ERR_NO_LINE;
            }
            this->ct.clear();
            return Result::OK;
        }

        /// <summary>
        /// 行を差し替えます。
        /// </summary>
        /// <param name="line"></param>
        /// <param name="buf"></param>
        /// <param name="size"></param>
        /// <returns></returns>
        Result update(MIB_UINT16 line, const char* buf, int size) {
            int start = 0;

            //キャッシュから取得
            auto item = this->ct.getEqualLess(line);
            if (item != NULL) {
                start = item->index;
            }
            //ブロックヘッダの探索            
            const struct MemBlockHeader* mbh = NULL;
            int idx = 0;
            for (idx = start;this->_buf.ptr(idx, 3, (const void*&)mbh);) {
                if (mbh->line() < line) {
                    idx += 3 + mbh->size;
                    continue;
                }
                break;
            }
            if (mbh == NULL) {
                MIB_UINT8* out;
                //ブロックヘッダが見つからない場合
                if (!this->_buf.reserve(idx, size + 3, (void*&)out)) {
                    return Result::ERR;
                }
                ((struct MemBlockHeader*)out)->set(line, (MIB_UINT8)size);
                memmove(out + 3, buf, size);
            }
            else if (mbh->line() == line) {
                //既存の行あり
                if (this->_buf.freeSize() + mbh->size - size < 0) {
                    return Result::ERR_OOM;
                }
                if (!this->_buf.remove(idx, mbh->size + 3)) {
                    return Result::ERR;
                }
                MIB_UINT8* out;
                if (!this->_buf.reserve(idx, size + 3, (void*&)out)) {
                    return Result::ERR;
                }
                ((struct MemBlockHeader*)out)->set(line, (MIB_UINT8)size);
                memmove(out + 3, buf, size);
            }
            else {
                MIB_UINT8* out;
                //手前に入れとく
                if (!this->_buf.reserve(idx, size + 3, (void*&)out)) {
                    return Result::ERR_OOM;
                }
                ((struct MemBlockHeader*)out)->set(line, (MIB_UINT8)size);
                memmove((out)+3, buf, size);
            }
            this->ct.clear();
            this->ct.pushIfNotExist(line, idx);
            return Result::OK;
        }
        /// <summary>
        /// bufidxにある行を読出し、次の行のbufidxを返す。
        /// -1でエラー。
        /// </summary>
        int iter(int bufidx, MIB_UINT16& line, const char*& text, int& size)
        {
            const struct MemBlockHeader* mbh = NULL;
            if (!this->_buf.ptr(bufidx, 3, (const void*&)mbh)) {
                return -1;
            }
            if (!this->_buf.ptr(bufidx + 3, mbh->size, (const void*&)text)) {
                return -1;
            }
            line = mbh->line();
            size = mbh->size;
            return bufidx + 3 + mbh->size;
        }
        int freeSize()const {
            return this->_buf.freeSize();
        }
    };
}