/** 行番号をキーにしたメモリ管理クラス

*/

#pragma once
#include "./_linebuf_imports.h"
#include "./GapVector.h"
using namespace MIB;
namespace MIB {

    /// <summary>
    /// 行単位のテキストバッファアクセサです。
    /// 行番号をキーにした編集APIを提供します。
    /// </summary>
    class ILineBuffer {
    public:
        enum class Result {
            OK,
            NG,        //分類不能な一般エラー
            NG_NO_LINE,//行が存在しない
            NG_OOM     //メモリ不足
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
        virtual Result update(MIB_UINT16 line, const char* value, int size) = 0;
    };


    /// <summary>
    /// GapVectorを使った実装です。
    /// 値はブロックヘッダと内容をセットにしたメモリブロックとして、行番号順に格納します。
    /// </summary>
    /// <typeparam name="BUFSIZE"></typeparam>
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
 



    private:
        GapVector<BUFSIZE> _buf;
    protected:
        /// <summary>
        /// targetlineと同じ、もしくは直前のmbhを返す。
        /// 直前が存在しない場合は、index=start_index,mbh=NULL
        /// これはメモリが空か、target_lineが既存の者より若い場合である。
        /// 直前の有無はmbhのNULLで判定して。
        /// </summary>
        /// <param name="target_line"></param>
        /// <param name="index"></param>
        /// <param name="mbh"></param>
        /// <returns></returns>
        bool findEqualLessMbh(MIB_UINT16 target_line,int& index, struct MemBlockHeader* &mbh,int start_index=0)
        {
            struct MemBlockHeader* mbh_tmp = NULL;
            //初期値設定
            int r_index=start_index;
            struct MemBlockHeader* r_mbh = NULL;
            //直前の候補点から探索    
            for (int idx = start_index;;)
            {
                if (idx >= this->_buf.size()) {
                    //バッファ位置超過
                    break;
                }
                if (!this->_buf.ptr(idx, 3, (void*&)mbh_tmp)) {
                    return false;
                }
                if (mbh_tmp->line() <= target_line) {
                    //line内
                    r_index = idx;
                    r_mbh = mbh_tmp;
                    if (mbh_tmp->line() == target_line) {
                        break;
                    }
                    idx += 3 + mbh_tmp->size;
                }
                else {
                    //探索位置超過
                    break;
                }
            }
            //終端
            index = r_index;
            mbh = r_mbh;
            return true;
        }


    public:
        Result read(MIB_UINT16 line, const char*& buf, int& size)
        {
            struct MemBlockHeader* mbh_tmp = NULL;
            int index = 0;
            int s = 0;
            //メモリブロックのインデクスを得る
            if (!this->findEqualLessMbh(line, index, mbh_tmp,0))
            {   //API失敗
                return Result::NG_NO_LINE;
            }
            if ((index==0 && mbh_tmp==NULL)||mbh_tmp->line()!=line)
            {   //lineが一致しない場合
                return Result::NG_NO_LINE;
            }
            char* t_buf = NULL;
            //データの参照ポインタ取得
            if (!this->_buf.ptr(index + 3, mbh_tmp->size, (void*&)t_buf)) {
                return Result::NG_NO_LINE;
            }
            buf = (const char*)t_buf;
            size = s;
            return Result::OK;
        }

        /// <summary>
        /// 指定した行を削除します。
        /// </summary>
        /// <returns></returns>
        Result remove(MIB_UINT16 line) {
            struct MemBlockHeader* mbh_tmp = NULL;
            int index=0;
            if (!this->findEqualLessMbh(line, index, mbh_tmp,0)) {
                return Result::NG;
            }
            if ((index == 0 && mbh_tmp == NULL) || mbh_tmp->line() != line)
            {   //lineが一致しない場合
                return Result::NG_NO_LINE;
            }
            //実体を削除
            if (!this->_buf.remove(index, mbh_tmp->size+3)) {
                return Result::NG;
            }
            return Result::OK;
        }

        /// <summary>
        /// 行を差し替えます。
        /// </summary>
        /// <param name="line"></param>
        /// <param name="buf"></param>
        /// <param name="size"></param>
        /// <returns></returns>
        virtual Result update(MIB_UINT16 line, const char* value, int size)
        {
            struct MemBlockHeader* mbh = NULL;
            int index = 0;
            int start = 0;
            if (!this->findEqualLessMbh(line, index, mbh, 0))
            {   //APIエラー
                return Result::NG;
            }
            if (mbh == NULL)
            {   //先頭に挿入
                MIB_INT8* mem = NULL;
                if (!this->_buf.reserve(0, size + 3, (void*&)mem)) {
                    return Result::NG_OOM;
                }
                ((struct MemBlockHeader*)mem)->set(line, (MIB_UINT8)size);
                memmove(mem + 3, buf, size);
            }else if (mbh->line() == line)
            {   //既存行に対する操作
                int d = size-mbh->size;//新サイズ-旧サイズ
                //読みだせたら更新
                if (d == 0) {
                    //同一サイズ
                    MIB_INT8* mem = NULL;
                    //データ部分のポインタに値をコピー
                    if (!this->_buf.ptr(index + 3, mbh->size, (void*&)mem))
                    {
                        return Result::NG;
                    }
                    memmove(mem, buf, size);
                }
                else if (d < 0) {
                    //縮小
                    if (!this->_buf.remove(index, -d)) {
                        return Result::NG;//It must not fail
                    }
                    MIB_INT8* mem = NULL;
                    if (!this->_buf.ptr(index, size+3, (void*&)mem))
                    {
                        return Result::NG;
                    }
                    ((struct MemBlockHeader*)mem)->set(line, (MIB_UINT8)size);
                    memmove(mem + 3, buf, size);
                    //
                }
                else {//d>0
                    //拡大
                    MIB_INT8* mem = NULL;
                    //領域の挿入
                    if (!this->_buf.reserve(index, d,(void*&)mem)) {
                        return Result::NG_OOM;
                    }
                    //連続した領域を確保
                    if (!this->_buf.ptr(index, size+3, (void*&)mem))
                    {
                        return Result::NG;
                    }
                    ((struct MemBlockHeader*)mem)->set(line, (MIB_UINT8)size);
                    memmove(mem + 3, buf, size);
                    //
                }
            }else
            {   //LINEが一致しない場合
                MIB_ASSERT(mbh->line()< line);
                void* mem = NULL;
                //手前の直後に領域を予約していれとこ
                if (!this->_buf.reserve(index + 3 + mbh->size, size + 3, (void*&)mem)) {
                    return Result::NG_OOM;
                }
                ((struct MemBlockHeader*)mem)->set(line, (MIB_UINT8)size);
                memmove((MIB_INT8*)mem+3, buf, size);
            }
            return Result::OK;
        }
        /// <summary>
        /// bufidxにある行を読出し、次の行のbufidxを返す。
        /// textは連続したメモリであることを補償します。
        /// @bug あれこれ終端検知どうなってるの？bufidxがメモリサイズより大きくなったときに下位がエラーを返すことを当てにしてる？
        /// -1でエラー。
        /// </summary>
        int iter(int bufidx, MIB_UINT16& line, const char*& text, int& size)
        {
            struct MemBlockHeader* mbh = NULL;
            if (bufidx >= this->_buf.size()) {
                return -1;
            }
            //ヘッダの読出し
            if (!this->_buf.ptr(bufidx, 3, (void*&)mbh)) {
                return -1;
            }
            //本体の読み出し
            if (!this->_buf.ptr(bufidx + 3, mbh->size, (void*&)text)) {
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