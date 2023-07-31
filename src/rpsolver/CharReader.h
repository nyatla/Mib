#pragma once
#include "./_resolver_imports.h"
namespace MIB {
    /// <summary>
    /// NULL終端メモリブロックを読みだすイテレータ
    /// </summary>
    class ICharReader {
    public:
        virtual ~ICharReader() {};
        /// <summary>
        /// 読出しポインタを1進めて1文字読み出します。
        /// </summary>
        /// <param name="d"></param>
        /// <returns></returns>
        virtual bool getc(char& d) =0;
        /// <summary>
        /// ポインタを移動します。
        /// </summary>
        /// <returns>移動に成功したらTrue</returns>
        virtual bool seek(MIB_INT16 skip) = 0;
        virtual MIB_INT16 pos()const=0;
        virtual const char* ptr()const = 0;
    };

    /// <summary>
    /// NULL終端メモリブロックを読みだすイテレータ
    /// </summary>
    class CharReader :public ICharReader{
    private:
        const char* _src;
        MIB_INT16 _pos;
    public:
        CharReader(const char* src) {
            this->_src = src;
            this->_pos = 0;
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="d"></param>
        /// <returns>終端に到達するとfalseです</returns>
        bool getc(char& d)override
        {
            if (this->_src[this->_pos] != 0) {
                d = this->_src[this->_pos++];
                return true;
            }
            return false;
        };
        /// <summary>
        /// ポインタを移動します。
        /// </summary>
        /// <returns>移動に失敗した</returns>
        bool seek(MIB_INT16 skip)override
        {
            if (skip < 0) {
                if (this->_pos + skip < 0) {
                    return false;
                }
            }
            else {
                for (auto i = 0;i < skip;i++) {
                    if (this->_src[this->_pos+i] == 0) {
                        return false;
                    }
                }
            }
            this->_pos += skip;
            return true;
        }
        MIB_INT16 pos()const override {
            return this->_pos;
        }
        const char* ptr()const override{
            return this->_pos + this->_src;
        }

    };
}