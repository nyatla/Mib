#pragma once
#include "./_resolver_imports.h"

using namespace MIB;

namespace MIB {


    /// <summary>
    /// 0-31まで
    /// </summary>
    enum class DelimType :MIB_UINT8 {
        BRKT_L = 0,   //  (
        BRKT_R = 1,   //  )
        MUL = 2,   //  *
        DIV = 3,   //  /
        MOD = 4,   //  %
        PLUS = 5,   //  +
        MINUS = 6,   //  -
        SHL = 7,   //  <<
        SHR = 8,   //  >>
        AND = 9,   //  未割当
        OR = 10,  //  未割当
        NOT = 11,  //  未割当
        XOR = 12,  //  未割当
        LT = 13,  //  <
        LTEQ = 14,  //  <=
        GT = 15,  //  >
        GTEQ = 16,  //  >=
        EQ = 17,  //  ==
        NOTEQ = 18,   //  != <>
        COM=19, //　,    並列演算子
    };


    typedef struct OpDef_t {
        DelimType delim;
        //    MIB_UINT8 shortid;//0から31まで。逆参照に使うからOpTableDefのインデクスと合わせて。
        MIB_UINT8 prio;
    }OpDef;

    /// <summary>
    /// 優先度テーブル。値が小さいほうが優先度が高い。
    /// </summary>
    const static OpDef OpTableDef[] = {
    OpDef{ DelimType::BRKT_L,  0},
    OpDef{ DelimType::BRKT_R,  0},  //マーカー
    OpDef{ DelimType::MUL,     2 },
    OpDef{ DelimType::DIV,     2 },
    OpDef{ DelimType::MOD,     2 },
    OpDef{ DelimType::PLUS,    3 },
    OpDef{ DelimType::MINUS,   3 },
    OpDef{ DelimType::SHL,     4 },
    OpDef{ DelimType::SHR,     4 },
    OpDef{ DelimType::AND,     5 },
    OpDef{ DelimType::OR ,     5 },
    OpDef{ DelimType::NOT ,    1 },
    OpDef{ DelimType::XOR ,    5 },
    OpDef{ DelimType::LT,      6 },
    OpDef{ DelimType::LTEQ,    6 },
    OpDef{ DelimType::GT,      6 },
    OpDef{ DelimType::GTEQ,    6 },
    OpDef{ DelimType::EQ,      6 },
    OpDef{ DelimType::NOTEQ,   6 },
    OpDef{ DelimType::COM,  1},
    };

#define OpTableDef_BRKT_L (OpTableDef[(int)DelimType::BRKT_L])
#define OpTableDef_BRKT_R (OpTableDef[(int)DelimType::BRKT_R])
#define OpTableDef_MUL (OpTableDef[(int)DelimType::MUL])
#define OpTableDef_DIV (OpTableDef[(int)DelimType::DIV])
#define OpTableDef_MOD (OpTableDef[(int)DelimType::MOD])
#define OpTableDef_PLUS (OpTableDef[(int)DelimType::PLUS])
#define OpTableDef_MINUS (OpTableDef[(int)DelimType::MINUS])
#define OpTableDef_SHL (OpTableDef[(int)DelimType::SHL])
#define OpTableDef_SHR (OpTableDef[(int)DelimType::SHR])
#define OpTableDef_AND (OpTableDef[(int)DelimType::AND])
#define OpTableDef_OR (OpTableDef[(int)DelimType::OR])
#define OpTableDef_XOR (OpTableDef[(int)DelimType::XOR])
#define OpTableDef_NOT (OpTableDef[(int)DelimType::NOT])
#define OpTableDef_LT (OpTableDef[(int)DelimType::LT])
#define OpTableDef_LTEQ (OpTableDef[(int)DelimType::LTEQ])
#define OpTableDef_GT (OpTableDef[(int)DelimType::GT])
#define OpTableDef_GTEQ (OpTableDef[(int)DelimType::GTEQ])
#define OpTableDef_EQ (OpTableDef[(int)DelimType::EQ])
#define OpTableDef_NOTEQ (OpTableDef[(int)DelimType::NOTEQ])
#define OpTableDef_COM (OpTableDef[(int)DelimType::COM])


    class RpQueue;
    /// <summary>
    /// キーワード関数のためのキュー操作オブジェクトです。
    /// </summary>
    class RpQueueContext{
    private:
        RpQueue& _q;
        int _top;
    public:
        /// <summary>
        /// 
        /// </summary>
        /// <param name="q">index番目の要素に,KWD,str,gt,v,com,...ltの順で値が格納されていると仮定します。</param>
        /// <param name="index"></param>
        RpQueueContext(RpQueue& q, int index) :_q(q), _top(index)
        {
        }
        virtual ~RpQueueContext() {};
    public:
        //bool done;
        //const int length;
        /// <summary>
        /// 0にキーワード名,1以降に変数への参照
        /// </summary>
        /// <param name="index"></param>
        //RpQueue::Result argType(int index,int& dest)const {
        //    return this->_q.peekType(2 + index * 2, dest);
        //};
        virtual void argAsInt(int index)const {};
        virtual void argAsStr(int index)const {};
        /// <summary>
        /// キーワード関数をキューから削除して、新しく値を積みます。
        /// この関数は1度だけ
        /// </summary>
        virtual void replaceToStr(const char* v,int size)=0;
        virtual void replaceToInt(int v) = 0;
    };


    class RpFunction {
    public:
        const char* name;
        virtual ~RpFunction() {

        }
    public:

        bool execute(RpQueueContext& context) {
        }

    };
    /// <summary>
    /// テスト用のダミー関数
    /// </summary>
    static class DummmyFunction: public RpFunction{
        bool execute(RpQueueContext& context) {
            context.replaceToInt(299);
            return true;
        }
    }_DummmyFunction;
    /// <summary>
    /// NULL終端のFunctionTable
    /// </summary>
    static struct FunctionTable {
        RpFunction* _table[];
        RpFunction* getFunctionByKwd(const char* name) {
            for (auto i = 0;;i++) {
                if (strcmp(this->_table[i]->name, name) == 0) {
                    return this->_table[i];
                }
            }
            return NULL;
        }
    }function_table = {
        {
            &_DummmyFunction,
            NULL,
        }
    };


    enum class RpResult {
        OK,
        NG,
        NG_EOS,
        //        NG_IO,
        NG_TokenTooLong,
        NG_StackOverFlow,
        NG_SyntaxError,
        NG_InvalidNumber,
        NG_NumberRange,
        NG_UnknownDelimiter,
        NG_UnknownToken,
        NG_KeywordTooLong,
        NG_OpQIndexRange,
        NG_CanNotCast,
        NG_UndefinedOP,
    };

