// ConsoleApplication1.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#define TEST 1
#include "./CharReader.h"
#include "./RawTokenIterator.h"
#include <string.h>

using namespace MIB;




class ErrorMessage{
public:
    const MIB_UINT8 eid;
    const char* message;
public:
    ErrorMessage(MIB_UINT8 eid_, const char* message_) :eid(eid_), message(message_) {}
};





/// <summary>
/// 0-31まで
/// </summary>
enum class DelimType:MIB_UINT8 {
    BRKT_L=0,   //  (
    BRKT_R=1,   //  )
    MUL   =2,   //  *
    DIV   =3,   //  /
    MOD   =4,   //  %
    PLUS  =5,   //  +
    MINUS =6,   //  -
    SHL   =7,   //  >>
    SHR  = 8,   //  <<
    LT   = 9,   //  <
    LTEQ = 10,  //  <=
    GT   = 11,  //  >
    GTEQ = 12,  //  >=
    EQ   = 13,  //  ==
    NOTEQ= 14   //  != <>
};


typedef struct OpDef_t {
    DelimType delim;
//    MIB_UINT8 shortid;//0から31まで。逆参照に使うからOpTableDefのインデクスと合わせて。
    MIB_UINT8 prio;
}OpDef;

const static OpDef OpTableDef[]={
    OpDef{ DelimType::BRKT_L,  31},
    OpDef{ DelimType::BRKT_R,  31},
    OpDef{ DelimType::MUL,     2 },
    OpDef{ DelimType::DIV,     2 },
    OpDef{ DelimType::MOD,     2 },
    OpDef{ DelimType::PLUS,    3 },
    OpDef{ DelimType::MINUS,   3 },
    OpDef{ DelimType::SHL,     4 },
    OpDef{ DelimType::SHR,     4 },
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
#define OpTableDef_LT (OpTableDef[(int)DelimType::LT])
#define OpTableDef_LTEQ (OpTableDef[(int)DelimType::LTEQ])
#define OpTableDef_GT (OpTableDef[(int)DelimType::GT])
#define OpTableDef_GTEQ (OpTableDef[(int)DelimType::GTEQ])
#define OpTableDef_EQ (OpTableDef[(int)DelimType::EQ])
#define OpTableDef_NOTEQ (OpTableDef[(int)DelimType::NOTEQ])






class RawTokenParser {
private:
    const struct RawToken_t& _token;
public:
    RawTokenParser(const struct RawToken_t& token):_token(token){
    }
    ParserResult asOpToken(const OpDef*& d)
    {
        auto& token = this->_token;
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
                return ParserResult::NG_UnknownDelimiter;
            }
            return ParserResult::OK;
        }
        if (token.size == 2) {
            switch (*token.ptr) {
            case '!':
                switch (*(token.ptr + 1)) {
                case '=':d = &OpTableDef_NOTEQ;break;
                default:
                    return ParserResult::NG_UnknownDelimiter;
                }
                break;
            case '=':
                switch (*(token.ptr + 1)) {
                case '=':d = &OpTableDef_EQ;break;
                default:
                    return ParserResult::NG_UnknownDelimiter;
                }
                break;
            case '>':
                switch (*(token.ptr + 1)) {
                case '=':d = &OpTableDef_GTEQ;break;
                default:
                    return ParserResult::NG_UnknownDelimiter;
                }
                break;
            case '<':
                switch (*(token.ptr + 1)) {
                case '=':d = &OpTableDef_LTEQ;break;
                case '>':d = &OpTableDef_NOTEQ;break;
                default:
                    return ParserResult::NG_UnknownDelimiter;
                }
                break;
            default:
                return ParserResult::NG_UnknownDelimiter;
            }
            return ParserResult::OK;
        }
        return ParserResult::NG_UnknownDelimiter;
    }
    /// <summary>
    /// トークンを数値に変換する
    /// </summary>
    /// <param name="d"></param>
    /// <param name="sign"></param>
    /// <returns></returns>
    ParserResult asInt(int& out, int sign = 1)
    {
        auto& token = this->_token;
        int t = 0;
        int i;
        if (sign > 0) {
            for (i = 0;i<token.size;i++) {
                int d = token.ptr[i] - '0';
                if (0 > d || d > 9) {
                    return ParserResult::NG_InvalidNumber;
                }

                if (t > 0 && (INT32_MAX - t - d) / t < 9) {
                    return ParserResult::NG_NumberRange;
                }
                t = t * 10 + d;
            };
        }
        else {
            for (i = 0;i<token.size;i++) {
                int d = token.ptr[i] - '0';
                if (0 > d || d > 9) {
                    return ParserResult::NG_InvalidNumber;
                }
                if (t < 0 && (INT_MIN - t + d) / t < 9) {
                    return ParserResult::NG_NumberRange;
                }
                t = t * 10 - d;
            };
        }
        out = t;
        return i > 0 ? ParserResult::OK : ParserResult::NG;
    }
    ParserResult asStr(const MIB_INT8*& out, int& len)const
    {
        auto& token = this->_token;
        out=token.ptr;
        len = token.size;
        return ParserResult::OK;
    }
};










