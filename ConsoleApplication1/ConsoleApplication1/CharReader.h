#pragma once
#include "mib_stdlib.h"
namespace MIB {
    /// <summary>
    /// NULL�I�[�������u���b�N��ǂ݂����C�e���[�^
    /// </summary>
    class ICharReader {
    public:
        virtual ~ICharReader() {};
        virtual bool getc(char& d) =0;
        /// <summary>
        /// �|�C���^���ړ����܂��B
        /// </summary>
        /// <returns>�ړ��ɐ���������True</returns>
        virtual bool seek(MIB_INT16 skip) = 0;
        virtual MIB_INT16 pos()const=0;
    };




    /// <summary>
    /// NULL�I�[�������u���b�N��ǂ݂����C�e���[�^
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
        /// <returns>�I�[�ɓ��B�����false�ł�</returns>
        bool getc(char& d)override
        {
            if (this->_src[this->_pos] != 0) {
                d = this->_src[this->_pos++];
                return true;
            }
            return false;
        };
        /// <summary>
        /// �|�C���^���ړ����܂��B
        /// </summary>
        /// <returns>�ړ��Ɏ��s����</returns>
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
        const char* ptr() {
            return this->_pos + this->_src;
        }

    };
}