    /// <summary>
    /// 逆ポーランド記法の計算キューの実装です。
    /// メモリサイズを決定する部分はRpQueueImplに実装します。
    /// 演算子:
    ///     順序
    ///         ( )　,
    ///     減算、積算、除算、余剰
    ///         +               ->  (int, int) (str, int) (int, str) (str, str)
    ///         - * / % Mod     ->  (int, int)
    ///     ビット演算子又は論理演算子
    ///         Xor Or And      ->  (int, int) (bool, bool)
    ///     ビット演算子又は論理演算子
    ///         Not             ->  (int)   (bool)
    ///     ビット演算子
    ///         << >>           ->  (int, int)
    ///     比較演算子
    ///         < > <= >=       ->  (int,int)
    ///         == <> !=        ->  (int,int) (bool, bool)   
    /// 値型:
    ///     INT32 STRING BOOL KEYWORD
    /// 
    /// 順序演算子の,は、直前の(、または,までを１つの値として格納する。
    /// 順序演算子の()は、)の直前にある(からの内部を1つの値とする。
    /// 順序演算子の、(の直前にある値型がKEYWORDの場合、キーワード関数として扱われる。
    /// </summary>
    /// <typeparam name="QSIZE"></typeparam>
    /// <typeparam name="STACKDEPTH"></typeparam>
    class RpQueue {
    public:
        //enum class Result {
        //    OK,
        //    NG,
        //    NG_UNDEF_FUNCTION_KEYWORD,
        //    NG_UNDEF_VALUE_KEYWORD,
        //    NG_CANNOT_CAST,
        //    NG_INDEX_RANGE,
        //    NG_KWD_TOO_LONG,
        //    NG_UNDEF_OP
        //};
        virtual ~RpQueue() {};
    protected:
        /// <summary>
        /// TYPE_XはUINT8型でスタックに積まれた変数の値を示します。
        /// </summary>
        const static unsigned char TYPE_OP_MAX = 31;
        const static unsigned char TYPE_INT32 = 32;
        const static unsigned char TYPE_INT16 = 33;
        const static unsigned char TYPE_INT8 = 34;
        const static unsigned char TYPE_BOOL_TRUE = 35;
        const static unsigned char TYPE_BOOL_FALSE = 36;

        const static unsigned char TYPE_KWD_LEN = 32-1;//31;
        const static unsigned char TYPE_KWD_MIN = 64;
        const static unsigned char TYPE_KWD_MAX = TYPE_KWD_MIN + TYPE_KWD_LEN;  //95

        const static unsigned char TYPE_SHORT_STR_LEN = 64 - 2;//64-2;
        const static unsigned char TYPE_SHORT_STR_MIN = 128;
        const static unsigned char TYPE_SHORT_STR_MAX = TYPE_SHORT_STR_MIN + TYPE_SHORT_STR_LEN;
        const static unsigned char TYPE_LONG_STR = TYPE_SHORT_STR_MAX + 1;  //191
    private:
        //[S][D...] SにサイズとTYPE,Dにデータを格納する。
        //S=0-31        OPType                  [OP+V:1]  1バイトの演算子定数
        //S=32,33,34    int(4),int(2),int(1)    [OP:1][V:*] 1+(1,2,4)バイトの整数
        //S=35,36       bool(1)                 [OP+V:1] 1バイトの真偽値
        //S=64-95       keyword (0-31)          [OP+S:1][V:*] 最大31文字のキーワード文字列
        //S=128-190     short string (0-62)     [OP+S:1][V:*] 最大で62文字の文字列
        //S=191         long string             [OP:1][S:2][V:*] 最大で65535文字の文字列。2バイトのサイズ情報付き
        //!PENDING S=192-254     short bytes (0-62)      [OP+S:1][V] 最大で62バイトのバイナリ
        //!PENDING S=255         long binaly             [OP:1][S:2][V:*]最大で65535バイトのバイナリ
        //MIB_UINT8 _buf[QSIZE] = {};         //値を格納するメモリ
        ////bufに格納したデータブロックの先頭位置を示すインデクス値
        //MIB_UINT16 _stack[STACKDEPTH] = {};  //格納位置
        //int _sp = 0;                //スタックポインタ   
        //int _ptr = 0;               //書込みポインタ
        bool _push_only = false;    //プロパティ。計算省略フラグ
    protected:
        /// <summary>
        /// キーワード演算の結果を要求する。
        /// 1番目にKWD,後ろに値
        /// </summary>
        virtual int handleKeyword(int sp,int size)
        {
            return 0;
        }
        virtual RpResult handleFunction() {
            return RpResult::NG;
        }
    public:
        /// <summary>
        /// インスタンスをリセットする
        /// </summary>
        virtual void reset() {
            this->_push_only = false;
        }
        /// <summary>
        /// n番目の要素のキューメモリ上のポインタを返します。
        /// </summary>
        /// <param name="idx"></param>
        /// <returns></returns>
        virtual RpResult ptr(int idx, MIB_UINT8*& ptr) = 0;
        /// <summary>
        /// constポインタ版
        /// </summary>
        /// <param name="idx"></param>
        /// <returns></returns>
        virtual RpResult constPtr(int idx,const MIB_UINT8*& ptr)const = 0;
        /// <summary>
        /// メモリブロックを1個pushします。
        /// </summary>
        /// <param name="d">NULLの場合、スタックポインタのみ移動します。</param>
        /// <param name="s"></param>
        /// <returns></returns>
        virtual RpResult push(const MIB_INT8* d, int s) = 0;
        /// <summary>
        /// n個の値をpopする。
        /// </summary>
        /// <param name="n"></param>
        /// <returns></returns>
        virtual RpResult pop(int n) = 0;

        ///// <summary>
        ///// キューからn番目の要素を取り外す。
        ///// </summary>
        ///// <param name="n"></param>
        ///// <returns></returns>
        virtual RpResult remove(int idx) = 0;


    public:
        RpResult pushInt(MIB_INT32 v) {
            if (v > INT16_MAX || v < INT16_MIN) {
                MIB_INT8 d[5] = { TYPE_INT32 , };
                int2bytes(v, d + 1);
                return this->push(d, 5);
            }
            else if (v > INT8_MAX || v < INT8_MIN) {
                MIB_INT8 d[3] = { TYPE_INT16, };
                short2bytes(v, d + 1);
                return this->push(d, 3);
            }
            else {
                MIB_INT8 d[2] = { TYPE_INT8, v & 0xff };
                return this->push(d, 2);
            }
        }
        RpResult pushKeyword(const MIB_INT8* v, int len) {
            if (len > TYPE_KWD_LEN) {
                return RpResult::NG_KeywordTooLong;
            }
            RpResult r;
            r = this->push(NULL, len + 1);
            if (r!= RpResult::OK) {
                return r;
            }
            MIB_UINT8* ptr = NULL;
            r= this->ptr(-1,ptr);
            if (r != RpResult::OK) {
                return r;
            }
            *(ptr + 0) = 0xff & (len + TYPE_KWD_MIN);
            memmove(ptr + 1, v, len);
            return RpResult::OK;
        }