template <int SIZE = 16> class OpStack {
private:
    MIB_UINT8 _buf[SIZE] = {};
    int _ptr = 0;//次の格納位置
public:
    bool push(const OpDef* v)
    {
        if (this->_ptr >= SIZE) {
            return false;
        }
        this->_buf[this->_ptr++] = (MIB_UINT8)v->delim;
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
        d = &OpTableDef[this->_buf[this->_ptr-1]];
        return true;
    };
    bool isEmpty() {
        return this->_ptr == 0;
    }
    int ptr() { return this->_ptr; }

};



/// <summary>
/// 逆ポーランド記法の計算キューです。
/// 演算子 +-*/%()
/// 値型  INT32 STRING BOOL
/// 
/// 
/// </summary>
/// <typeparam name="QSIZE"></typeparam>
/// <typeparam name="STACKDEPTH"></typeparam>
template <int QSIZE = 256,int STACKDEPTH=16> class RpoQueue{

private:
    const static unsigned char TYPE_INT32 = 32;
    const static unsigned char TYPE_INT16 = 33;
    const static unsigned char TYPE_INT8 = 34;
    const static unsigned char TYPE_BOOL_TRUE = 35;
    const static unsigned char TYPE_BOOL_FALSE = 36;

    const static unsigned char TYPE_SHORT_STR_LEN = 64-2;//64-2;
    const static unsigned char TYPE_SHORT_STR_MIN = 128;
    const static unsigned char TYPE_SHORT_STR_MAX = TYPE_SHORT_STR_MIN+ TYPE_SHORT_STR_LEN;
    const static unsigned char TYPE_LONG_STR = TYPE_SHORT_STR_MAX+1;
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
    int _sp=0;        //スタックポインタ   
    int _ptr = 0;   //書込みポインタ
private:
    bool _getStackIndex(int idx,int &s)const{
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
        return this->_buf+*(this->_stack + p);
    }

public:
    /// <summary>
    /// 
    /// </summary>
    /// <param name="d">NULLの場合、スタックポインタのみ移動します。</param>
    /// <param name="s"></param>
    /// <returns></returns>
    bool push(const MIB_INT8* d, int s) {
        auto& ptr = this->_ptr;
        auto& buf = this->_buf;
        if (ptr + s >= QSIZE ||this->_sp>= STACKDEPTH) {
            return false;
        }
        if (d != NULL) {
            memmove(buf + ptr, d, s);
        }
        this->_stack[this->_sp++] = ptr;
        this->_ptr += s;
        return true;
    }
    bool pushInt(MIB_INT32 v){
        if (v > INT16_MAX || v < INT16_MIN) {
            MIB_INT8 d[5] = { TYPE_INT32 ,};
            int2bytes(v, d + 1);
            return this->push(d, 5);
        }
        else if(v > INT8_MAX || v < INT8_MIN) {
            MIB_INT8 d[3] = { TYPE_INT16, };
            short2bytes(v, d + 1);
            return this->push(d, 3);
        }
        else {
            MIB_INT8 d[2] = {TYPE_INT8, v & 0xff};
            return this->push(d, 2);
        }
    }
    bool pushStr(const MIB_INT8* v,int len) {
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




    /// <summary>
    /// N番目のスタックのタイプ値を得る。
    /// </summary>
    /// <param name="idx">-1で末尾。</param>
    /// <returns></returns>
    bool peekType(int idx,int& dst)const
    {
        auto p = this->constPtr(idx);
        if (p == NULL) {
            return false;
        }
        dst = *p;
        return true;
    }
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
        case TYPE_INT8:out =  (MIB_INT8)b[1];break;
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
    /// 文字列と解釈して、文字数とサイズを返す
    /// </summary>
    /// <param name="idx"></param>
    /// <param name="ptr"></param>
    /// <param name="len"></param>
    /// <returns></returns>
    bool peekStr(int idx, const MIB_INT8*& ptr, int& len)const{
        auto b = this->constPtr(idx);
        if (b == NULL) {
            return false;
        }
        //型チェック
        if (TYPE_SHORT_STR_MIN <= b[0] && b[0] <= TYPE_SHORT_STR_MAX) {
            len = b[0] - TYPE_SHORT_STR_MIN;
            ptr = (const MIB_INT8*)b+1;
            return true;
        }
        if (b[0] == TYPE_LONG_STR) {
            len=bytes2ushort(b + 1);
            ptr = (const MIB_INT8*)b+3;
            return true;
        }
        return false;
    }

    static inline bool _isIntType(MIB_UINT8 t) { return (t == TYPE_INT32 || t == TYPE_INT16 || t == TYPE_INT8); }
    static inline bool _isStrType(MIB_UINT8 t) { return (t == TYPE_LONG_STR || (TYPE_SHORT_STR_MIN <= t && t <= TYPE_SHORT_STR_MAX)); }

    /// <summary>
    /// スタック先頭の２要素を、整数として取得する。
    /// スタックの要素は両方とも整数であること。
    /// </summary>
    /// <param name="v1"></param>
    /// <param name="v2"></param>
    /// <returns></returns>
    bool _popII(int& v1,int& v2)
    {
        auto t1 = 0, t2 = 0;
        if (!this->peekType(-1, t1) || !this->peekType(-2, t2)) {
            return false;
        }
        if (t1 == t2 && _isIntType(t1)) {
            if (this->peekInt(-1, v1) && !this->peekInt(-2, v2)) {
                return false;
            }
            this->_sp -= 2;
            return true;
        }
        return false;
    }
    /// <summary>
    /// 文字列の加算関数
    /// スタック先頭の2要素のうち、どちらから文字列であれば、結合した文字列１つに統合する。
    /// </summary>
    /// <returns></returns>
    bool _strPlus() {
        //strがどちらかにあること
        int t[2] = {};
        for (int i = 0;i < 2;i++) {
            if (!this->peekType(-1 - i, t[i])) {
                return false;
            }
        }
        if(!_isStrType(t[0]) && !_isStrType(t[1]) ){
            return false;
        }
        int l[2] = { 0,0 };
        const MIB_INT8* s[2] = { NULL,NULL };
        char work[16];
        for (int i = 0;i < 2;i++) {
            if (_isStrType(t[i])) {
                if (this->peekStr(-1 - i, s[i], l[i])) {
                    continue;
                }
            }
            //両方がintということはない
            if (_isIntType(t[i])) {
                int w=0;
                if (!this->peekInt(-1 - i, w)) {
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

        this->_sp -= 2;
        //スタックポインタを2個破棄して領域をpush
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
    bool execute()
    {
        for (;;) {
            auto t1 = 0;
            if (!this->peekType(-1, t1)) {
                return false;
            }
            {   //popIIで可能な演算                
                this->_sp--;
                auto a = 0, b = 0;
                if (this->_popII(a, b)) {
                    switch (t1) {
                    case (int)DelimType::MUL:
                        this->pushInt(b * a);
                        continue;
                    case (int)DelimType::MINUS:
                        this->pushInt(b - a);
                        continue;
                    case (int)DelimType::PLUS:
                        this->pushInt(b + a);
                        continue;
                    case (int)DelimType::MOD:
                        this->pushInt(b % a);
                        continue;
                    case (int)DelimType::DIV:
                        this->pushInt(b / a);
                        continue;
                    case (int)DelimType::LT:
                        this->pushBool(b < a);
                        continue;
                    case (int)DelimType::LTEQ:
                        this->pushBool(b <= a);
                        continue;
                    case (int)DelimType::GT:
                        this->pushBool(b > a);
                        continue;
                    case (int)DelimType::GTEQ:
                        this->pushBool(b >= a);
                        continue;
                    case (int)DelimType::EQ:
                        this->pushBool(b == a);
                        continue;
                    case (int)DelimType::NOTEQ:
                        this->pushBool(b != a);
                        continue;
                    }
                }
                this->_sp++;
            } 
            {
                this->_sp--;
                if (t1 == (int)DelimType::PLUS && this->_strPlus()) {
                    //strでのplus演算
                    continue;
                }
                this->_sp++;
            }
            //全ての演算に失敗したらfalse
            return true;
        }
    }
    bool pushOp(const OpDef* o) {
        MIB_INT8 c = (MIB_UINT8)o->delim;
        
        if (!this->push(&c, 1)) {
            return false;
        }
        this->execute();
        return true;
    }
    bool isEmpty() {
        return this->_ptr == 0;
    }

    /// <summary>
    /// opsスタックにSをマージする
    /// </summary>
    /// <param name="ops"></param>
    /// <param name="s"></param>
    ParserResult marge(OpStack<>& ops, const OpDef* s)
    {
        const OpDef* w = NULL;
        if (!ops.peek(w)) {
            //スタックが空なら追記
        }
        else if (w->prio > s->prio) {
            //sが優先度の高い演算子ならそのまま積む
        }
        else {
            for (;;) {
                //Lブラケットならおわり
                if (w->delim == DelimType::BRKT_L) {
                    break;
                }
                //優先度が同じか低い演算子なら払い出し
                if (!this->pushOp(w)) {
                    return ParserResult::NG;  //スタック超過
                }
                ops.pop();//常に成功
                if (!ops.peek(w)) {
                    break;
                }
            }
        }
        if (!ops.push(s)) {
            return ParserResult::NG;  //スタック超過
        }
        return ParserResult::OK;
    }
    
    /// <summary>
    /// バッファの内容をダンプする。
    /// バッファオーバーフローとかあまり気にしていないのでデバッグ以外には使わないで。
    /// </summary>
    /// <param name="inst"></param>
    /// <returns></returns>
    static const char* sdump(const RpoQueue& inst) {


        static char strbuf[256];
        char* str = strbuf;
        auto& b = inst._buf;
        for (int i = 0;i < inst._sp;i++) {            
            int p = 0;
            inst.peekType(i,p);
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
            bool b;
            if (inst.peekBool(i, b)) {
                str = str + sprintf(str, "%s ", b?"TRUE":"FALSE");
                continue;

            }
            str = str + sprintf(str, "x ");break;
        }
        sprintf(str,"\0");
        return strbuf;
    }

};

class Rp {
private:
    OpStack<> ops;
    RpoQueue<> vs;
public:
    ParserResult parse(RawTokenIterator& iter,int depth=0)
    {
        struct RawToken_t token;
        RawTokenParser parser(token);
        int sign = 1;
        bool hassign = false;
        bool is_need_sign = false; //最後に読みだしたのが符号であるか
        //const OpDef* last_delim = NULL;//符号以外の直前に来たデリミタを記録
        for (;;) {
            ParserResult pr = iter.next(token);
            switch (pr)
            {
            case ParserResult::OK:break;
            case ParserResult::NG_EOF:
                //OPの払い出し
                for (;;) {
                    const OpDef* w;
                    if (!this->ops.peek(w)) {
                        return ParserResult::OK;
                    }
                    this->ops.pop();//常に成功
                    if (!this->vs.pushOp(w)) {
                        return ParserResult::NG;  //スタック超過
                    }
                }
                return ParserResult::OK;
            }
            switch (token.type) {
            case RawTokenType::DELIM:
            {
                const OpDef* tmp_delim = NULL;
                auto r = parser.asOpToken(tmp_delim);
                if (r != ParserResult::OK) {
                    return ParserResult::NG;
                }
                //連続する演算子の統合
                switch (tmp_delim->delim)
                {
                case DelimType::MINUS:
                    sign *= -1;
                case DelimType::PLUS:
                    if (!hassign && is_need_sign) {
                        r = this->vs.marge(ops, &OpTableDef_PLUS);
                        if (r != ParserResult::OK) {
                            return r;
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
                            return ParserResult::NG_StackOverFlow;
                        }
                        r = this->vs.marge(ops, &OpTableDef_MUL);
                        if (r != ParserResult::OK) {
                            return r;
                        }
                        sign = 1;//リセット
                    }
                case DelimType::LT:
                    if (!this->ops.push(tmp_delim)) {
                        return ParserResult::NG_StackOverFlow;
                    }
                    continue;
                case DelimType::BRKT_R:
                    for (;;) {
                        const OpDef* w;
                        if (!this->ops.peek(w)) {
                            return ParserResult::NG;  //括弧の対応がおかしい
                        }
                        this->ops.pop();//常に成功
                        if (w->delim == DelimType::BRKT_L) {
                            is_need_sign = true;
                            break;
                        }
                        if (!this->vs.pushOp(w)) {
                            return ParserResult::NG;  //スタック超過
                        }
                    }
                    continue;
                }
                {
                    auto r = this->vs.marge(ops, tmp_delim);
                    if (r != ParserResult::OK) {
                        return r;
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
                    auto r = parser.asInt(vi, sign);
                    sign = 1;//符号の初期化
                    if (r != ParserResult::OK) {
                        return r;
                    }
                }
                if (!this->vs.pushInt(vi)) {
                    return ParserResult::NG_StackOverFlow;
                }
                break;
            }
            case RawTokenType::STR:
            {
                hassign = false;
                is_need_sign = true;
                const MIB_INT8 * s=NULL;
                int l=0;
                auto r = parser.asStr(s, l);
                if (r != ParserResult::OK) {
                    return r;
                }
                if (!this->vs.pushStr(s,l)) {
                    return ParserResult::NG_StackOverFlow;
                }
                break;
            }
            default:
                return ParserResult::NG_UnknownToken;
            }

        }
    }
    static void test() {
        struct local {
            static void parse(const char* s,const char* a) {
                Rp rp;
                RawTokenIterator rti(s);
                auto r = rp.parse(rti);
                if (r==ParserResult::OK) {
                    const char* r = rp.vs.sdump(rp.vs);
                    printf("%s -> %s(%s)\n", s, r, memcmp(a, r, strlen(a)) == 0 ? "OK" : "NG");
                }
                else {
                    printf("ERR %d\n", r);
                }
            }
        };
        //INT32 TEST
        //local::parse("1+127+32767+2147483647"   ,"-2147450754");
        //local::parse("-1-128-32768-2147483648"  ,"2147450751");
        //local::parse("1+(2+3+(4+5+6)+7)", "28");
        //local::parse("-1+(-2+-3+(-4+-5+-6)+-7)", "-28");
        //local::parse("-1+-2"        ,"-3");
        //local::parse("1+2"          ,"3");
        //local::parse("-1+-2*-3"     ,"5");
        //local::parse("1+2*3"        ,"7");
        //local::parse("-1+-2*-3+-4"  ,"1");
        //local::parse("1+2*3+4"      ,"11");
        //local::parse("-1+(-2+-3)"   ,"-6");
        //local::parse("1+(2+3)"      ,"6");
        //local::parse("1*-(2*3)"     ,"-6");
        //local::parse("1*--(2*3)"    ,"6");
        //local::parse("-1+(-2+-3)"   ,"-6");
        //local::parse("-1*+(-2+-3)"  ,"5");
        //local::parse("-10/(-2+-3)", "2");
        //local::parse("-10%(-2+-3)", "0");

        ////chatGPT generated test
        //local::parse("3+4*2", "11");         // 3 + 4 * 2 = 11
        //local::parse("(7+3)*2", "20");       // (7 + 3) * 2 = 20
        //local::parse("1+2+3+4+5", "15");     // 1 + 2 + 3 + 4 + 5 = 15
        //local::parse("8*4-6/2", "29");       // 8 * 4 - 6 / 2 = 16 (修正)
        //local::parse("(1+2)*(3+4)", "21");   // (1 + 2) * (3 + 4) = 21
        //local::parse("-2*4+3", "-5");        // -2 * 4 + 3 = -5
        //local::parse("-(3+4)*2", "-14");     // -(3 + 4) * 2 = -14
        //local::parse("-2+3*4", "10");        // -2 + 3 * 4 = 10
        //local::parse("-(2+3*4)", "-14");     // -(2 + 3 * 4) = -14
        //local::parse("---2", "-2");          // -(-(-2)) = -2
        //local::parse("--2*3", "6");          // -(-2) * 3 = 6
        //local::parse("(((2+3*3)*4))", "44");   // (((2 + 3) * 4)) = 20
        //local::parse("((2+3)*4+(5-2))", "23"); // ((2 + 3) * 4 + (5 - 2)) = 21
        //local::parse("(2+3)*(4+5)", "45");   // (2 + 3) * (4 + 5) = 45
        //local::parse("((2+3)*(4+5))*(3-1)", "90"); // ((2 + 3) * (4 + 5)) * (3 - 1) = 90

        local::parse("\"ABCDE\"+\"FG\"", "\"ABCDEFG\"");
        local::parse("\"ABCDE\"+1+2-3", "\"ABCDE12-3\"");
        local::parse("\"ABCDE\"+1+(2+3)", "\"ABCDE15\"");
        local::parse("\"AB\"+1+(2+3+4)", "\"AB19\"");
        local::parse("1+(2+\"AB\"+3)", "\"12AB3\"");
        local::parse("1+(2+\"AB\"+3*-2)", "\"12AB-6\"");
        local::parse("1<3", "TRUE");
        local::parse("3<3", "FALSE");
        local::parse("3<3+1", "TRUE");
//        local::parse("(3<3)+1", "TRUE");
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



        return;
    }
};



int main()
{
    Rp::test();
    
    //RawTokenIterator::test();
}


