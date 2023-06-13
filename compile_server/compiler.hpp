#pragma once

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"

namespace yyjs_compiler
{
    using namespace yyjs_util;
    using namespace yyjs_log;

    class Compiler
    {
    public:
        Compiler()
        {
        }
        ~Compiler()
        {
        }

        static bool Compile(const std::string &file_name)
        {
            pid_t pid = fork();
            
            if(pid < 0)
            {
                LOG(ERROR) << "创建子进程失败" << std::endl;
                return false;
            }
            else if(pid == 0)
            {
                //child process

                //redirect
                umask(0);
                int _err_fd = open(PathUtil::CompileError(file_name).c_str(),\
                    O_CREAT | O_WRONLY, 0644);
                
                if(_err_fd < 0)
                {
                    LOG(WARNING) << "没有形成标准错误的文件" << std::endl;
                    exit(1);
                }
                dup2(_err_fd, 2); //makes 2 be the copy of _err_fd
                //LOG(DEBUG) << "标准错误重定向成功" << std::endl;

                //g++
                execlp("g++", "g++", "-o", PathUtil::Exe(file_name).c_str(),\
                    PathUtil::Src(file_name).c_str(), "-D", "COMPILER_ONLINE", "-std=c++11", nullptr);

                LOG(ERROR) << "启动g++程序失败" << std::endl;
                exit(2);//exit child process
            }
            else
            {
                //parent process
                waitpid(pid, nullptr, 0); //block wait
                if(FileUtil::IsFileExists(PathUtil::Exe(file_name)))
                {
                    LOG(INFO) << PathUtil::Exe(file_name).c_str() << " 编译成功!" << std::endl;
                    return true;
                }

                LOG(ERROR) << "编译失败, 没有形成可执行文件" << std::endl;
                return false;
            }

        }
    };
}
