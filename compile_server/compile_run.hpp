#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <jsoncpp/json/json.h>
#include "compiler.hpp"
#include "runner.hpp"

namespace yyjs_compile_run
{
    using namespace yyjs_compiler;
    using namespace yyjs_runer;
    class CompileRun
    {
    public:
        CompileRun()
        {
        }
        ~CompileRun()
        {
        }

    public:
        /*
         * in_json:  得到序列化后的字符串
         * * code:   用户提交的代码
         * * input:  代码对应的输入,  不做处理
         * * cpu_limit:  时间限制
         * * mem_limit:  空间限制
         * *******************
         * out_jons: 将输出信息进行序列化后返回
         * * status: 状态码
         * * reason: 状态码说明
         * * <stdout>:   程序运行完后的结果
         * * <stderr>:   程序运行时,错误或提示信息
         */
        static int Start(std::string &in_json, std::string *out_json)
        {
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            Json::Value out_value;
            int status = 0;
            std::string reason;

            std::string file_name;
            // 如果读取失败后, 差错处理..
            if (code.size() == 0)
            {
                status = -1;
            }
            else
            {
                // 形成的文件名只具有唯一性，没有目录没有后缀
                file_name = FileUtil::UniqueFileName();

                // 形成src文件
                if (!FileUtil::WriteFile(PathUtil::Src(file_name), code))
                {
                    status = -2;
                }
                else
                {
                    // compile
                    if (!Compiler::Compile(file_name))
                    {
                        status = -3;
                    }
                    else
                    {
                        // run
                        int ret = Runner::Run(file_name, cpu_limit, mem_limit);
                        status = ret < 0 ? -2 : ret;
                        if (status == 0)
                        {
                            std::string stdout, stderr;
                            FileUtil::ReadFile(PathUtil::Stdout(file_name), &stdout, true);
                            FileUtil::ReadFile(PathUtil::Stderr(file_name), &stderr, true);

                            out_value["stdout"] = stdout;
                            out_value["stderr"] = stderr;
                        }
                    }
                }
            }
            reason = StatusToDesc(status, file_name);

            out_value["status"] = status;
            out_value["reason"] = reason;

            // 输出型参数
            Json::StyledWriter writer;
            *out_json = writer.write(out_value);

            RemoveTempFile(file_name);
        }

    private:
        static void RemoveTempFile(const std::string &file_name)
        {
            // 清理文件的个数是不确定的，但是有哪些我们是知道的
            std::string _src = PathUtil::Src(file_name);
            if (FileUtil::IsFileExists(_src))
                unlink(_src.c_str());
            std::string _compiler_error = PathUtil::CompileError(file_name);
            if (FileUtil::IsFileExists(_compiler_error))
                unlink(_compiler_error.c_str());
            std::string _execute = PathUtil::Exe(file_name);
            if (FileUtil::IsFileExists(_execute))
                unlink(_execute.c_str());
            std::string _stdin = PathUtil::Stdin(file_name);
            if (FileUtil::IsFileExists(_stdin))
                unlink(_stdin.c_str());
            std::string _stdout = PathUtil::Stdout(file_name);
            if (FileUtil::IsFileExists(_stdout))
                unlink(_stdout.c_str());
            std::string _stderr = PathUtil::Stderr(file_name);
            if (FileUtil::IsFileExists(_stderr))
                unlink(_stderr.c_str());
        }

        static std::string StatusToDesc(int status, const std::string &file_name)
        {
            std::string description;

            switch (status)
            {
            case 0:
                description = "编译运行成功";
                break;
            case -1:
                description = "提交的代码为空";
                break;
            case -2:
                description = "未知错误";
                break;
            case -3:
                // 编译失败,读取.compile_error文件
                FileUtil::ReadFile(PathUtil::CompileError(file_name), &description, true);
                break;
            case SIGABRT: // 6
                description = "内存超过范围";
                break;
            case SIGXCPU: // 24
                description = "CPU使用超时";
                break;
            case SIGFPE: // 8
                description = "浮点数溢出";
                break;
            // case SIGSEGV: // 11
            //     description = "段错误";
            //     break;
            default:
                description = "未知: " + std::to_string(status);
                break;
            }
            return description;
        }
    };
}