        RpResult pushStr(const MIB_INT8* v, int len) {
            RpResult r;
            if (len <= TYPE_SHORT_STR_LEN) {
                r = this->push(NULL, len + 1);
                if (r!= RpResult::OK) {
                    return r;
                }
                MIB_UINT8* ptr = NULL;
                r = this->ptr(-1,ptr);
                if (r != RpResult::OK) {
                    return r;
                }
                *(ptr + 0) = 0xff & (len + TYPE_SHORT_STR_MIN);
                memmove(ptr + 1, v, len);
                return RpResult::OK;
            }
            else {
                r = this->push(NULL, len + 3);
                if (r!= RpResult::OK) {
                    return r;
                }
                MIB_UINT8* ptr = NULL;
                r = this->ptr(-1, ptr);
                if (r != RpResult::OK) {
                    return r;
                }
                *(ptr + 0) = TYPE_LONG_STR;
                ushort2bytes((MIB_UINT16)len, ptr + 1);
                memmove(ptr + 3, v, len);
                return RpResult::OK;
            }
        }
        RpResult pushBool(bool v)
        {
            MIB_INT8 d = v ? TYPE_BOOL_TRUE : TYPE_BOOL_FALSE;
            return this->push(&d, 1);
        }



        ///// <summary>
        ///// N番目の要素が取得可能であり、型が一致するかチェックする
        ///// </summary>
        ///// <param name="idx"></param>
        ///// <returns></returns>
        RpResult isTypeEqual(int idx, int c,bool& match)const {
            int w;
            auto r = this->peekType(idx, w);
            if (r!= RpResult::OK) {
                return r;
            }
            match= w == c;
            return RpResult::OK;
        }

        /// <summary>
        /// N番目の要素のタイプ値を得る。
        /// </summary>
        /// <param name="idx">-1で末尾。</param>
        /// <returns></returns>
        RpResult peekType(int idx, int& dst)const
        {
            const MIB_UINT8* ptr;
            auto ret = this->constPtr(idx,ptr);
            if (ret!= RpResult::OK!= NULL) {
                return ret;
            }
            dst = *ptr;
            return RpResult::OK;
        }
        /// <summary>
        /// N番目の要素からint値を取り出す。
        /// </summary>
        /// <param name="idx"></param>
        /// <param name="out"></param>
        /// <returns></returns>
        RpResult peekInt(int idx, int& out)const
        {
            const MIB_UINT8* ptr;
            auto r = this->constPtr(idx,ptr);
            if (r!= RpResult::OK) {
                return r;
            }
            //型チェック
            switch (ptr[0])
            {
            case TYPE_INT32:out = bytes2int(ptr + 1);break;
            case TYPE_INT16:out = bytes2short(ptr + 1);break;
            case TYPE_INT8:out = (MIB_INT8)ptr[1];break;
            default:
                return RpResult::NG_CanNotCast;
            }
            return RpResult::OK;
        }
        RpResult peekBool(int idx, bool& out)const
        {
            const MIB_UINT8* ptr;
            auto r = this->constPtr(idx, ptr);
            if (r== RpResult::OK && (*ptr == TYPE_BOOL_TRUE || *ptr == TYPE_BOOL_FALSE)) {
                out = *ptr == TYPE_BOOL_TRUE;
                return RpResult::OK;
            }
            return RpResult::NG_CanNotCast;
        }


        /// <summary>
        /// N番目の要素を文字列と解釈して、文字数とサイズを返す
        /// </summary>
        /// <param name="idx"></param>
        /// <param name="ptr"></param>
        /// <param name="len"></param>
        /// <returns></returns>
        RpResult peekStr(int idx, const MIB_INT8*& ptr, int& len)const {
            const MIB_UINT8* p;
            auto r = this->constPtr(idx,p);
            if (r!= RpResult::OK) {
                return r;
            }
            //型チェック
            if (TYPE_SHORT_STR_MIN <= p[0] && p[0] <= TYPE_SHORT_STR_MAX) {
                len = p[0] - TYPE_SHORT_STR_MIN;
                ptr = (const MIB_INT8*)p + 1;
                return RpResult::OK;
            }
            if (p[0] == TYPE_LONG_STR) {
                len = bytes2ushort(p + 1);
                ptr = (const MIB_INT8*)p + 3;
                return RpResult::OK;
            }
            return RpResult::NG_CanNotCast;
        }
        /// <summary>
        /// N番目の要素を文字列と解釈して、文字数とサイズを返す
        /// </summary>
        /// <param name="idx"></param>
        /// <param name="ptr"></param>
        /// <param name="len"></param>
        /// <returns></returns>
        RpResult peekKeyword(int idx, const MIB_INT8*& ptr, int& len)const
        {
            const MIB_UINT8* p;
            RpResult r=this->constPtr(idx,p);
            if (r!= RpResult::OK) {
                return r;
            }
            //型チェック
            if (TYPE_KWD_MIN <= p[0] && p[0] <= TYPE_KWD_MAX) {
                len = p[0] - TYPE_KWD_MIN;
                ptr = (const MIB_INT8*)p + 1;
                return RpResult::OK;
            }
            return RpResult::NG_CanNotCast;
        }
        static inline bool _isIntType(MIB_UINT8 t) { return (t == TYPE_INT32 || t == TYPE_INT16 || t == TYPE_INT8); }
        static inline bool _isStrType(MIB_UINT8 t) { return (t == TYPE_LONG_STR || (TYPE_SHORT_STR_MIN <= t && t <= TYPE_SHORT_STR_MAX)); }
        static inline bool _isKwdType(MIB_UINT8 t) { return (TYPE_KWD_MIN <= t && t <= TYPE_KWD_MAX); }

    private:
        typedef union OpValue {
            int i;
            bool b;
        }OpValue_t;
        RpResult popII(int&a,int&b)
        {
            RpResult r;
            r = this->peekInt(-1, a);
            if (r != RpResult::OK) {
                return r;
            }
            r = this->peekInt(-2, b);
            if (r != RpResult::OK) {
                return r;
            }
            return this->pop(2);
        }
        RpResult popBB(bool& a, bool& b)
        {
            RpResult r;
            r = this->peekBool(-1, a);
            if (r != RpResult::OK) {
                return r;
            }
            r = this->peekBool(-2, b);
            if (r != RpResult::OK) {
                return r;
            }
            return this->pop(2);
        }

