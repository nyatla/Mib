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
        NOTEQ = 18   //  != <>
    };


    typedef struct OpDef_t {
        DelimType delim;
        //    MIB_UINT8 shortid;//0から31まで。逆参照に使うからOpTableDefのインデクスと合わせて。
        MIB_UINT8 prio;
    }OpDef;

    const static OpDef OpTableDef[] = {
    OpDef{ DelimType::BRKT_L,  31},
    OpDef{ DelimType::BRKT_R,  31},
    OpDef{ DelimType::MUL,     2 },
    OpDef{ DelimType::DIV,     2 },
    OpDef{ DelimType::MOD,     2 },
    OpDef{ DelimType::PLUS,    3 },
    OpDef{ DelimType::MINUS,   3 },
    OpDef{ DelimType::SHL,     4 },
    OpDef{ DelimType::SHR,     4 },
    OpDef{ DelimType::AND,     5 },
    OpDef{ DelimType::OR ,     5 },
    OpDef{ DelimType::NOT ,    5 },
    OpDef{ DelimType::XOR ,    5 },
    OpDef{ DelimType::LT,      6 },
    OpDef{ DelimType::LTEQ,    6 },
    OpDef{ DelimType::GT,      6 },
    OpDef{ DelimType::GTEQ,    6 },
    OpDef{ DelimType::EQ,      6 },
    OpDef{ DelimType::NOTEQ,   6 }
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





    /// <summary>
    /// 逆ポーランド記法の計算キューです。
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
    template <int QSIZE = 256, int STACKDEPTH = 16> class RpQueue {

    private:
        const static unsigned char TYPE_INT32 = 32;
        const static unsigned char TYPE_INT16 = 33;
        const static unsigned char TYPE_INT8 = 34;
        const static unsigned char TYPE_BOOL_TRUE = 35;
        const static unsigned char TYPE_BOOL_FALSE = 36;

        const static unsigned char TYPE_KWD_LEN = 32-1;//31;
        const static unsigned char TYPE_KWD_MIN = 64;
        const static unsigned char TYPE_KWD_MAX = TYPE_KWD_MIN + TYPE_KWD_LEN;

        const static unsigned char TYPE_SHORT_STR_LEN = 64 - 2;//64-2;
        const static unsigned char TYPE_SHORT_STR_MIN = 128;
        const static unsigned char TYPE_SHORT_STR_MAX = TYPE_SHORT_STR_MIN + TYPE_SHORT_STR_LEN;
        const static unsigned char TYPE_LONG_STR = TYPE_SHORT_STR_MAX + 1;
        //[S][D...] SにサイズとTYPE,Dにデータを格納する。
        //S=0-31    OPType                  1バイトの演算子定数
        //S=32-34   int(4),int(2),int(1)    1+(1-4)バイトの整数
        //S=64-95   keyword 0-31
        //S=128-190 tiny string (0-62)      最大で62文字の文字列
        //S=191     short string            最大で65535文字の文字列。2バイトのサイズ情報付き
        //S=192-254 short bytes
        //S=255     long binaly
        MIB_UINT8 _buf[QSIZE] = {};
        MIB_UINT8 _stack[STACKDEPTH] = {};//格納位置
        int _sp = 0;                //スタックポインタ   
        int _ptr = 0;               //書込みポインタ
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
    public:
        /// <summary>
        /// インスタンスをリセットする
        /// </summary>
        void reset() {
            this->_sp = 0;
            this->_ptr = 0;
            this->_push_only = false;
        }
    private:
        bool _getStackIndex(int idx, int& s)const {
            if (idx >= 0 && idx < this->_sp) {
                s = idx;
            }
            else if (this->_sp + idx >= 0) {
                s = this->_sp + idx;
            }
            else {
                return false;
            }
            return true;
        }
        /// <summary>
        /// n番目の要素のキューメモリ上のポインタを返します。
        /// </summary>
        /// <param name="idx"></param>
        /// <returns></returns>
        MIB_UINT8* ptr(int idx)
        {
            int p;
            if (!_getStackIndex(idx, p)) {
                return NULL;
            }
            return &this->_buf[*(this->_stack + p)];
        }
        /// <summary>
        /// constポインタ版
        /// </summary>
        /// <param name="idx"></param>
        /// <returns></returns>
        const MIB_UINT8* constPtr(int idx)const
        {
            int p;
            if (!_getStackIndex(idx, p)) {
                return NULL;
            }
            return this->_buf + *(this->_stack + p);
        }
    public:
        /// <summary>
        /// メモリブロックをpushします。
        /// </summary>
        /// <param name="d">NULLの場合、スタックポインタのみ移動します。</param>
        /// <param name="s"></param>
        /// <returns></returns>
        bool push(const MIB_INT8* d, int s) {
            auto& ptr = this->_ptr;
            auto& buf = this->_buf;
            if (ptr + s >= QSIZE || this->_sp >= STACKDEPTH) {
                return false;
            }
            if (d != NULL) {
                memmove(buf + ptr, d, s);
            }
            this->_stack[this->_sp++] = ptr;
            this->_ptr += s;
            return true;
        }
        bool pushInt(MIB_INT32 v) {
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
        bool pushKeyword(const MIB_INT8* v, int len) {
            if (len > TYPE_KWD_LEN) {
                return false;
            }

            if (!this->push(NULL, len + 1)) {
                return false;
            }
            auto ptr = this->ptr(-1);
            *(ptr + 0) = 0xff & (len + TYPE_KWD_MIN);
            memmove(ptr + 1, v, len);
            return true;
        }

        bool pushStr(const MIB_INT8* v, int len) {
            if (len <= TYPE_SHORT_STR_LEN) {
                if (!this->push(NULL, len + 1)) {
                    return false;
                }
                auto ptr = this->ptr(-1);
                *(ptr + 0) = 0xff & (len + TYPE_SHORT_STR_MIN);
                memmove(ptr + 1, v, len);
                return true;
            }
            else {
                if (!this->push(NULL, len + 3)) {
                    return false;
                }
                auto ptr = this->ptr(-1);
                *(ptr + 0) = TYPE_LONG_STR;
                ushort2bytes((MIB_UINT16)len, ptr + 1);
                memmove(ptr + 3, v, len);
                return true;
            }
        }
        bool pushBool(bool v) {
            MIB_INT8 d = v ? TYPE_BOOL_TRUE : TYPE_BOOL_FALSE;
            return this->push(&d, 1);
        }
        bool pop(int n)
        {
            if (n > this->_sp) {
                return false;
            }
            this->_sp -= n;
            return true;
        }


        /// <summary>
        /// N番目の要素が取得可能であり、型が一致するかチェックする
        /// </summary>
        /// <param name="idx"></param>
        /// <returns></returns>
        bool isTypeEqual(int idx, int c)const {
            int w;
            if (!this->peekType(idx, w)) {
                return false;
            }
            return w == c;
        }

        /// <summary>
        /// N番目の要素のタイプ値を得る。
        /// </summary>
        /// <param name="idx">-1で末尾。</param>
        /// <returns></returns>
        bool peekType(int idx, int& dst)const
        {
            auto p = this->constPtr(idx);
            if (p == NULL) {
                return false;
            }
            dst = *p;
            return true;
        }
        /// <summary>
        /// N番目の要素からint値を取り出す。
        /// </summary>
        /// <param name="idx"></param>
        /// <param name="out"></param>
        /// <returns></returns>
        bool peekInt(int idx, int& out)const
        {
            auto b = this->constPtr(idx);
            if (b == NULL) {
                return false;
            }
            //型チェック
            switch (b[0])
            {
            case TYPE_INT32:out = bytes2int(b + 1);break;
            case TYPE_INT16:out = bytes2short(b + 1);break;
            case TYPE_INT8:out = (MIB_INT8)b[1];break;
            default:
                return false;
            }
            ;
            return true;
        }
        bool peekBool(int idx, bool& out)const
        {
            auto p = this->constPtr(idx);
            if (p != NULL && (*p == TYPE_BOOL_TRUE || *p == TYPE_BOOL_FALSE)) {
                out = *p == TYPE_BOOL_TRUE;
                return true;
            }
            return false;
        }


        /// <summary>
        /// N番目の要素を文字列と解釈して、文字数とサイズを返す
        /// </summary>
        /// <param name="idx"></param>
        /// <param name="ptr"></param>
        /// <param name="len"></param>
        /// <returns></returns>
        bool peekStr(int idx, const MIB_INT8*& ptr, int& len)const {
            auto b = this->constPtr(idx);
            if (b == NULL) {
                return false;
            }
            //型チェック
            if (TYPE_SHORT_STR_MIN <= b[0] && b[0] <= TYPE_SHORT_STR_MAX) {
                len = b[0] - TYPE_SHORT_STR_MIN;
                ptr = (const MIB_INT8*)b + 1;
                return true;
            }
            if (b[0] == TYPE_LONG_STR) {
                len = bytes2ushort(b + 1);
                ptr = (const MIB_INT8*)b + 3;
                return true;
            }
            return false;
        }
        /// <summary>
        /// N番目の要素を文字列と解釈して、文字数とサイズを返す
        /// </summary>
        /// <param name="idx"></param>
        /// <param name="ptr"></param>
        /// <param name="len"></param>
        /// <returns></returns>
        bool peekKeyword(int idx, const MIB_INT8*& ptr, int& len)const {
            auto b = this->constPtr(idx);
            if (b == NULL) {
                return false;
            }
            //型チェック
            if (TYPE_KWD_MIN <= b[0] && b[0] <= TYPE_KWD_MAX) {
                len = b[0] - TYPE_KWD_MIN;
                ptr = (const MIB_INT8*)b + 1;
                return true;
            }
            return false;
        }
        static inline bool _isIntType(MIB_UINT8 t) { return (t == TYPE_INT32 || t == TYPE_INT16 || t == TYPE_INT8); }
        static inline bool _isStrType(MIB_UINT8 t) { return (t == TYPE_LONG_STR || (TYPE_SHORT_STR_MIN <= t && t <= TYPE_SHORT_STR_MAX)); }
        static inline bool _isKwdType(MIB_UINT8 t) { return (TYPE_KWD_MIN <= t && t <= TYPE_KWD_MAX); }

    private:
        /// <summary>
        /// 文字列の加算関数。
        /// キューの後端は, -1:[+]-2:[STR]-3:[STR]であること。
        /// </summary>
        /// <returns></returns>
        bool _strProc(int t1) {
            //演算子の確認
            if (t1 != (int)DelimType::PLUS) {
                return false;
            }
            //strがどちらかにあること
            int t[2] = {};
            for (int i = 0;i < 2;i++) {
                if (!this->peekType(-2 - i, t[i])) {
                    return false;
                }
            }
            if (!_isStrType(t[0]) && !_isStrType(t[1])) {
                return false;
            }
            int l[2] = { 0,0 };
            const MIB_INT8* s[2] = { NULL,NULL };
            char work[16];
            for (int i = 0;i < 2;i++) {
                if (_isStrType(t[i])) {
                    if (this->peekStr(-2 - i, s[i], l[i])) {
                        continue;
                    }
                }
                //両方がintということはない
                if (_isIntType(t[i])) {
                    int w = 0;
                    if (!this->peekInt(-2 - i, w)) {
                        return false;
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

            this->pop(3);
            //スタックポインタを3個破棄して領域をpush
            if (!this->push(NULL, hsize + sl)) {
                return false;
            }
            auto ptr = this->ptr(-1);
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
            return true;

        }
        bool ii_int(int op, int a, int b, int& d)const
        {
            switch (op) {
            case (int)DelimType::MUL:   d = (b * a);break;
            case (int)DelimType::MINUS: d = (b - a);break;
            case (int)DelimType::PLUS:  d = (b + a);break;
            case (int)DelimType::MOD:   d = (b % a);break;
            case (int)DelimType::DIV:   d = (b / a);break;
            case (int)DelimType::AND:   d = (b & a);break;
            case (int)DelimType::OR:    d = (b | a);break;
            case (int)DelimType::XOR:   d = (b ^ a);break;
            case (int)DelimType::SHL:   d = (b << a);break;
            case (int)DelimType::SHR:   d = (b >> a);break;
            default:
                return false;
            }
            return true;
        }
        bool ii_bool(int op, int a, int b, bool& d)const
        {
            switch (op) {
            case (int)DelimType::LT:    d = (b < a);break;
            case (int)DelimType::LTEQ:  d = (b <= a);break;
            case (int)DelimType::GT:    d = (b > a);break;
            case (int)DelimType::GTEQ:  d = (b >= a);break;
            case (int)DelimType::EQ:    d = (b == a);break;
            case (int)DelimType::NOTEQ: d = (b != a);break;
            default:
                return false;
            }
            return true;
        }
        bool bb_bool(int op, bool a, bool b, bool& d)const
        {
            switch (op) {
            case (int)DelimType::AND:   d = (b && a);break;
            case (int)DelimType::OR:    d = (b || a);break;
            case (int)DelimType::EQ:    d = (b == a);break;
            case (int)DelimType::NOTEQ: d = (b != a);break;
            default:
                return false;
            }
            return true;
        }

        /// <summary>
        /// キューの末尾の演算子を実行する。
        /// </summary>
        /// <returns></returns>
        bool execute()
        {
            auto t1 = 0;
            if (!this->peekType(-1, t1)) {
                return false;
            }

            union {
                int _int;
                bool _bool;
            }d;
            {   //Int,(Int)
                auto a = 0, b = 0;
                if (this->peekInt(-2, a)) {
                    if (this->peekInt(-3, b)) {
                        //int int OP = int  
                        if (ii_int(t1, a, b, d._int))
                        {
                            this->pop(3);
                            this->pushInt(d._int);
                            return true;
                        }
                        //int int OP = bool
                        if (ii_bool(t1, a, b, d._bool))
                        {
                            this->pop(3);
                            this->pushBool(d._bool);
                            return true;
                        }
                    }
                    if (t1 == (int)DelimType::NOT) {
                        this->pop(2);
                        this->pushInt(~a);
                        return true;
                    }
                }
            }
            {   //Bool,(Bool)
                bool a, b;
                if (this->peekBool(-2, a)) {
                    if (this->peekBool(-3, b)) {
                        //int int OP = bool
                        if (bb_bool(t1, a, b, d._bool))
                        {
                            this->pop(3);
                            this->pushBool(d._bool);
                            return true;
                        }
                    }
                    if (t1 == (int)DelimType::NOT) {
                        this->pop(2);
                        this->pushBool(!a);
                        return true;
                    }
                }

            }
            if (this->_strProc(t1)) {
                //strでのplus演算
                return true;
            }
            //全ての演算に失敗したらfalse
            return false;
        }
    public:
        /// <summary>
        /// 演算子をキューに追加し、キューの末尾に対して演算を実行する。
        /// </summary>
        /// <param name="o"></param>
        /// <returns></returns>
        bool pushOp(const OpDef* o) {
            MIB_INT8 c = (MIB_UINT8)o->delim;

            if (!this->push(&c, 1)) {
                return false;
            }
            if (!this->_push_only)
            {
                this->execute();
            }
            return true;
        }
        /// <summary>
        /// trueをセットすると演算を行わない。
        /// </summary>
        /// <param name="f"></param>
        void setPushOnly(bool f) {
            this->_push_only = f;
        }
#ifdef TEST
    public:
        /// <summary>
        /// バッファの内容をダンプする。
        /// バッファオーバーフローとかあまり気にしていないのでデバッグ以外には使わないで。
        /// </summary>
        /// <param name="inst"></param>
        /// <returns></returns>
        static const char* sdump(const RpQueue& inst) {


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
                if (inst.peekBool(i, b)) {
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
        /// </summary>
        /// <param name="v"></param>
        /// <param name="q"></param>
        /// <returns></returns>
        bool push(const OpDef* s, RpQueue<>& q)
        {
            //超過チェック
            if (this->_ptr >= SIZE) {
                return false;
            }
            const OpDef* w;
            if (this->peek(w)) {
                switch (s->delim) {
                case DelimType::BRKT_R:
                    //ここでDelimType::BRKT_Lの存在チェックしないとエラー時にキューが変更されるけどとりあえず無視
                    //払い出しのみ
                    for (;;) {
                        this->pop();
                        if (w->delim == DelimType::BRKT_L) {
                            break;
                        }
                        if (!q.pushOp(w)) {
                            return false;  //スタック超過
                        }
                        if (!this->peek(w)) {
                            return false;  //括弧の対応がおかしい
                        }
                    }
                    return true;
                case DelimType::BRKT_L:
                    //積むだけ
                    break;
                default:
                    //でかけりゃ払い出し
                    if (w->prio <= s->prio) {
                        //払い出し
                        q.pushOp(w);
                        this->pop();
                    }
                    break;
                }
            }
            //積む
            this->_buf[this->_ptr++] = (MIB_UINT8)s->delim;
            return true;
        };
        bool pop() {
            if (this->_ptr == 0) {
                return false;
            }
            this->_ptr--;
            return true;
        };
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
    enum class Result {
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
    };
    static Result covertResult(RawTokenIterator::Result r) {
        switch (r) {
        case RawTokenIterator::Result::OK:return Result::OK;
        case RawTokenIterator::Result::NG_EOS:return Result::NG_EOS;
        case RawTokenIterator::Result::NG_InvalidNumber:return Result::OK;
        case RawTokenIterator::Result::NG_InvalidToken:return Result::OK;
        case RawTokenIterator::Result::NG_NumberRange:return Result::OK;
        default:return Result::NG;
        }
    }

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
        Result asOpToken(const RawTokenIterator::RawToken_t& token,const OpDef*& d)
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
                default:
                    return Result::NG_UnknownDelimiter;
                }
                return Result::OK;
            }
            if (token.size == 2) {
                switch (*token.ptr) {
                case '!':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_NOTEQ;break;
                    default:
                        return Result::NG_UnknownDelimiter;
                    }
                    break;
                case '=':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_EQ;break;
                    default:
                        return Result::NG_UnknownDelimiter;
                    }
                    break;
                case '>':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_GTEQ;break;
                    case '>':d = &OpTableDef_SHR;break;
                    default:
                        return Result::NG_UnknownDelimiter;
                    }
                    break;
                case '<':
                    switch (*(token.ptr + 1)) {
                    case '=':d = &OpTableDef_LTEQ;break;
                    case '>':d = &OpTableDef_NOTEQ;break;
                    case '<':d = &OpTableDef_SHL;break;
                    default:
                        return Result::NG_UnknownDelimiter;
                    }
                    break;
                case 'O':   //Or
                    switch (*(token.ptr + 1)) {
                    case 'r':d = &OpTableDef_OR;break;
                    default:
                        return Result::NG_UnknownDelimiter;
                    }
                    break;
                default:
                    return Result::NG_UnknownDelimiter;
                }
                return Result::OK;
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
                    return Result::NG_UnknownDelimiter;
                }
                return Result::OK;
            }
            return Result::NG_UnknownDelimiter;
        }
        /// <summary>
        /// トークンを数値に変換する
        /// </summary>
        /// <param name="d"></param>
        /// <param name="sign"></param>
        /// <returns></returns>
        Result asInt(const RawTokenIterator::RawToken_t& token,int& out, int sign = 1)
        {
            return covertResult(token.asInt32(out, sign));
        }
        Result asStr(const RawTokenIterator::RawToken_t& token,const MIB_INT8*& out, int& len)const
        {
            out = token.ptr;
            len = token.size;
            return Result::OK;
        }
    };
