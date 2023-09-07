/** 名前をキーにしたメモリ管理クラス
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
    class IDictBuffer {
    public:
        enum class Result {
            OK,
            NG,        //分類不能な一般エラー
            NG_NO_KEYWORD,//行が存在しない
            NG_OOM     //メモリ不足
        };
    public:
        const static unsigned char TYPE_INT32 = 1;
        const static unsigned char TYPE_SHORT_TEXT = 2;
        const static unsigned char TYPE_BOOL_TRUE = 0;
        const static unsigned char TYPE_BOOL_FALSE = 7;
    public:
        const static int MemBlock_HEADER_SIZE = 2;
        const static int MemBlock_SHORT_STR_HEADER_SIZE = MemBlock_HEADER_SIZE+1;
        /// <summary>
        //  先頭2バイトは管理情報。ハッシュ値とデータ型,を持つ。
        /// メモリブロックの参照型。1バイト境界で読みだされるのでメンバ変数はUINT8であること。
        /// 先頭2バイトに属性情報,後続に名前と値を持つ。
        /// 値型を示すtype値により、データブロック全体の長さの計算式は異なる。
        /// TYPE_SHORT_TEXT:    2+name_len+(UINT8)data0 + 1      //data0に、[name_len]の名前、1バイトの後続データの長さ、最大255バイトの後続データ
        /// TYPE_INT32:    2+name_len+4                          //data0に、[name_len]の名前、4バイトの値
        /// TYPE_BOOL:    2+name_len                             //data0に8ビットの後続データの長さ
        /// </summary>
        struct MemBlock {
            MIB_UINT8 hash; //値のハッシュ値
            MIB_UINT8 type : 3;    //後続データ型
            MIB_UINT8 name_len : 5;//名前長
            MIB_UINT8 data0;    //後続データの先頭
        public:
            bool asInt32(int& dest)const {
                if (this->type != TYPE_INT32) {
                    return false;
                }
                dest=bytes2int(&this->data0 + this->name_len);
                return true;
            }
            bool asText(const char*& dest, int& dest_len)const {
                if (this->type != TYPE_SHORT_TEXT) {
                    return false;
                }
                dest=(const char*)(& this->data0) + this->name_len + 1;
                dest_len = *((MIB_UINT8*)(&this->data0) + this->name_len);
                return true;
            }
            bool asBool(bool& dest)const {
                if (this->type == TYPE_BOOL_TRUE) {
                    dest = true;
                    return true;
                }
                if (this->type == TYPE_BOOL_FALSE) {
                    dest = false;
                    return true;
                }
                return false;
            }
        };

    public:
        /// <summary>
        /// 名前に一致する値を格納したメモリブロックを返す。
        /// </summary>
        /// <param name="name"></param>
        /// <param name="name_len"></param>
        /// <param name="mb"></param>
        /// <returns></returns>
        virtual Result read(const char* name, int name_len, const struct MemBlock*& mb) = 0;

        /// <summary>
        /// 指定した行を削除します。
        /// </summary>
        /// <returns></returns>
        virtual Result remove(const char* name, int name_len) = 0;
        virtual Result update(const char* name, int name_len, int value) = 0;
        virtual Result update(const char* name, int name_len, bool value) = 0;
        virtual Result update(const char* name, int name_len, const char* value, int size) = 0;

    public:
        Result read(const char* name, const struct MemBlock*& mb)
        {
            return this->read(name, strlen(name), mb);
        }
        Result remove(const char* name)
        {
            return this->remove(name, strlen(name));
        }


    };


    /// <summary>
    /// GapVectorを使ったDictです。
    /// 格納型は、bool,text,int32です。
    /// 
    /// 値はブロックヘッダと内容をセットにしたメモリブロックとして、ハッシュ値順に格納します。
    /// ハッシュ値はnameから計算した重複の可能性のある値です。
    /// メモリブロックには、ハッシュ値、値型、名前サイズ、名前、値を順に格納します。
    /// </summary>
    /// <typeparam name="BUFSIZE"></typeparam>
    template<int BUFSIZE = 64> class DictBuffer :public IDictBuffer {
    public:
        virtual ~DictBuffer() {
        }

    private:
        GapVector<BUFSIZE> _buf;
        bool isValidHeader(int type, int size) {

        }
    protected:
        MIB_UINT8 toHash(const char* value, int len) {
            return *value;//先頭位置文字
        }
    protected:
        /// <summary>
        /// indexをメモリブロックの先頭と仮定し、メモリの再配置無しでメモリブロックの値を得る。
        /// </summary>
        /// <param name="index"></param>
        /// <param name="size"></param>
        /// <returns></returns>
        bool getMemBlockSize(int index,int& size)
        {
            struct MemBlock tmp = {};
            //先頭を取得
            if (MemBlock_HEADER_SIZE != this->_buf.gets(index, MemBlock_HEADER_SIZE, (MIB_UINT8*)&tmp)) {
                return false;
            }
            //型に応じて追加情報を取得してサイズを計算
            switch (tmp.type)
            {
            case TYPE_INT32:
                size = MemBlock_HEADER_SIZE + tmp.name_len + 4;
                break;
            case TYPE_SHORT_TEXT:
            {
                auto s = this->_buf.get(index + MemBlock_HEADER_SIZE + tmp.name_len);
                if (s < 0) {
                    return false;
                }
                size = MemBlock_HEADER_SIZE + tmp.name_len + 1+s;
                break;
            }
            case TYPE_BOOL_TRUE:
            case TYPE_BOOL_FALSE:
                size = MemBlock_HEADER_SIZE + tmp.name_len;
                break;
            default:
                return false;
            }
            return true;
        }
        /// <summary>
        /// indexをメモリブロック先頭と仮定して、キーワード名を読み出し可能なポインタを返す。
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        bool getKeywordPtr(int index,const char*& kwd,int& len)
        {
            struct MemBlock tmp = {};
            //先頭を取得
            if (MemBlock_HEADER_SIZE != this->_buf.gets(index, MemBlock_HEADER_SIZE, (MIB_UINT8*)&tmp)) {
                return false;
            }
            if (!this->_buf.ptr(index + MemBlock_HEADER_SIZE, tmp.name_len,(void*&)kwd)) {
                return false;
            }
            len = tmp.name_len;
            return true;

        }


        /// <summary>
        /// 昇順に並ぶhashメモリブロックから、
        /// target_hashと一致、又は小さいhashの先頭にあるメモリブロックのヘッダのインデクスを得る。
        /// 直前、または一致が存在しない場合,index=-1でtrueを返す。
        /// ハッシュブロックの読取操作のエントリポイントの計算に使う。
        /// </summary>
        /// <param name="target_hash"></param>
        /// <param name="index"></param>
        /// <param name="mbh"></param>
        /// <param name="start_index"></param>
        /// <returns></returns>
        bool findLessThanHashTop(MIB_UINT8 target_hash, int& index,int start_index = 0)
        {
            //初期値設定
            int last_index = -1;
            struct MemBlock mb_head = {}; //r_index>-1の場合、blocksize読み出し可能なヘッダのコピー
            //直前の候補点から探索    
            for (int idx = start_index;;)
            {
                if (idx >= this->_buf.size()) {
                    //バッファ位置超過
                    break;
                }
                int read_size = this->_buf.gets(idx,MemBlock_HEADER_SIZE, (MIB_UINT8*)&mb_head);//3か2
                if (read_size < MemBlock_HEADER_SIZE) {
                    return false;
                }
                if (mb_head.hash <= target_hash) {
                    //line内
                    last_index = idx;
                    if (mb_head.hash == target_hash) {
                        //一致
                        break;
                    }
                    int last_mbsize = 0;
                    if (!this->getMemBlockSize(idx, last_mbsize)) {
                        return false;
                    }
                    idx += last_mbsize;
                }
                else {
                    //探索位置超過
                    break;
                }
            }
            if (last_index == -1)
            {   //未発見
                index = -1;
            }
            else {
                //発見
                index = last_index;
            }
            return true;
        }
        /// <summary>
        /// 昇順に並ぶhashメモリブロックから、
        /// target_hashよりも大きいhashの先頭にあるメモリブロックのヘッダのインデクスを得る。
        /// 直前、または一致が存在しない場合,index=-1でtrueを返す。
        /// ハッシュブロックの書込み操作のエントリポイントの計算に使う。
        /// </summary>
        /// <param name="target_hash"></param>
        /// <param name="index"></param>
        /// <param name="mbh"></param>
        /// <param name="start_index"></param>
        /// <returns></returns>
        bool findGreaterHashTop(MIB_UINT8 target_hash, int& index, int start_index = 0)
        {
            //初期値設定
            int last_index = -1;
            struct MemBlock mb_head = {}; //r_index>-1の場合、blocksize読み出し可能なヘッダのコピー
            //直前の候補点から探索    
            for (int idx = start_index;;)
            {
                if (idx >= this->_buf.size()) {
                    //バッファ位置超過
                    break;
                }
                int read_size = this->_buf.gets(idx, MemBlock_HEADER_SIZE, (MIB_UINT8*)&mb_head);//3か2
                if (read_size < MemBlock_HEADER_SIZE) {
                    return false;
                }
                if (mb_head.hash <= target_hash) {
                    //line内
                    int last_mbsize = 0;
                    if (!this->getMemBlockSize(idx, last_mbsize)) {
                        return false;
                    }
                    idx += last_mbsize;
                    last_index = idx;//次の位置
                }
                else {
                    //探索位置超過
                    break;
                }
            }
            if (last_index == -1)
            {   //未発見
                index = -1;
            }
            else {
                //発見
                index = last_index;
            }
            return true;
        }






        /// <summary>
        /// nameに一致する値を格納するmbhを返す。
        /// 存在しない場合は、index=-1,mbh=NULL
        /// 発見できた場合、mbにはデータブロック全体を参照できるポインタを返す。
        /// 直前の有無はindexの-1で判定して。
        /// </summary>
        /// <param name="target_line"></param>
        /// <param name="index"></param>
        /// <param name="mbh"></param>
        /// <returns></returns>
        bool findByName(const char* name, int name_len, int& index, struct MemBlock*& mb, int start_index = 0)
        {
            int tmp_index = 0;
            //ハッシュ探索
            MIB_UINT8 target_hash = toHash(name, name_len);
            if (!this->findLessThanHashTop(target_hash, tmp_index, start_index)) {
                return false;
            }
            //一致するハッシュはなかった
            if (tmp_index ==-1) {
                index = -1;
                mb = NULL;
                return true;
            }
            //直前の候補点から探索    
            for (int idx = tmp_index;;)
            {
                //この時点でtmp_indexのハッシュ一致済であること

                //キーワード名を取得
                const char* kwd=NULL;
                int kwd_len;
                if (!this->getKeywordPtr(idx, kwd, kwd_len)) {
                    return false;
                }
                //名前一致検査
                if (kwd_len == name_len && memcmp(kwd, name, name_len) == 0) {
//                    struct MemBlock* mb_tmp;
                    //ここで内容を確定

                    //メモリブロックのサイズを得る
                    int s = 0;
                    if (!this->getMemBlockSize(idx, s)) {
                        return false;
                    }
                    //メモリブロックを取得
                    if (!this->_buf.ptr(idx, s, (void*&)mb)) {
                        return false;
                    }
                    index = idx;
                    return true;
                }

                //次のヘッダを探す

                //スキップのためにブロックサイズを計算
                int blocksize = 0;
                if (!this->getMemBlockSize(idx, blocksize)) {
                    return false;
                }
                idx += blocksize;
                //終端チェック
                if (idx >= this->_buf.size()) {
                    //終端→未発見
                    index = -1;
                    mb = NULL;
                    return true;
                }
                struct MemBlock mb_head = {};
                int read_size = this->_buf.gets(idx, MemBlock_HEADER_SIZE,(MIB_UINT8*) & mb_head);
                if (read_size < MemBlock_HEADER_SIZE) {
                    return false;
                }
                if (mb_head.hash != target_hash) {
                    //ハッシュが違う。終端→未発見
                    index = -1;
                    mb = NULL;
                    return true;
                }
            }
        }

    protected:
        /// <summary>
        /// mb_destに書込み領域を用意します。
        /// 書込み領域は、size+2(ヘッダ)+name_lenのサイズです。
        /// data0+name_lenから先にsizeの領域が確保されます。
        /// この関数は元の値を破壊します。
        /// </summary>
        /// <param name="line"></param>
        /// <param name="buf"></param>
        /// <param name="size"></param>
        /// <returns></returns>
        Result reserve(const char* name, int name_len, int size, MIB_UINT8 data_type,struct MemBlock*& mb_dest)
        {
            struct MemBlock* mb_tmp = NULL;
            int index_tmp = 0;
            int start_index = 0;

            if (!this->findByName(name, name_len, index_tmp, mb_tmp, start_index))
            {   //APIエラー
                return Result::NG_NO_KEYWORD;
            }
            int new_blocksize = (MemBlock_HEADER_SIZE + name_len+size);//新しいデータブロックのサイズ


            if (index_tmp ==-1)
            {   //ハッシュを計算してそのハッシュの先頭に挿入
                MIB_UINT8 target_hash = toHash(name, name_len);
                //初期値設定
                //More
                if (!this->findGreaterHashTop(target_hash, index_tmp, start_index)) {
                    return Result::NG;
                }
                //同一ハッシュ値ブロックの後端迄走査。
                if (index_tmp < 0) {
                    index_tmp = start_index;
                }
                if (!this->_buf.reserve(index_tmp, new_blocksize, (void*&)mb_dest)) {
                    return Result::NG_OOM;
                }
                //ハッシュ,値名、サイズを書き込み
                mb_dest->hash = target_hash;
                mb_dest->name_len = name_len;
                memmove((&mb_dest->data0), name, name_len);
            }
            else
            {   //既存行(hashと名前の一致する行)に対する操作
                MIB_ASSERT(name_len == mb_tmp->name_len);
                MIB_ASSERT(memcmp(name, &mb_tmp->data0, name_len) == 0);

                int d = 0;
                if (!this->getMemBlockSize(index_tmp, d)) {
                    return Result::NG;
                }
                d = new_blocksize - d;//新サイズ-旧サイズ
                //読みだせたら更新
                if (d == 0)
                {
                    //メモリ領域の変更はなし
                }
                else if (d < 0)
                {   //縮小 名前の後ろのデータ領域のみ
                    if (!this->_buf.remove(index_tmp + MemBlock_HEADER_SIZE + mb_tmp->name_len, -d)) {
                        return Result::NG;//It must not fail
                    }
                }
                else
                {   //d>0 拡大
                    //名前の後ろに領域の挿入
                    if (!this->_buf.reserve(index_tmp + MemBlock_HEADER_SIZE + mb_tmp->name_len, d, (void*&)mb_tmp)) {
                        return Result::NG_OOM;
                    }
                }
                //連続した領域を確保
                if (!this->_buf.ptr(index_tmp, new_blocksize, (void*&)mb_dest))
                {
                    return Result::NG;
                }
            }
            //名前チェック
            MIB_ASSERT(mb_dest->name_len == name_len);
            MIB_ASSERT(memcmp(name, &mb_dest->data0, name_len) == 0);
            //型の書込み
            mb_dest->type = data_type;

            return Result::OK;
        }
    public:
        Result read(const char* name, const struct MemBlock*& mb)
        {
            return this->read(name, strlen(name), mb);
        }
        Result remove(const char* name)
        {
            return this->remove(name, strlen(name));
        }
        Result update(const char* name, int value) {
            return this->update(name, strlen(name), value);
        }
        Result update(const char* name, bool value) {
            return this->update(name, strlen(name), value);
        }
        Result update(const char* name, const char* value, int size) {
            return this->update(name, strlen(name), value,size);
        }
        /// <summary>
        /// 名前に一致する値を格納したメモリブロックを返す。
        /// </summary>
        /// <param name="name"></param>
        /// <param name="name_len"></param>
        /// <param name="mb"></param>
        /// <returns></returns>
        Result read(const char* name, int name_len, const struct MemBlock*& mb)
        {
            struct MemBlock* mb_tmp = NULL;
            int index = 0;
            int s = 0;
            //メモリブロックのインデクスを得る
            if (!this->findByName(name, name_len, index, mb_tmp,0))
            {   //API失敗
                return Result::NG_NO_KEYWORD;
            }
            if (index<0)
            {   //見つからない場合
                return Result::NG_NO_KEYWORD;
            }
            mb = mb_tmp;
            return Result::OK;
        }
        /// <summary>
        /// 指定した行を削除します。
        /// </summary>
        /// <returns></returns>
        Result remove(const char* name, int name_len)
        {
            struct MemBlock* mb_tmp = NULL;
            int index = 0;
            int s = 0;
            //メモリブロックのインデクスを得る
            if (!this->findByName(name, name_len, index, mb_tmp, 0))
            {   //API失敗
                return Result::NG_NO_KEYWORD;
            }
            if (index < 0)
            {   //lineが一致しない場合
                return Result::NG_NO_KEYWORD;
            }
            //実体を削除
            int bs = 0;
            if (!this->getMemBlockSize(index, bs)) {
                return Result::NG;
            }
            if (!this->_buf.remove(index,bs)) {
                return Result::NG_NO_KEYWORD;
            }
            return Result::OK;
        }

        Result update(const char* name, int name_len, int value) {
            struct MemBlock* mb_tmp =NULL;
            auto r = this->reserve(name, name_len, 4, TYPE_INT32,mb_tmp);
            if (r != Result::OK) {
                return r;
            }
            int2bytes(value, (&mb_tmp->data0) + name_len);
            return Result::OK;

        }
        Result update(const char* name, int name_len, bool value)
        {
            struct MemBlock* mb_tmp = NULL;
            auto r = this->reserve(name, name_len, 0, value?TYPE_BOOL_TRUE: TYPE_BOOL_FALSE, mb_tmp);
            return r;
        }
        Result update(const char* name, int name_len, const char* value, int size) {
            if (size > 255) {
                return Result::NG;
            }
            struct MemBlock* mb_tmp = NULL;

            auto r = this->reserve(name, name_len, size+1, TYPE_SHORT_TEXT, mb_tmp);
            if (r != Result::OK) {
                return r;
            }
            *((&mb_tmp->data0) + name_len) = (MIB_INT8)size;
            memcpy((&mb_tmp->data0) + name_len + 1, value, size);
            return Result::OK;
        }

        /// <summary>
        /// bufidxにある行を読出し、次の行のbufidxを返す。
        /// textは連続したメモリであることを補償します。
        /// @bug あれこれ終端検知どうなってるの？bufidxがメモリサイズより大きくなったときに下位がエラーを返すことを当てにしてる？
        /// -1でエラー。
        /// </summary>
        int iter(int bufidx, const struct MemBlock*& mb)
        {
            int s = 0;
            if (!this->getMemBlockSize(bufidx, s)) {
                return -1;
            }
            //メモリブロックを取得
            if (!this->_buf.ptr(bufidx, s, (void*&)mb)) {
                return -1;
            }
            return bufidx + s;
        }
        int freeSize()const {
            return this->_buf.freeSize();
        }
    };
}