        RpResult do_and() {
            OpValue_t a = {}, b = {};
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK){
                return this->pushInt(b.i & a.i);
            }
            r = this->popBB(a.b, b.b);
            if (r == RpResult::OK) {
                return this->pushBool(b.b && a.b);
            }
            return r;
        }
        RpResult do_or() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK) {
                return this->pushInt(b.i | a.i);
            }
            r = this->popBB(a.b, b.b);
            if(r== RpResult::OK){
                return this->pushBool(b.b || a.b);
            }
            return r;
        }
        RpResult do_xor() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK) {
                return this->pushInt(b.i ^ a.i);
            }
            r = this->popBB(a.b, b.b);
            if(r== RpResult::OK){
                return this->pushBool(b.b != a.b);
            }
            return r;
        }
        RpResult do_eq() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK){
                return this->pushBool(b.i == a.i);
            }
            r = this->popBB(a.b, b.b);
            if (r == RpResult::OK) {
                return this->pushBool(b.i == a.i);
            }
            return r;
        }
        RpResult do_noteq()
        {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if (r == RpResult::OK) {
                return this->pushBool(b.i != a.i);
            }
            r = this->popBB(a.b, b.b);
            if(r== RpResult::OK){
                return this->pushBool(b.i != a.i);
            }
            return r;
        }
        RpResult do_not() {
            OpValue_t a;
            RpResult r;
            r = this->peekInt(-1, a.i);
            if (r == RpResult::OK) {
                this->pop(1);
                return this->pushInt(~a.i);
            }
            r = this->peekBool(-1, a.b);
            if(r== RpResult::OK){
                this->pop(1);
                return this->pushBool(!a.b);
            }
            return r;
        }
        RpResult do_lt() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK) {
                return this->pushBool(b.i < a.i);
            }
            return r;
        }
        RpResult do_lteq() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if (r== RpResult::OK) {
                return this->pushBool(b.i <= a.i);
            }
            return r;
        }
        RpResult do_gt() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK){
                return this->pushBool(b.i > a.i);
            }
            return r;
        }
        RpResult do_gteq() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK){
                return this->pushBool(b.i >= a.i);
            }
            return r;
        }
        RpResult do_mul() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK){
                return this->pushInt(b.i * a.i);
            }
            return r;
        }
        RpResult do_minus() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if (r== RpResult::OK) {
                return this->pushInt(b.i - a.i);
            }
            return r;
        }
        RpResult do_plus() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if (r== RpResult::OK) {
                return this->pushInt(b.i + a.i);
            }
            {
                //strがどちらかにあること
                int t[2] = {};
                for (int i = 0;i < 2;i++) {
                    r = this->peekType(-1 - i, t[i]);
                    if (r!= RpResult::OK) {
                        return r;
                    }
                }
                if (!_isStrType(t[0]) && !_isStrType(t[1])) {
                    return RpResult::NG_CanNotCast;
                }
                int l[2] = { 0,0 };
                const MIB_INT8* s[2] = { NULL,NULL };
                char work[16];
                for (int i = 0;i < 2;i++) {
                    if (_isStrType(t[i])) {
                        r = this->peekStr(-1 - i, s[i], l[i]);
                        if (r== RpResult::OK) {
                            continue;
                        }
                    }
                    //両方がintということはない
                    if (_isIntType(t[i])) {
                        int w = 0;
                        r = this->peekInt(-1 - i, w);
                        if (r!= RpResult::OK) {
                            return r;
                        }
                        l[i] = countDigits(w);
                        s[i] = work;
                        MIB::itoa(w, work, 10);
                        continue;
                    }
                }
                //書込み位置の計算
                int sl = l[0] + l[1];
                int hsize = (sl <= TYPE_SHORT_STR_LEN) ? 1 : 3;
                int d2 = hsize;//d2の書込み位置
                int d1 = d2 + l[1];

                this->pop(2);
                //スタックポインタを3個破棄して領域をpush
                r = this->push(NULL, hsize + sl);
                if (r!= RpResult::OK) {
                    return r;
                }
                MIB_UINT8* ptr;
                r= this->ptr(-1,ptr);
                if (r != RpResult::OK) {
                    return r;
                }
                //領域に文字列を編集
                memmove(ptr + d1, s[0], l[0]);//t1
                memmove(ptr + d2, s[1], l[1]);//t2
                //TYPE値の書込み
                if (hsize == 1) {
                    *ptr = TYPE_SHORT_STR_MIN + (MIB_INT8)sl;
                }
                else {
                    *ptr = TYPE_LONG_STR;
                    ushort2bytes((MIB_UINT16)sl, ptr + 1);
                }
            }
            return RpResult::OK;
        }
        RpResult do_mod() {
            OpValue_t a = {}, b = {};
            RpResult r;
            r = this->popII(a.i, b.i);
            if (r== RpResult::OK) {
                return this->pushInt(b.i % a.i);
            }
            return r;
        }
        RpResult do_div() {
            OpValue_t a = {}, b = {};
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK){
                return this->pushInt(b.i / a.i);
            }
            return r;
        }
        RpResult do_shl() {
            OpValue_t a = {}, b = {};
            RpResult r;
            r = this->popII(a.i, b.i);
            if (r== RpResult::OK) {
                return this->pushInt(b.i << a.i);
            }
            return r;
        }
        RpResult do_shr() {
            OpValue_t a, b;
            RpResult r;
            r = this->popII(a.i, b.i);
            if(r== RpResult::OK){
                return this->pushInt(b.i >> a.i);
            }
            return r;
        }
        RpResult do_brk_r()
        {
            //直前の(を検出する。
            for (auto i = 1;;i++) {
                auto t = 0;
                RpResult ret;
                ret = this->peekType(-i, t);
                if (ret!= RpResult::OK) {
                    return ret;
                }
                if (t != (int)DelimType::BRKT_L) {
                    continue;
                }
                //ブラケット直前がキーワードか判定(エラーはハンドル不要)
                if (this->peekType(-(i + 1), t)== RpResult::OK) {
                    if (TYPE_KWD_MIN <= t && t <= TYPE_KWD_MAX) {
                        ////キーワード関数処理
                        //関数がスタックを操作し終えていることを期待する。
                        return RpResult::NG;
                        //return this->handleFunction(int index);

                        //Args args(*this, -(i - 1), n);
                        ////名前,引数リスト
                        //functionCallback(args);
                    }
                }
                //単純ブラケット
                //発見->ブラケットを取り去る
                this->remove(-i);
                return RpResult::OK;
                //if (i == 2) {
                //    this->remove(-i);
                //    return true;
                //}


                //直前がKWDの場合→関数
                //それ以外の場合→ブラケットの包んでいる値は１個だけ。かつブラケットの取り外し
            }
        }
        /// <summary>
        /// キューの末尾の演算子を実行する。
        /// </summary>
        /// <returns></returns>
        RpResult execute()
        {
            auto t1 = 0;
            RpResult ret = this->peekType(-1, t1);
            if (ret!= RpResult::OK) {
                return ret;
            }
            bool oret=false;
            this->pop(1);
            switch(t1){
            case (int)DelimType::AND:   return this->do_and();
            case (int)DelimType::OR:    return this->do_or();
            case (int)DelimType::XOR:   return this->do_xor();
            case (int)DelimType::EQ:    return this->do_eq();
            case (int)DelimType::NOTEQ: return this->do_noteq();
            case (int)DelimType::NOT:   return this->do_not();break;
            case (int)DelimType::LT:    return this->do_lt();break;
            case (int)DelimType::LTEQ:  return this->do_lteq();break;
            case (int)DelimType::GT:    return this->do_gt();break;
            case (int)DelimType::GTEQ:  return this->do_gteq();break;

            case (int)DelimType::MUL:   return this->do_mul();break;
            case (int)DelimType::MINUS: return this->do_minus();break;
            case (int)DelimType::PLUS:  return this->do_plus();break;
            case (int)DelimType::MOD:   return this->do_mod();break;
            case (int)DelimType::DIV:   return this->do_div();break;
            case (int)DelimType::SHL:   return this->do_shl();break;
            case (int)DelimType::SHR:   return this->do_shr();break;
            //COMはそのまま
            case (int)DelimType::COM:   return RpResult::OK;
            case (int)DelimType::BRKT_L:return RpResult::OK;
            case (int)DelimType::BRKT_R:
                return this->do_brk_r();
            default:
                return RpResult::NG_UndefinedOP;
            }
        }
    public:
        /// <summary>
        /// 演算子をキューに追加し、キューの末尾に対して演算を実行する。
        /// 構造識別子の場合はそのままキューに積む
        /// </summary>
        /// <param name="o"></param>
        /// <returns></returns>
        RpResult pushOp(const OpDef* o)
        {
            MIB_INT8 c = (MIB_UINT8)o->delim;
            RpResult r;
            r = this->push(&c, 1);
            if (r!= RpResult::OK) {
                return r;
            }
            switch (o->delim) {
            case DelimType::BRKT_L:
            case DelimType::COM:
                return RpResult::OK;
            }
            if (!this->_push_only)
            {
                return this->execute();
            }
            return RpResult::OK;
        }
        RpResult pushBraket() {
            MIB_INT8 c = (MIB_UINT8)DelimType::BRKT_L;
            return this->push(&c,1);
        }
        /// <summary>
        /// trueをセットすると演算を行わない。
        /// </summary>
        /// <param name="f"></param>
        void setPushOnly(bool f) {
            this->_push_only = f;
        }
    };


    /// <summary>
    /// 固定長メモリをラップする実装
    /// </summary>
    /// <typeparam name="QSIZE"></typeparam>
    /// <typeparam name="STACKDEPTH"></typeparam>
    template <int QSIZE = 256, int STACKDEPTH = 16> class RpQueueImpl :public RpQueue{
    private:
        MIB_UINT8 _buf[QSIZE] = {};         //値を格納するメモリ
        //bufに格納したデータブロックの先頭位置を示すインデクス値
        MIB_UINT16 _stack[STACKDEPTH] = {};  //格納位置
        int _sp = 0;                //スタックポインタ   
        int _ptr = 0;               //書込みポインタ
    private:
        RpResult _getStackIndex(int idx, int& s)const
        {
            if (idx >= 0) {
                if (idx < this->_sp) {
                    s = idx;
                    return RpResult::OK;
                }
            }
            else if (this->_sp + idx >= 0) {
                s = this->_sp + idx;
                return RpResult::OK;
            }
            return RpResult::NG_OpQIndexRange;
        }
        /// <summary>
        /// N番目のスタック要素のbuf上のサイズを返します。
        /// </summary>
        /// <returns></returns>
        RpResult _getBlockSize(int idx, int& s)const
        {
            int p;
            RpResult r;
            r = this->_getStackIndex(idx, p);
            if (r!= RpResult::OK) {
                return r;
            }
            const MIB_UINT8* ptr;
            r=this->constPtr(idx,ptr);
            if (r!= RpResult::OK) {
                return r;
            }
            auto tval = ptr[0];//型変数
            if (tval <= TYPE_OP_MAX || tval == TYPE_BOOL_TRUE || tval == TYPE_BOOL_FALSE) {
                s = 1;
            }
            else if (tval == TYPE_INT32) {
                s = 5;
            }
            else if (tval == TYPE_INT16) {
                s = 3;
            }
            else if (tval == TYPE_INT8) {
                s = 2;
            }
            else if ((TYPE_KWD_MIN <= tval && tval <= TYPE_KWD_MAX)) {
                s = 1 + tval - TYPE_KWD_MIN;
            }
            else if ((TYPE_SHORT_STR_MIN <= tval && tval <= TYPE_SHORT_STR_MAX)) {
                s = 1 + tval - TYPE_SHORT_STR_MIN;
            }
            else if (tval == TYPE_LONG_STR) {
                s = 1 + 2 + bytes2ushort(&ptr[1]);
            }
            else {
                return RpResult::NG;
            }
            return RpResult::OK;
        }
    protected:

        /// <summary>
        /// n番目の要素のキューメモリ上のポインタを返します。
        /// </summary>
        /// <param name="idx"></param>
        /// <returns></returns>
        RpResult ptr(int idx, MIB_UINT8*& ptr)
        {
            int p;
            RpResult r;
            r = this->_getStackIndex(idx, p);
            if (r != RpResult::OK) {
                return r;
            }
            ptr=&this->_buf[*(this->_stack + p)];
            return RpResult::OK;
        }
        /// <summary>
        /// constポインタ版
        /// </summary>
        /// <param name="idx"></param>
        /// <returns></returns>
        RpResult constPtr(int idx,const MIB_UINT8*& ptr)const
        {
            int p;
            RpResult r;
            r = this->_getStackIndex(idx, p);
            if (r!= RpResult::OK) {
                return r;
            }
            ptr=this->_buf + *(this->_stack + p);
            return RpResult::OK;
        }
    public:
        void reset()
        {
            RpQueue::reset();
            this->_sp = 0;
            this->_ptr = 0;
        }
        /// <summary>
        /// メモリブロックをpushします。
        /// </summary>
        /// <param name="d">NULLの場合、スタックポインタのみ移動します。</param>
        /// <param name="s"></param>
        /// <returns></returns>
        RpResult push(const MIB_INT8* d, int s) {
            auto& ptr = this->_ptr;
            auto& buf = this->_buf;
            if (ptr + s >= QSIZE || this->_sp >= STACKDEPTH) {
                return RpResult::NG_OpQIndexRange;
            }
            if (d != NULL) {
                memmove(buf + ptr, d, s);
            }
            this->_stack[this->_sp++] = ptr;
            this->_ptr += s;
            return RpResult::OK;
        }
        /// <summary>
        /// n個の値をpopする。
        /// </summary>
        /// <param name="n"></param>
        /// <returns></returns>
        RpResult pop(int n)
        {
            for (auto i = 0;i < n;i++) {
                if (this->_sp < 1) {
                    return RpResult::NG_OpQIndexRange;
                }
                int bs = 0;
                RpResult r;
                r = this->_getBlockSize(-1, bs);
                if (r!= RpResult::OK) {
                    return r;
                }
                this->_sp--;
                this->_ptr -= bs;
            }
            return RpResult::OK;
        }
        ///// <summary>
        ///// キューからn番目の要素を取り外す。
        ///// </summary>
        ///// <param name="n"></param>
        ///// <returns></returns>
        RpResult remove(int idx) {
            int p;
            RpResult r;
            r = this->_getStackIndex(idx, p);
            if (r!= RpResult::OK) {
                return r;
            }
            //末尾ならpopで終える。
            if (p == this->_sp - 1) {
                return this->pop(-1);
            }
            //中途なら左詰め
            int bs = 0;
            r = this->_getBlockSize(p, bs);
            if (r!= RpResult::OK) {   //ブロックのサイズ
                return r;
            }
            auto bp = this->_stack[p];             //bufのポインタ
            memmove(&this->_buf[bp], &this->_buf[bp + bs], this->_ptr - bs);//移動
            //スタックの移動
            for (auto i = p + 1;i < this->_sp;i++) {
                this->_stack[i - 1] = this->_stack[i] - bs;
            }
            this->_sp--;
            return RpResult::OK;

        }

#ifdef TEST
    public:
        /// <summary>
        /// バッファの内容をダンプする。
        /// バッファオーバーフローとかあまり気にしていないのでデバッグ以外には使わないで。
        /// </summary>
        /// <param name="inst"></param>
        /// <returns></returns>
        static const char* sdump(const RpQueueImpl& inst) {


            static char strbuf[256];
            char* str = strbuf;
            auto& b = inst._buf;
            for (int i = 0;i < inst._sp;i++) {
                int p = 0;
                inst.peekType(i, p);
                switch (p) {
                case (int)DelimType::BRKT_L:str = str + sprintf(str, "( ");continue;
                case (int)DelimType::BRKT_R:str = str + sprintf(str, ") ");continue;
                case (int)DelimType::MUL:str = str + sprintf(str, "* ");continue;
                case (int)DelimType::DIV:str = str + sprintf(str, "/ ");continue;
                case (int)DelimType::MOD:str = str + sprintf(str, "%% ");continue;
                case (int)DelimType::PLUS:str = str + sprintf(str, "+ ");continue;
                case (int)DelimType::MINUS:str = str + sprintf(str, "- ");continue;
                case (int)DelimType::LT:str = str + sprintf(str, "< ");continue;
                case (int)DelimType::LTEQ:str = str + sprintf(str, "<= ");continue;
                case (int)DelimType::GT:str = str + sprintf(str, "> ");continue;
                case (int)DelimType::GTEQ:str = str + sprintf(str, ">= ");continue;
                case (int)DelimType::EQ:str = str + sprintf(str, "== ");continue;
                case (int)DelimType::NOTEQ:str = str + sprintf(str, "!= ");continue;
                case (int)DelimType::COM:str = str + sprintf(str, ", ");continue;
                }
                if (_isIntType(p)) {
                    int v = 0;
                    inst.peekInt(i, v);
                    str = str + sprintf(str, "%d ", v);
                    continue;
                }
                if (_isStrType(p)) {
                    const MIB_INT8* s = NULL;
                    auto l = 0;
                    inst.peekStr(i, s, l);
                    str = str + sprintf(str, "\"%.*s\" ", l, s);
                    continue;
                }
                if (_isKwdType(p)) {
                    const MIB_INT8* s = NULL;
                    auto l = 0;
                    inst.peekKeyword(i, s, l);
                    str = str + sprintf(str, "%.*s ", l, s);
                    continue;
                }
                bool b;
                if (inst.peekBool(i, b)== RpResult::OK) {
                    str = str + sprintf(str, "%s ", b ? "TRUE" : "FALSE");
                    continue;

                }
                str = str + sprintf(str, "x ");break;
            }
            sprintf(str, "\0");
            return strbuf;
        }
#endif

    };


    /// <summary>
    /// 演算子スタック
    /// </summary>
    /// <typeparam name="SIZE"></typeparam>
    template <int SIZE = 16> class OpStack {
    private:
        MIB_UINT8 _buf[SIZE] = {};
        int _ptr = 0;//次の格納位置
    public:
        void reset() {
            this->_ptr = 0;             //書込みポインタ
        }
    public:
        /// <summary>
        /// 演算子をスタックに積む。優先順位が低い場合はqに払いだす。
        /// 構造演算子はOPスタックとキュー両方に追記する。
        /// </summary>
        /// <param name="v"></param>
        /// <param name="q"></param>
        /// <returns></returns>
        RpResult push(const OpDef* s, RpQueue& q)
        {
            //超過チェック
            if (this->_ptr >= SIZE) {
                return RpResult::NG_OpQIndexRange;
            }
            switch (s->delim) {
            case DelimType::COM:
            {
                //左端/左端識別子に当たるまで演算子を払い出す。
                //払い出しのみ
                const OpDef* w;
                while(this->peek(w)){
                    switch (w->delim) {
                    case DelimType::BRKT_L:
                    case DelimType::COM:
                        break;//左端到達
                    //case DelimType::COM:
                    //    break;//COMを積む
                    default:
                        RpResult r = q.pushOp(w);
                        if (r!= RpResult::OK) {
                            return r;  //スタック超過
                        }
                        this->pop();
                        continue;
                    }
                    break;
                }
                return q.pushOp(&OpTableDef_COM);
            }
            case DelimType::BRKT_R:
                //Lブラケットにあたるまで払い出し
                const OpDef* w;
                while(this->peek(w)) {
                    //直前がLならば取り外しのみ
                    if (w->delim == DelimType::BRKT_L) {
                        this->pop();
                        break;
                    }
                    else {                        
                        MIB_ASSERT(w->prio != 0);
                        RpResult r = q.pushOp(w);
                        if (r != RpResult::OK) {
                            return r;
                        }
                    }
                    this->pop();
                }
                return q.pushOp(&OpTableDef_BRKT_R);
            case DelimType::BRKT_L:
            {
                RpResult r = q.pushOp(&OpTableDef_BRKT_L);
                if (r != RpResult::OK) {
                    return r;
                }
                break;//Lを構造デリミタとして積む
            }
            default:
            {
                //新しい演算子より優先度の高い演算子を払い出し
                const OpDef* w;
                if (this->peek(w)) {
                    //直前がLブラケットならそのまま積む
                    if (w->delim == DelimType::BRKT_L) {
                        //pass
                    }
                    else {
                        //優先順位が低ければ払い出し
                        MIB_ASSERT(w->prio != 0);
                        if (w->prio <= s->prio) {
                            //払い出し
                            RpResult r;
                            r = q.pushOp(w);
                            if (r!=RpResult::OK) {
                                return r;
                            }
                            this->pop();
                        }
                    }
                }
                break;
            }
            }
            //積む
            this->_buf[this->_ptr++] = (MIB_UINT8)s->delim;
            return RpResult::OK;
        };
        bool pop() {
            if (this->_ptr == 0) {
                return false;
            }
            this->_ptr--;
            return true;
        };

        bool peek(int idx,const OpDef*& d)
        {
            auto s;
            if (idx >= 0) {
                if (idx < this->_ptr) {
                    s = idx;
                }
                else {
                    return false;
                }
            }
            else if (this->_ptr + idx >= 0) {
                s = this->_ptr + idx;
            }
            else {
                return false;
            }
            d = &OpTableDef[this->_buf[s]];
            return true;
        };

        /// <summary>
        /// dは定数への参照値です。popしても内容は保証されます。
        /// </summary>
        /// <param name="d"></param>
        /// <returns></returns>
        bool peek(const OpDef*& d) {
            if (this->_ptr == 0) {
                return false;
            }
            d = &OpTableDef[this->_buf[this->_ptr - 1]];
            return true;
        };
        bool isEmpty() {
            return this->_ptr == 0;
        }
        int ptr() { return this->_ptr; }

    };





