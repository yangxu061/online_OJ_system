#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"

namespace yyjs_runer
{
    using namespace yyjs_util;
    using namespace yyjs_log;
    class Runner
    {
    public:
        Runner() {}
        ~Runner() {}

    public:
        // 提供设置进程占用资源大小的接口
        static void SetProcLimit(int _cpu_limit, int _mem_limit)
        {
            struct rlimit cpu_limit;
            cpu_limit.rlim_max = RLIM_INFINITY;
            cpu_limit.rlim_cur = _cpu_limit;
            setrlimit(RLIMIT_CPU, &cpu_limit);

            struct rlimit mem_limit;
            mem_limit.rlim_max = RLIM_INFINITY;
            mem_limit.rlim_cur = _mem_limit * 1024;
            setrlimit(RLIMIT_AS, &mem_limit);
        }
        
        // 指明文件名即可，不需要代理路径，不需要带后缀
        /*******************************************
        * 返回值 > 0: 程序异常了，退出时收到了信号，返回值就是对应的信号编号
        * 返回值 == 0: 正常运行完毕的，结果保存到了对应的临时文件中
        * 返回值 < 0: 内部错误
        * 
        * cpu_limit: 该程序运行的时候，可以使用的最大cpu资源上限
        * mem_limit: 改程序运行的时候，可以使用的最大的内存大小(KB)
        * *****************************************/
        static int Run(const std::string &file_name, int _cpu_limit, int _mem_limit)
        {
            /*
            1.运行成功,结果正确
            2.运行成功,结果错误
            3.运行失败 ---> 异常退出 ---> 信号

            程序默认启动
            * 标准输入: 不处理, 由OJ题目提供
            * 标准输出: 运行完成后的结果
            * 标准错误: 运行时错误信息
            */
            std::string _stdin = PathUtil::Stdin(file_name);
            std::string _stdout = PathUtil::Stdout(file_name);
            std::string _stderr = PathUtil::Stderr(file_name);
            std::string _execute = PathUtil::Exe(file_name);

            umask(0);
            int _stdin_fd = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdout_fd = open(_stdout.c_str(), O_CREAT | O_WRONLY, 0644);
            int _stderr_fd = open(_stderr.c_str(), O_CREAT | O_WRONLY, 0644);

            if (_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                LOG(ERROR) << "运行时,标准文件打开失败" << std::endl;
                return -1;
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                LOG(ERROR) << "运行时, 创建子进程失败" << std::endl;
                return -2;
            }
            else if (pid == 0)
            {
                // child process

                // redirect
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);
                //  limit
                SetProcLimit(_cpu_limit, _mem_limit);
                //  ./XX.exe
                // execl(PathUtil::Exe(file_name).c_str(), PathUtil::Exe(file_name).c_str(), nullptr);
                // execl(PathUtil::Exe(file_name).c_str(), nullptr);
                execl(_execute.c_str()/*我要执行谁*/, _execute.c_str()/*我想在命令行上如何执行该程序*/, nullptr);

                LOG(ERROR) << "可执行程序,运行失败" << std::endl;
                exit(1);
            }
            else
            {
                // parent
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);

                int status = 0;
                waitpid(pid, &status, 0);
                LOG(INFO) << "程序执行完成, info:" << (status & 0x7F) << std::endl; 
                return status & 0x7F;
            }
        }
    };
}