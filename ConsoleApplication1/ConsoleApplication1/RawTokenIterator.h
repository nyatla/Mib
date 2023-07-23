#pragma once
#include "mib_stdlib.h"
#include "./CharReader.h"
namespace MIB {

    enum class RawTokenType {
        DELIM, STR, NUMBER, TEXT, UNKNOWN
    };


    struct RawToken_t {
        RawTokenType type = RawTokenType::UNKNOWN;
        const char* ptr;
        int size;
    };


    /// <summary>
    /// �s�P�ʂ�WORD��ǂ݂����C�e���[�^
    /// [WORD] [DELIM] [NUMBER] [STR]�̉��ꂩ�ɊY�����镶�����Ԃ��B
    /// DELIM   + - * ^ / % = ; ( ) , & | > < !
    ///         == <= >= != >> << <>
    ///         Xor Mod And Or Not
    /// CONST   TRUE FALSE
    /// NUMBER  [0-9]+
    /// WORD   ��
    /// �I�[��"\r"
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


    public:
        RawTokenIterator(const char* src) :_iter(CharReader(src)){
        }
        /// <summary>
        /// Iterator����SP��؂�̃g�[�N����ǂ݂���
        /// </summary>
        /// <returns></returns>
        ParserResult next(struct RawToken_t& token) {
            //auto& b = token.buf;
            int s = 0;
            auto& iter = this->_iter;
            char c0;
            for (;;) {
                if (!iter.getc(c0)) {
                    return ParserResult::NG_EOF;
                }
                //�擪SP�̃X�L�b�v
                if (isSp(c0)) {
                    continue;
                }
                //b[0]�ɒl
                token.ptr = iter.ptr() - 1;
                s++;
                break;
            }
            MIB_ASSERT(s > 0);
            if (isDelim(c0))
            {   // DELIM: ()+-*/<>=&|��1�����̃f���~�^,�܂��� <<,>>,<>,!=
                char c1 = 0;
                if (iter.getc(c1)) {
                    if ((c0 == '<' && (c1 == '>' || c1 == '<') || c1 == '=') || // <<,<=,<>
                        (c0 == '>' && c1 == '>' || c1 == '=') ||                // >>,>=
                        (c0 == '!' && c1 == '=')                                // =!,==
                        ) {
                        token.size = 2;
                    }
                    else {
                        iter.seek(-1);
                        token.size = 1;
                    }
                }else{
                    token.size = 1;
                }
                token.type = RawTokenType::DELIM;
                return ParserResult::OK;
            }
            if (c0 == '"')
            {   // STR:    "����J�n���A"�ŏI��镶����
                for (s = 0;;s++) {
                    char c1;
                    if (!iter.getc(c1)) {
                        break;
                    }
                    if (c1 == '"') {
                        token.ptr++;
                        token.size = s;
                        token.type = RawTokenType::STR;
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
                token.size = s;
                token.type = RawTokenType::NUMBER;
                return ParserResult::OK;//���Ȃ��Ƃ�1�����͎��Ă�
            }
            {
                // TEXT:   ����ȊO�̘A���l
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
                token.type = RawTokenType::TEXT;
                //Xor,Mod,And,Or,Not�ɂ��Ă̓f���~�^
                if (memchr("MAOXN", *token.ptr,5)!=NULL) {
                    static const char* d[] = { "Mod","And","Or","Xor","Not" };
                    for (auto i = 0;i < sizeof(d);i++) {
                        if (strlen(d[i])==s && memcmp(d[i], token.ptr,s) == 0) {
                            token.type = RawTokenType::DELIM;
                            break;
                        }
                    }
                }
                token.size = s;
                return ParserResult::OK;//���Ȃ��Ƃ�1�����͂���
            }

            return ParserResult::NG;   //INVALID_TOKEN
        };
    public:
#ifdef TEST
    public:
        static void test() {
            RawTokenIterator rti(" 1234 -85  % & hjiofgr0_ -aa < << >> \"ssss\" 123456789541test (10) ");
            struct RawToken_t token;
            while (rti.next(token) == ParserResult::OK) {
                printf("%d %.*s\n", token.type, token.size,token.ptr);
            }
            return;
        }

#endif

    };
}