class RpSolver
{
public:
//    enum class Result {
//        OK,
//        NG,
//        NG_EOS,
////        NG_IO,
//        NG_TokenTooLong,
//        NG_StackOverFlow,
//        NG_SyntaxError,
//        NG_InvalidNumber,
//        NG_NumberRange,
//        NG_UnknownDelimiter,
//        NG_UnknownToken,
//        NG_KeywordTooLong,
//        NG_OpQIndexRange,
//        NG_CanNotCast,
//    };
    static RpResult covertResult(RawTokenIterator::Result r) {
        switch (r) {
        case RawTokenIterator::Result::OK:return RpResult::OK;
        case RawTokenIterator::Result::NG_EOS:return RpResult::NG_EOS;
        case RawTokenIterator::Result::NG_InvalidNumber:return RpResult::OK;
        case RawTokenIterator::Result::NG_InvalidToken:return RpResult::OK;
        case RawTokenIterator::Result::NG_NumberRange:return RpResult::OK;
        default:return RpResult::NG;
        }
    }
    //static Result covertResult(RpQueue::Result r) {
    //    switch (r) {
    //    case RpQueue::Result::OK:return Result::OK;
    //    case RpQueue::Result::NG:return Result::NG;
    //    case RpQueue::Result::NG_UNDEF_FUNCTION_KEYWORD:return Result::NG;
    //    case RpQueue::Result::NG_UNDEF_VALUE_KEYWORD:return Result::NG;
    //    case RpQueue::Result::NG_CANNOT_CAST:return Result::NG_CanNotCast;
    //    case RpQueue::Result::NG_INDEX_RANGE:return Result::NG_OpQIndexRange;
    //    case RpQueue::Result::NG_KWD_TOO_LONG:return Result::NG_KeywordTooLong;
    //    case RpQueue::Result::NG_UNDEF_OP:return Result::NG;
    //    default:return Result::NG;
    //    }
    //}

private:
    /// <summary>
    /// サイズ付テキストトークンを値トークンに変換するパーサ。
    /// </summary>
    class RawTokenParser {
    public:
        RawTokenParser(){
        }
        /// <summary>
        /// デリミタとして登録されているトークンをOPとして解釈します。
        /// </summary>
        /// <param name="d"></param>
        /// <returns></returns>
        RpResult asOpToken(const RawTokenIterator::RawToken_t& token,const OpDef*& d)
        {
            MIB_ASSERT(token.type == RawTokenType::DELIM);
            if (token.size == 1) {
                switch (*token.ptr) {
                case '(':d = &OpTableDef_BRKT_L;break;
                case ')':d = &OpTableDef_BRKT_R;break;
                case '*':d = &OpTableDef_MUL;break;
                case '/':d = &OpTableDef_DIV;break;
                case '%':d = &OpTableDef_MOD;break;
                case '+':d = &OpTableDef_PLUS;break;
                case '-':d = &OpTableDef_MINUS;break;
                case '<':d = &OpTableDef_LT;break;
                case '>':d = &OpTableDef_GT;break;
                case ',':d = &OpTableDef_COM;break;
                default:
                    return RpResult::NG_UnknownDelimiter;
                }
                return RpResult::OK;
            }
            if (token.size == 2) {
                switch (*token.ptr) {
                case '!':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_NOTEQ;break;
                    default:
                        return RpResult::NG_UnknownDelimiter;
                    }
                    break;
                case '=':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_EQ;break;
                    default:
                        return RpResult::NG_UnknownDelimiter;
                    }
                    break;
                case '>':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_GTEQ;break;
                    case '>':d = &OpTableDef_SHR;break;
                    default:
                        return RpResult::NG_UnknownDelimiter;
                    }
                    break;
                case '<':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_LTEQ;break;
                    case '>':d = &OpTableDef_NOTEQ;break;
                    case '<':d = &OpTableDef_SHL;break;
                    default:
                        return RpResult::NG_UnknownDelimiter;
                    }
                    break;
                case 'O':   //Or
                    switch (*(token.ptr + 1)) {
                    case 'r':d = &OpTableDef_OR;break;
                    default:
                        return RpResult::NG_UnknownDelimiter;
                    }
                    break;
                default:
                    return RpResult::NG_UnknownDelimiter;
                }
                return RpResult::OK;
            }
            if (token.size == 3) {
                if (memcmp(token.ptr, "And", 3) == 0) {
                    d = &OpTableDef_AND;
                }
                else if (memcmp(token.ptr, "Xor", 3) == 0) {
                    d = &OpTableDef_XOR;
                }
                else if (memcmp(token.ptr, "Mod", 3) == 0) {
                    d = &OpTableDef_MOD;
                }
                else if (memcmp(token.ptr, "Not", 3) == 0) {
                    d = &OpTableDef_NOT;
                }
                else {
                    return RpResult::NG_UnknownDelimiter;
                }
                return RpResult::OK;
            }
            return RpResult::NG_UnknownDelimiter;
        }
        /// <summary>
        /// トークンを数値に変換する
        /// </summary>
        /// <param name="d"></param>
        /// <param name="sign"></param>
        /// <returns></returns>
        RpResult asInt(const RawTokenIterator::RawToken_t& token,int& out, int sign = 1)
        {
            return covertResult(token.asInt32(out, sign));
        }
        RpResult asStr(const RawTokenIterator::RawToken_t& token,const MIB_INT8*& out, int& len)const
        {
            out = token.ptr;
            len = token.size;
            return RpResult::OK;
        }
    };
