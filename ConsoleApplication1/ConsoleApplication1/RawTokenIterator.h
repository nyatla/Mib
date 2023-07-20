#pragma once
#include "mib_types.h"
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
    /// WORD   �A�����镶����
    /// DELIM 
    /// NUMBER [0-9]+
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
            char c0= 0;
            for (;;) {
                auto r = iter.getc(c0);
                switch (r) {
                case ParserResult::OK:break;
                case ParserResult::NG_EOF:return ParserResult::NG_EOF;
                default:return ParserResult::NG;
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
            if (strchr("+-*^/%=;(),&|><!", c0))
            {   // DELIM: ()+-*/<>=&|��1�����̃f���~�^,�܂��� <<,>>,<>,!=
                char c1 = 0;
                auto r = iter.getc(c1);
                switch (r) {
                case ParserResult::OK:
                    if ((c0 == '<' && (c1 == '>' || c1 == '<')) ||
                        (c0 == '>' && c1 == '>') ||
                        (c0 == '!' && c1 == '=')
                        ) {
                        token.size = 2;
                    }
                    else {
                        iter.seek(-1);
                        token.size = 1;
                    }
                    break;
                default:
                    token.size = 1;
                }
                token.type = RawTokenType::DELIM;
                return ParserResult::OK;
            }
            else if (c0 == '"')
            {   // STR:    "����J�n���A"�ŏI��镶����
                for (s = 0;;s++) {
                    if (iter.getc(c0) != ParserResult::OK) {
                        break;
                    }
                    if (c0 == '"') {
                        token.ptr++;
                        token.size = s;
                        token.type = RawTokenType::STR;
                        return ParserResult::OK;
                    }
                }
            }
            else if (isNum(c0))
            {   // NUMBER: [0-9]+
                for (;;s++) {
                    auto r = iter.getc(c0);
                    if (r == ParserResult::OK && isNum(c0)) {
                        continue;
                    }
                    token.size = s;
                    token.type = RawTokenType::NUMBER;
                    if (r!= ParserResult::NG_EOF) {
                        iter.seek(-1);
                    }
                    return ParserResult::OK;
                }
            }
            else {
                // TEXT:   ����ȊO�̘A���l
                for (;;s++) {
                    if (iter.getc(c0) != ParserResult::OK || isSp(c0)) {
                        token.size = s;
                        token.type = RawTokenType::TEXT;
                        return ParserResult::OK;
                    }
                }
            }

            return ParserResult::NG_TokenTooLong;   //TOKEN TOO LONG
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