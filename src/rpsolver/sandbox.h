#pragma once
/*
やあ （´・ω・｀)
ようこそ、sandbox.hへ。
ここの関数はどこにも使われていないから、まずは落ち着いて欲しい。

うん、「また」なんだ。済まない。
仏の顔もって言うしね、謝って許してもらおうとも思っていない。

でも、このファイルを見たとき、君は、きっと言葉では言い表せない「ときめき」みたいなものを感じてくれたと思う。
殺伐とした世の中で、そういう気持ちを忘れないで欲しい
そう思って、このファイルを作ったんだ。

*/


/// <summary>
/// 名前テーブルのテンプレート。
/// 継承先でValueMapItem<VTYPE> _tbl[]メンバ変数を定義してね.
/// </summary>
/// <typeparam name="VTYPE"></typeparam>
template<typename VTYPE> class ConstValueMap {
public:
    struct Item {
        const char* name;
        VTYPE value;
    };
protected:
    const struct Item* _tbl;
public:
    ConstValueMap(const ConstValueMap::Item* tbl) :_tbl{ tbl } {};
    bool indexOf(const char* name, const VTYPE& out)const {
        for (auto i = 0;i < this->_tbl[i].name != NULL;i++) {
            if (strcmp(name, this->_tbl[i].name) == 0) {
                out = this->_tbl[i].value;
                return true;
            }
        }
        return false;
    }
    virtual ~ConstValueMap() {}

};





//const static ConstValueMap<OpDef>::Item _WORD_OP_MAP_DATA[] = {
//    {"And",OpTableDef_AND},
//    {"Or",OpTableDef_OR},
//    {"Xor",OpTableDef_XOR},
//    {"Not",OpTableDef_NOT},
//    {NULL,}
//};
//const static ConstValueMap<OpDef> _WORD_OP_MAP(_WORD_OP_MAP_DATA);