private:
    struct FunctionTable* _func_table = &function_table;
private:
    OpStack<> ops;
    RpQueueImpl<> vs;
public:
    RpResult parse(RawTokenIterator& iter,bool nosolve=false)
    {
        const RawTokenIterator::RawToken_t* token;
        RawTokenParser parser;
        int sign = 0;   //正、負、標準値
        bool is_left_edge = true;
        this->vs.setPushOnly(nosolve);
        for (;;) {
            RawTokenIterator::Result pr = iter.next(token);
            //            printf("%s\n",this->vs.sdump(this->vs));
            switch (pr)
            {
            case RawTokenIterator::Result::OK:break;
            case RawTokenIterator::Result::NG_EOS:
                //OPの払い出し
                for (;;) {
                    const OpDef* w;
                    if (!this->ops.peek(w)) {
                        return RpResult::OK;
                    }
                    this->ops.pop();//常に成功
                    RpResult r = this->vs.pushOp(w);
                    if (r!= RpResult::OK) {
                        return r;  //スタック超過
                    }
                }
                return RpResult::OK;
            }
            if(token->type==RawTokenType::DELIM)
            {
                const OpDef* tmp_delim = NULL;
                auto r = parser.asOpToken(*token, tmp_delim);
                if (r != RpResult::OK) {
                    return r;
                }
                //+と-を一時的に符号として処理
                switch (tmp_delim->delim)
                {
                case DelimType::MINUS:
                    sign = sign==0?-1:-sign;
                    continue;
                case DelimType::PLUS:
                    sign = sign == 0 ? 1 : sign;
                    continue;
                case DelimType::NOT:
                {
                    //単項演算子の場合、signは許可される。
                    //signはLeftEdgeの場合のみ-1*Nに展開する。
                    if (is_left_edge) {
                        if (sign < 0) {
                            //-1*を挿入
                            RpResult r;
                            r = this->vs.pushInt(-1);
                            if (r!= RpResult::OK) {
                                return r;
                            }
                            r = this->ops.push(&OpTableDef_MUL, vs);
                            if (r!= RpResult::OK) {
                                return r;
                            }
                        }
                    }
                    else {
                        if (sign != 0) {
                            const OpDef* op = sign < 0 ? &OpTableDef_MINUS : &OpTableDef_PLUS;
                            RpResult r;
                            r = this->ops.push(op, vs);
                            if (r!= RpResult::OK) {
                                return r;
                            }
                        }
                    }
                    sign = 0;
                    is_left_edge = true;
                    //そのまま演算子を積む
                    RpResult r;
                    r = this->ops.push(tmp_delim, this->vs);
                    if (r!= RpResult::OK) {
                        return r;
                    }
                    continue;
                }

                case DelimType::BRKT_L:
                {
                    //単項演算子、ブラケットLの前にはsignが許可される。
                    //signはLeftEdgeの場合のみ-1*Nに展開する。
                    if (is_left_edge) {
                        if (sign < 0) {
                            //-1*を挿入
                            RpResult r;
                            r= this->vs.pushInt(-1);
                            if (r != RpResult::OK) {
                                return r;
                            }
                            r = this->ops.push(&OpTableDef_MUL, vs);
                            if (r != RpResult::OK) {
                                return r;
                            }
                        }
                    }
                    else {
                        if (sign != 0) {
                            const OpDef* op = sign < 0 ? &OpTableDef_MINUS : &OpTableDef_PLUS;
                            RpResult r = this->ops.push(op, vs);
                            if (r != RpResult::OK) {
                                return r;
                            }
                        }
                    }
                    sign = 0;
                    is_left_edge = true;
                    //そのまま演算子を積む
                    RpResult r = this->ops.push(tmp_delim, this->vs);
                    if (r != RpResult::OK) {
                        return r;
                    }
                    continue;
                }
                case DelimType::COM:
                {
                    //演算子の直前に符号があってはならない。
                    if (sign != 0) {
                        return RpResult::NG_SyntaxError;
                    }
                    is_left_edge = true;// 1,-(1-3)ためして
                    //そのまま演算子を積む
                    RpResult r = this->ops.push(tmp_delim, this->vs);
                    if (r != RpResult::OK) {
                        return r;
                    }
                    continue;
                }
                case DelimType::BRKT_R:
                {
                    //演算子の直前に符号があってはならない。
                    if (sign != 0) {
                        return RpResult::NG_SyntaxError;
                    }
                    //そのまま演算子を積む
                    RpResult r = this->ops.push(tmp_delim, this->vs);
                    if (r!=RpResult::OK) {
                        return r;
                    }
                    continue;
                }
                default:
                {
                    //その他は演算子の直前に符号があってはならない。
                    if (sign != 0) {
                        return RpResult::NG;
                    }
                    is_left_edge = true;
                    //そのまま演算子を積む
                    RpResult r = this->ops.push(tmp_delim, this->vs);
                    if (r!=RpResult::OK) {
                        return r;
                    }
                    continue;
                }
                }
            }
            auto current_left_edge = is_left_edge;
            is_left_edge = false;
            auto current_sign = sign;
            sign = 0;
            switch (token->type) {
            case RawTokenType::NUMBER:
            {
                //符号と演算子のマッチング
                if (current_sign !=0) {
                    if (!current_left_edge) {
                        //LeftEdgeでなければ演算子を積む
                        RpResult r = this->ops.push(&OpTableDef_PLUS, vs);
                        if (r!=RpResult::OK) {
                            return r;
                        }
                    }
                }
                else {
                    current_sign = 1;
                }
                int vi;
                RpResult r;
                r = parser.asInt(*token, vi, current_sign);
                if (r != RpResult::OK) {
                    return r;
                }
                r = this->vs.pushInt(vi);
                if (r != RpResult::OK) {
                    return r;
                }
                break;
            }
            case RawTokenType::STR:
            {
                if (current_left_edge) {
                    if (current_sign != 0) {
                        return RpResult::NG;//Lエッジの文字列に符号がついていたらおかしい
                    }
                }
                else {
                    if (current_sign <= 0) {
                        return RpResult::NG;//文字列は+符号を持つべき
                    }
                    RpResult r = this->ops.push(&OpTableDef_PLUS, vs);
                    if (r!=RpResult::OK) {
                        return r;
                    }
                }
                const MIB_INT8* s = NULL;
                int l = 0;
                RpResult r;
                r= parser.asStr(*token, s, l);
                if (r != RpResult::OK) {
                    return r;
                }
                r = this->vs.pushStr(s, l);
                if (r!=RpResult::OK) {
                    return r;
                }
                break;
            }
            case RawTokenType::TEXT:
            {
                if (current_left_edge) {
                    if (current_sign<0) {
                        RpResult r;
                        r= this->vs.pushInt(-1);
                        //-1*を挿入
                        if (r != RpResult::OK) {
                            return r;
                        }
                        r = this->ops.push(&OpTableDef_MUL, vs);
                        if (r!=RpResult::OK) {
                            return r;
                        }
                    }
                }
                else {
                    if (current_sign < 0) {
                        RpResult r;
                        r = this->ops.push(&OpTableDef_MINUS, vs);
                        if (r!=RpResult::OK) {
                            return r;
                        }
                    }
                    else {
                        RpResult r;
                        r = this->ops.push(&OpTableDef_PLUS, vs);
                        if (r!=RpResult::OK) {
                            return r;
                        }
                    }
                }
                const MIB_INT8* s = NULL;
                int l = 0;
                RpResult r;
                r= parser.asStr(*token, s, l);
                if (r != RpResult::OK) {
                    return r;
                }
                r = this->vs.pushKeyword(s, l);
                if (r != RpResult::OK) {
                    return r;
                }
                break;

            }
            default:
                return RpResult::NG_UnknownToken;
            }

        }
    }

#ifdef TEST
    const char* sdump() {
        return this->vs.sdump(this->vs);
    }

 

#endif
};

}

