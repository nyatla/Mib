// ConsoleApplication1.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#define TEST 1
#include "./CharReader.h"
#include "./RawTokenIterator.h"

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
    BRKT_L=0,
    BRKT_R=1,
    MUL   =2,
    DIV   =3,
    MOD   =4,
    PLUS  =5,
    MINUS =6,
    SHL   =7,
    SHR   =8,
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
    OpDef{ DelimType::SHR,     4 }
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
            default:
                return ParserResult::NG_UnknownDelimiter;
            }
        }
        else {
            return ParserResult::NG_UnknownDelimiter;
        }
        return ParserResult::OK;
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




template <int QSIZE = 256,int STACKDEPTH=16> class VStack{

private:
    const static unsigned char TYPE_INT32 = 32;
    const static unsigned char TYPE_INT16 = 33;
    const static unsigned char TYPE_INT8  = 34;
    //[D...][S] Dにデータ,SにサイズとTYPEを格納する。
    //T=0-31    OPType
    //T=32      32bit int(4)
    //T=127-254 STR (T-127)
    MIB_UINT8 _buf[QSIZE] = {};
    MIB_UINT8 _stack[STACKDEPTH] = {};//格納位置
    int _sp=0;        //スタックポインタ   
    int _ptr = 0;   //書込みポインタ
private:
    static int _readInt(const MIB_UINT8* b,int& out) {
        switch (b[0]) {
        case TYPE_INT32:
            out=(((MIB_INT32)b[1]) << 24) | (((MIB_INT32)b[2]) << 16) | (((MIB_INT32)b[3]) << 8) | (((MIB_INT32)b[4]));
            break;
        case TYPE_INT16:
            out = (((MIB_INT32)b[1]) << 8) | (((MIB_INT32)b[2]));
            break;
        case TYPE_INT8:
            out = (MIB_INT32)b[1];
            break;
        default:
            return false;
        }
        return true;
    }
public:
    bool push(const MIB_INT8* d, size_t s) {
        auto& ptr = this->_ptr;
        auto& buf = this->_buf;
        if (ptr + s >= QSIZE ||this->_sp>= STACKDEPTH) {
            return false;
        }
        memcpy(buf+ptr, d, s);
        this->_stack[this->_sp++] = ptr;
        this->_ptr += s;
        return true;
    }
    bool pushInt(MIB_INT32 v){
        if (v > INT16_MAX || v < INT16_MIN) {
            MIB_INT8 d[5] = {
                TYPE_INT32, (v >> 24) & 0xff, (v >> 16) & 0xff, (v >> 8) & 0xff, v & 0xff
            };
            return this->push(d, 5);
        }
        else if(v > INT8_MAX || v < INT8_MIN) {
            MIB_INT8 d[3] = {
                TYPE_INT16, (v >> 8) & 0xff, v & 0xff
            };
            return this->push(d, 3);
        }
        else {
            MIB_INT8 d[2] = {
                TYPE_INT8, v & 0xff
            };
            return this->push(d, 2);
        }
    }
    /// <summary>
    /// N番目のスタックのタイプ値を得る。
    /// </summary>
    /// <param name="idx">-1で末尾。</param>
    /// <returns></returns>
    bool peekType(int idx,int& dst)const
    {
        int p;
        if (idx >= 0 && idx<this->_sp) {
            p=idx;
        }
        else if(this->_sp - idx > 0) {
            p=this->_sp + idx;
        }
        else {
            return false;
        }
        dst = this->_buf[*(this->_stack+p)];
        return true;
    }
    bool peekInt(int idx, int& out)const
    {
        int p;
        if (idx >= 0 && idx < this->_sp) {
            p = idx;
        }
        else if (this->_sp - idx > 0) {
            p = this->_sp + idx;
        }
        else {
            return false;
        }
        auto b = &this->_buf[*(this->_stack+p)];
        //型チェック
        switch (b[0])
        {
        case TYPE_INT32:out = (MIB_INT32)(((MIB_UINT32)b[1]) << 24) | (((MIB_UINT32)b[2]) << 16) | (((MIB_UINT32)b[3]) << 8) | b[4];break;
        case TYPE_INT16:out = (MIB_INT32)(((MIB_UINT32)b[1]) << 8) | b[2];break;
        case TYPE_INT8:out =  (MIB_INT32)b[1];break;
        default:
            return false;
        }
        ;
        return true;
    }
   
    //両方ともint変換可能型であること
    static bool _isII(const MIB_UINT8 v1, const MIB_UINT8 v2) { return v1==v2 && (v1 == TYPE_INT32 || v1 == TYPE_INT16 || v1 == TYPE_INT8); }
    bool execute()
    {
        for (;;) {
            auto t1 = 0;
            if (!this->peekType(-1, t1)) {
                return false;
            }
            switch (t1) {
            case (int)DelimType::PLUS:
            {
                auto t2 = 0, t3 = 0;
                if (!this->peekType(-2, t2) || !this->peekType(-3, t3)) {
                    return false;
                }
                // II,SI,FIが計算可能。他の場合は事前展開。
                if (_isII(t2, t3)) {
                    int a = 0, b = 0;
                    if (!this->peekInt(-2, a) || !this->peekInt(-3, b)) {
                        return false;
                    }
                    this->_sp -= 3;
                    this->pushInt(a + b);
                }
                else {
                    return false;
                }
                continue;
            }
            default:
                return false;
            }
            return true;
        }
    }
    bool pushOp(const OpDef* o) {
        MIB_INT8 c = (MIB_UINT8)o->delim;
        
        if (!this->push(&c, 1)) {
            return false;
        }
        //return false?true:this->execute();
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
            //otが優先度の高い演算子ならそのまま積む
        }
        else {
            for (;;) {
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

    //bool getOp(const OpDef*& d) {
    //    if (this->ptr == 0) {
    //        return false;
    //    }
    //    auto c = *(this->ptr - 1);
    //    if (c > 0 && c < 32) {
    //        d = &OpTableDef[c];
    //        return true;
    //    }
    //    return false;
    //}
    
    /// <summary>
    /// バッファの内容をダンプする。
    /// バッファオーバーフローとかあまり気にしていないのでデバッグ以外には使わないで。
    /// </summary>
    /// <param name="inst"></param>
    /// <returns></returns>
    static const char* sdump(const VStack& inst) {


        static char strbuf[256];
        char* str = strbuf;
        auto& b = inst._buf;
        for (int i = 0;i < inst._sp;i++) {            
            int p = 0;
            inst.peekType(i,p);
            switch (p) {
            case (int)DelimType::BRKT_L:str= str+sprintf(str,"(");break;
            case (int)DelimType::BRKT_R:str = str + sprintf(str, ")");break;
            case (int)DelimType::MUL:str = str + sprintf(str, "*");break;
            case (int)DelimType::DIV:str = str + sprintf(str, "/");break;
            case (int)DelimType::MOD:str = str + sprintf(str, "%%");break;
            case (int)DelimType::PLUS:str = str + sprintf(str, "+");break;
            case (int)DelimType::MINUS:str = str + sprintf(str, "-");break;
            case TYPE_INT32:
            case TYPE_INT16:
            case TYPE_INT8:
            {
                int v=0;
                inst.peekInt(i,v);
                str = str + sprintf(str, "%d",v);break;
                break;
            }
            default:
                str = str + sprintf(str, "x");break;
            }
            *str = ' ';
            str = str + 1;
        }
        sprintf(str,"\0");
        return strbuf;
    }

};

class Rp {
private:
    OpStack<> ops;
    VStack<> vs;
public:
    ParserResult parse(RawTokenIterator& iter,int depth=0)
    {
        struct RawToken_t token;
        RawTokenParser parser(token);
        int sign = 1;
        bool hassign = false;
        bool isaddsignop = false; //最後に読みだしたのが符号であるか
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
                    if (!hassign && isaddsignop) {
                        r = this->vs.marge(ops, &OpTableDef_PLUS);
                        if (r != ParserResult::OK) {
                            return r;
                        }
                    }
                    hassign = true;
                    continue;
                }
                hassign = false;
                isaddsignop = false;
                //TODO signを考慮して
                //符号以外のデリミタがきた
                switch (tmp_delim->delim) {
                case DelimType::BRKT_L:
                    if (sign<0) {
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
                hassign = false;
                isaddsignop = true;
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
        }
    }
    static void test() {
        struct local {
            static void parse(const char* s) {
                Rp rp;
                RawTokenIterator rti(s);
                rp.parse(rti);
                printf("%s -> %s\n",s,rp.vs.sdump(rp.vs));
            }
        };
        //local::parse("1+127+32767+2147483647");
        //local::parse("-1-128-32768-2147483648");
        local::parse("1+(2+3+(4+5+6)+7)");
        //local::parse("-1+-2");
        //local::parse("1+2");
        //local::parse("-1+-2*-3");
        //local::parse("1+2*3");
        //local::parse("-1+-2*-3+-4");
        //local::parse("1+2*3+4");
        //local::parse("-1+(-2+-3)");
        //local::parse("1+(2+3)");
        //local::parse("-1*(-2+-3)");
        //local::parse("1*(2+3)");
        //local::parse("-1*(-2*-3)");
        //local::parse("1*(2*3)");
        //local::parse("-1-(-2+-3)");
        //local::parse("1-(2+3)");
        //local::parse("-1*-(-2+-3)");
        //local::parse("1*-(+2+3)");
        //local::parse("-1*-(-2*-3)");
        //local::parse("1*-(2*3)");
        //local::parse("1*--(2*3)");
        local::parse("-1+(-2+-3)");
        local::parse("-1*+(-2+-3)");
        //local::parse("3+4*2");//, 11), # 3 + 4 * 2 = 11
        //local::parse("(7+3)*2");//,, 20), # (7 + 3) * 2 = 20
        //local::parse("1+2+3+4+5");//,, 15), # 1 + 2 + 3 + 4 + 5 = 15
        //local::parse("8*4-6/2");//,, 30), # 8 * 4 - 6 / 2 = 30
        //local::parse("(1+2)*(3+4)");//,, 21), # (1 + 2) * (3 + 4) = 21
        //local::parse("-2*4+3");//,, -5), #  - 2 * 4 + 3 = -5
        //local::parse("-(3+4)*2");//,, -14), #  - (3 + 4) * 2 = -14
        //local::parse("-2+3*4");//,, 10), #  - 2 + 3 * 4 = 10
        //local::parse("-(2+3*4)");//,, -14), #  - (2 + 3 * 4) = -14
        //local::parse("---2");//,, -2), #  - (-(-2)) = -2
        //local::parse("--2*3");//,, 6), #  - (-2) * 3 = 6
        return;
    }
};



int main()
{
    Rp::test();
    
    //RawTokenIterator::test();
}


