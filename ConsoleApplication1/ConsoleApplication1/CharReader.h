#pragma once
#include "mib_stdlib.h"
namespace MIB {
    /// <summary>
    /// NULL終端メモリブロックを読みだすイテレータ
    /// </summary>
    class CharReader {
    private:
        const char* _src;
        MIB_INT16 _pos;
    public:
        CharReader(const char* src) {
            this->_src = src;
            this->_pos = 0;
        }
        ParserResult getc(char& d)
        {
            if (this->_src[this->_pos] != 0) {
                d = this->_src[this->_pos++];
                return ParserResult::OK;
            }
            return ParserResult::NG_EOF;
        };
        /// <summary>
        /// ポインタを移動します。
        /// </summary>
        /// <returns></returns>
        ParserResult seek(MIB_INT16 skip)
        {
            if (skip < 0) {
                if (this->_pos + skip < 0) {
                    return ParserResult::NG;
                }
            }
            else {
                for (auto i = 0;i < skip;i++) {
                    if (this->_src[this->_pos+i] == 0) {
                        return ParserResult::NG;
                    }
                }
            }
            this->_pos += skip;
            return ParserResult::OK;
        }
        MIB_INT16 pos() {
            return this->_pos;
        }
        const char* ptr() {
            return this->_pos + this->_src;
        }

    };
}