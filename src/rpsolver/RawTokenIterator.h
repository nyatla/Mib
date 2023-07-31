#pragma once
#include "./_resolver_imports.h"
#include "./CharReader.h"
namespace MIB {

    enum class RawTokenType {
        DELIM, STR, NUMBER, TEXT, UNKNOWN
    };

    struct RawToken_t {
        RawTokenType type = RawTokenType::UNKNOWN;
        const char* ptr;
        int size;
        /// <summary>
        /// トークンをint32に変換して返します。
        /// 値範囲は[INT32MIN,INT32MAX]です。
        /// </summary>
        /// <param name="out"></param>
        /// <param name="sign"></param>
        /// <returns>パース結果を返します。</returns>
        ParserResult asInt32(int& out, int sign = 1)const
        {
            if (this->type != RawTokenType::NUMBER) {
                return ParserResult::NG;
            }
            int t = 0;
            int i;
            if (sign > 0) {
                for (i = 0;i < this->size;i++) {
                    int d = this->ptr[i] - '0';
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
                for (i = 0;i < this->size;i++) {
                    int d = this->ptr[i] - '0';
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


    /// <summary>
    /// 行単位でWORDを読みだすイテレータ
    /// [WORD] [DELIM] [NUMBER] [STR]の何れかに該当する文字列を返す。
    /// DELIM   + - * ^ / % = ; ( ) , & | > < !
    ///         == <= >= != >> << <>
    ///         Xor Mod And Or Not
    /// CONST   TRUE FALSE
    /// NUMBER  [0-9]+
    /// WORD   他
    /// 終端は"\r"
    /// </summary>
    class RawTokenIterator {
    private:
        CharReader _iter;
        static bool isSp(char c) {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n';
        }
        static bool isNum(char c) {
            return c >= '0' && c <= '9';
        }
        static bool isDelim(char c) {
            return strchr("+-*^/%=;(),&|><!", c) != NULL;
        }
        static bool isKeywordChar(char c) {
            return ('a'<c && c<'z')||('A' < c && c < 'Z')|| ('0' < c && c < '9')||( memchr("_",c,1) != NULL);
        }

    private:
        struct RawToken_t last_token = {};
        bool has_token;
    public:
        RawTokenIterator(const char* src) :_iter(CharReader(src)),has_token(false){
        }
        /// <summary>
        /// 次のトークンを読みだす。
        /// </summary>
        /// <param name="token">次回のpeek/nextまで有効な参照値</param>
        /// <returns></returns>
        ParserResult next(const struct RawToken_t*& token)
        {
            //既に読出し済ならそれを返す。
            if (!this->has_token) {
                auto r = this->peek(token);
                if (r != ParserResult::OK) {
                    return r;
                }
            }
            this->has_token = false;
            return ParserResult::OK;
        }
        /// <summary>
        /// 次のトークンをスキップします。
        /// </summary>
        /// <param name="token">次回のpeek/nextまで有効な参照値</param>
        /// <returns></returns>
        ParserResult skip()
        {
            const struct RawToken_t* token;
            return this->next(token);
        }

        /// <summary>
        /// 次のトークンをポインタをすすめずに読みだす。
        /// </summary>
        /// <param name="token">次回のpeek/nextまで有効な参照値</param>
        /// <returns></returns>
        ParserResult peek(const struct RawToken_t*& token)
        {
            //既に読出し済ならそれを返す。
            if (this->has_token) {
                token = &this->last_token;
                return ParserResult::OK;
            }
            //新規に読出し
            this->has_token = false;
            struct RawToken_t& lasttoken = this->last_token;
            int s = 0;
            auto& iter = this->_iter;
            char c0;
            for (;;) {
                if (!iter.getc(c0)) {
                    return ParserResult::NG_EOF;
                }
                //先頭SPのスキップ
                if (isSp(c0)) {
                    continue;
                }
                //b[0]に値
                lasttoken.ptr = iter.ptr() - 1;
                s++;
                break;
            }
            MIB_ASSERT(s > 0);
            if (isDelim(c0))
            {   // DELIM: ()+-*/<>=&|の1文字のデリミタ,または <<,>>,<>,!=
                char c1 = 0;
                if (iter.getc(c1)) {
                    if ((c0 == '<' && (c1 == '>' || c1 == '<') || c1 == '=') || // <<,<=,<>
                        (c0 == '>' && c1 == '>' || c1 == '=') ||                // >>,>=
                        (c0 == '!' && c1 == '=')                                // =!,==
                        ) {
                        lasttoken.size = 2;
                    }
                    else {
                        iter.seek(-1);
                        lasttoken.size = 1;
                    }
                }else{
                    lasttoken.size = 1;
                }
                lasttoken.type = RawTokenType::DELIM;
                token = &lasttoken;
                this->has_token = true;
                return ParserResult::OK;
            }
            if (c0 == '"')
            {   // STR:    "から開始し、"で終わる文字列
                for (s = 0;;s++) {
                    char c1;
                    if (!iter.getc(c1)) {
                        break;
                    }
                    if (c1 == '"') {
                        lasttoken.ptr++;
                        lasttoken.size = s;
                        lasttoken.type = RawTokenType::STR;
                        token = &lasttoken;
                        this->has_token = true;
                        return ParserResult::OK;
                    }
                }
                return ParserResult::NG_EOF;
            }
            if (isNum(c0))
            {   // NUMBER: [0-9]+
                for (;;s++) {
                    char c1;
                    if (!iter.getc(c1)) {
                        break;
                    }
                    if(isNum(c1)) {
                        continue;
                    }
                    if (!iter.seek(-1)) {
                        return ParserResult::NG_IO;
                    }
                    break;
                }
                lasttoken.size = s;
                lasttoken.type = RawTokenType::NUMBER;
                token = &lasttoken;
                this->has_token = true;
                return ParserResult::OK;//少なくとも1文字は取れてる
            }
            {
                // TEXT:   それ以外の連続値
                for (;;s++) {
                    char c1;
                    if (!iter.getc(c1)) {
                        break;
                    }
                    if (!isKeywordChar(c1)) {
                        if (!iter.seek(-1)) {
                            return ParserResult::NG_IO;
                        }
                        break;
                    }
                }
                lasttoken.type = RawTokenType::TEXT;
                //Xor,Mod,And,Or,Notについてはデリミタ
                if (memchr("MAOXN", *lasttoken.ptr,5)!=NULL) {
                    static const char* d[] = { "Mod","And","Or","Xor","Not" };
                    for (auto i = 0;i < sizeof(d);i++) {
                        if (strlen(d[i])==s && memcmp(d[i], lasttoken.ptr,s) == 0) {
                            lasttoken.type = RawTokenType::DELIM;
                            break;
                        }
                    }
                }
                lasttoken.size = s;
                token = &lasttoken;
                this->has_token = true;
                return ParserResult::OK;//少なくとも1文字はある
            }

            return ParserResult::NG;   //INVALID_TOKEN
        };
#ifdef TEST
    public:
        static void test() {
            RawTokenIterator rti(" 1234 -85  % & hjiofgr0_ -aa < << >> \"ssss\" 123456789541test (10) ");
            const struct RawToken_t* token;
            while (rti.next(token) == ParserResult::OK) {
                printf("%d %.*s\n", token->type, token->size,token->ptr);
            }
            return;
        }

#endif

    };
}