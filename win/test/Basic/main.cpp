// LineBuf.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//


#define TEST

#include "../../../src/linebuf/linebuf.h"
#include "../../../src/rpsolver/rpsolver.h"
#include <stdarg.h>
using namespace MIB;


class Method {
    bool isMatch() {

    }
};

/*
    予約語テーブル
    Command
        RUN,LIST,SAVE,LOAD,CLS

    Statement
        FOR TO NEXT STEP,IF THEN ELSE,GOTO

    Command/Statement
        PRINT,LET,REM,INPUT

    Function
        TBSK.{IN,OUT,SET},
        GPIO,ADC,DAC,SPI,I2C,UART
*/




class IConsole
{
public:
    /// <summary>
    /// printf like output
    /// </summary>
    /// <param name="fmt">
    /// % d       int値
    /// % c       char値
    /// %% '%'
    /// %s, % .*s  文字列
    /// </param>
    /// <param name="args"></param>
    virtual void print(const char* fmt...) = 0;
    virtual const char* input(int& len)= 0;
};

/// <summary>
/// 長すぎるとバッファオーバーフローする緩いコンソール
/// </summary>
class StandardConsole :public IConsole {
private:
    char _buf[256] = {};
public:
    void print(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
    const char* input(int &len) {
        auto& buf = this->_buf;
        char lf;
        memset(buf, 0, sizeof(buf));
        int n=scanf("%[^\n]%c",buf,&lf);
        len = (int)strlen(buf);
        buf[len] = 0;
        return buf;
    }
};

class BasicInstance
{

public:
    enum class Result {
        OK,
        NG,
        NG_COMMAND_NOT_FOUND,
        NG_NOT_IMPLEMENTED,
    };
private:
    class AKeywordProc
    {
    public:
        virtual Result execute(BasicInstance& basic, RawTokenIterator& rti) {
            return Result::NG_NOT_IMPLEMENTED;
        }
        virtual ~AKeywordProc() {};
    };
    class PrintProc :public AKeywordProc
    {
    public:
        Result execute(BasicInstance& basic, RawTokenIterator& rti) {
            return Result::NG_NOT_IMPLEMENTED;
        }
    };
    class ListProc :public AKeywordProc
    {
    public:
        Result execute(BasicInstance& basic, RawTokenIterator& rti) {
            MIB_UINT16 line = 0;
            const char* text = NULL;
            int size;
            for (int idx = 0;;) {
                idx = basic.lines.iter(idx, line, text, size);
                if (idx < 0) {
                    break;
                }
                basic._console.print("%d %.*s\n", line,size,text);
            }
            return Result::OK;
        }
    };

private:
    LineBuffer<128> lines;
    IConsole& _console;
private:
    static bool equalKeyword(const char* kwd, const RawTokenIterator::RawToken_t* test) {
        return test->type == RawTokenType::TEXT && strlen(kwd) == test->size && memcmp(test->ptr, kwd, test->size)==0;
    }
public:
    BasicInstance(IConsole& console) :_console(console) {
    }


    /// <summary>
    /// ステートメントを実行します。
    /// </summary>
    /// <param name="statement"></param>
    /// <param name="size"></param>
    Result exec(const char* statement, int size)
    {
        /// この関数はCharReaderのバッファが連続するconst charであることを要求します。
        RawTokenIterator rti(statement);
        const RawTokenIterator::RawToken_t* token;
        RawTokenIterator::Result r;

        //get 1st token
        r = rti.peek(token);
        if (r != RawTokenIterator::Result::OK) {
            return Result::NG;
        }

        if (token->type == RawTokenType::NUMBER)
        {   //行番号がある場合

            //行番号の確定
            int lno;
            r = token->asInt32(lno);
            if (r != RawTokenIterator::Result::OK) {
                return Result::NG;
            }
            //行番号範囲の確認
            if (lno < 0 || lno >= 0x7fff) {
                return Result::NG;
            }
            //行番号をスキップ
            r = rti.skip();
            if (r != RawTokenIterator::Result::OK) {
                return Result::NG;
            }
            //次のトークン先頭を得る。
            r = rti.peek(token);
            if (r != RawTokenIterator::Result::OK) {
                return Result::NG;
            }
            //行番号に続く任意トークンの位置（CharReaderのメモリ依存性がある部分）
            int skip_size = (int)(token->ptr - statement);
            //行番号の次のトークンを得る
            auto r2 = this->lines.update((MIB_UINT16)lno, statement + skip_size, size - skip_size);
            if (r2 != ILineBuffer::Result::OK) {
                return Result::NG;
            }
            return Result::OK;
        }
        if (token->type == RawTokenType::TEXT) {
            if (equalKeyword("PRINT", token)) {
                PrintProc proc;
                return proc.execute(*this, rti);
            }
            if (equalKeyword("LIST", token)) {
                ListProc proc;
                return proc.execute(*this, rti);
            }
            if (equalKeyword("LET", token)) {
            }
            if (equalKeyword("RUN", token)) {
            }
            return Result::NG_COMMAND_NOT_FOUND;   //InvalidCommand
        }
    }
    void run()
    {
        MIB_UINT16 line = 0;
        const char* text = NULL;
        int size = 0;
        for (int idx = 0;;) {
            this->lines.iter(idx, line, text, size);

        }
    }
    void mainloop() {
        for (;;) {
            Result ret;
            switch (1)
            {
            case 1:
            {
                int len;
                const char* statement = this->_console.input(len);
                ret=this->exec(statement, len);
                break;
            }
            }
            this->_console.print("Ret %d\n",(int)ret);
        }

    }
};
int main()
{
    StandardConsole c;
    BasicInstance basic(c);
    basic.mainloop();
}