private:
    OpStack<> ops;
    RpQueue<> vs;
public:
    Result parse(RawTokenIterator& iter, int depth = 0,bool nosolve=false)
    {
        const RawTokenIterator::RawToken_t* token;
        RawTokenParser parser;
        int sign = 1;
        bool hassign = false;       //演算子を持つか
        bool is_need_sign = false;  //最後に読みだしたのが符号であるか
        this->vs.setPushOnly(nosolve);
        //const OpDef* last_delim = NULL;//符号以外の直前に来たデリミタを記録
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
                        return Result::OK;
                    }
                    this->ops.pop();//常に成功
                    if (!this->vs.pushOp(w)) {
                        return Result::NG_StackOverFlow;  //スタック超過
                    }
                }
                return Result::OK;
            }
            switch (token->type) {
            case RawTokenType::DELIM:
            {
                const OpDef* tmp_delim = NULL;
                auto r = parser.asOpToken(*token,tmp_delim);
                if (r != Result::OK) {
                    return r;
                }
                //連続する演算子の統合
                switch (tmp_delim->delim)
                {
                case DelimType::MINUS:
                    sign *= -1;
                case DelimType::PLUS:
                    if (!hassign && is_need_sign) {
                        if (!this->ops.push(&OpTableDef_PLUS, vs)) {
                            return Result::NG;
                        }
                    }
                    hassign = true;
                    continue;
                }
                hassign = false;
                is_need_sign = false;
                //TODO signを考慮して
                //符号以外のデリミタがきた
                switch (tmp_delim->delim) {
                case DelimType::BRKT_L:
                    if (sign < 0) {
                        //-1*を挿入
                        if (!this->vs.pushInt(-1)) {
                            return Result::NG_StackOverFlow;
                        }
                        if (!this->ops.push(&OpTableDef_MUL, vs)) {
                            return Result::NG;
                        }
                        sign = 1;//リセット
                    }
                    if (!this->ops.push(tmp_delim, this->vs)) {
                        return Result::NG;
                    }
                    continue;;
                case DelimType::BRKT_R:
                    if (!this->ops.push(tmp_delim, this->vs)) {
                        return Result::NG;
                    }
                    is_need_sign = true;
                    continue;
                }
                {
                    if (!this->ops.push(tmp_delim, vs)) {
                        return Result::NG;
                    }
                }
                break;
            }
            case RawTokenType::NUMBER:
            {
                hassign = false;
                is_need_sign = true;
                int vi;
                {
                    auto r = parser.asInt(*token,vi, sign);
                    sign = 1;//符号の初期化
                    if (r != Result::OK) {
                        return r;
                    }
                }
                if (!this->vs.pushInt(vi)) {
                    return Result::NG_StackOverFlow;
                }
                break;
            }
            case RawTokenType::STR:
            {
                hassign = false;
                is_need_sign = true;
                const MIB_INT8* s = NULL;
                int l = 0;
                auto r = parser.asStr(*token, s, l);
                if (r != Result::OK) {
                    return r;
                }
                if (!this->vs.pushStr(s, l)) {
                    return Result::NG_StackOverFlow;
                }
                break;
            }
            case RawTokenType::TEXT:
            {
                hassign = false;
                is_need_sign = true;
                const MIB_INT8* s = NULL;
                int l = 0;
                auto r = parser.asStr(*token, s, l);
                if (r != Result::OK) {
                    return r;
                }
                if (!this->vs.pushKeyword(s, l)) {
                    return Result::NG_StackOverFlow;
                }
                break;
            }
            default:
                return Result::NG_UnknownToken;
            }

        }
    }

#ifdef TEST
    const char* sdump() {
        return this->vs.sdump(this->vs);
    }

    static void test() {
        struct local {
            static void parse(const char* s, const char* a) {
                RpSolver rp;
                RawTokenIterator rti(s);
                auto r = rp.parse(rti);
                const char* m = rp.vs.sdump(rp.vs);
                if (r == Result::OK) {
                    printf("%s -> %s(%s)\n", s, m, memcmp(a, m, strlen(a)) == 0 ? "OK" : "NG");
                }
                else {
                    printf("ERR %d %s\n", r, m);
                }
            }
        };
        local::parse("1+2*3)", "10");

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
        return;
    }
#endif
};

}

