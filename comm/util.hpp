#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <atomic>
#include <boost/algorithm/string.hpp>

namespace yyjs_util
{

    const std::string temp_path = "./temp/";

    class PathUtil
    {
    private:
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string path_name = temp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }

    public:
        // Compile
        static std::string Src(const std::string &file_name)
        {
            return AddSuffix(file_name, ".c++");
        }
        static std::string Exe(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }
        static std::string CompileError(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compile_error");
        }
        // Run
        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }
        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }
    };

     class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }

        //获得毫秒时间戳
        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    class FileUtil
    {
    public:
        static std::string UniqueFileName()
        {
            // 形成的文件名只具有唯一性，没有目录没有后缀
            // 毫秒级时间戳+原子性递增唯一值: 来保证唯一性
            std::string name = TimeUtil::GetTimeMs();
            static std::atomic_int id(0);
            id++;//线程安全的
            
            return name + "_" + std::to_string(id);
        }
        static bool IsFileExists(const std::string &path_name)
        {
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
            {
                // 获取属性成功，文件已经存在
                return true;
            }

            return false;
        }

        static bool WriteFile(const std::string& path_name, const std::string& content)
        {
            std::ofstream out(path_name);
            if(!out.is_open())
            {
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }

        static bool ReadFile(const std::string& path_name, std::string * content, bool keep = false)
        {
            (*content).clear();

            std::ifstream in(path_name);

            if(!in.is_open()) return false;

            std::string line;

            while(std::getline(in, line))
            {
                (*content) += line;
                //getline函数默认会舍弃结尾的\n
                (*content) += (keep ? "\n" : "");
            }
            in.close();
            return true;
        }
    };

    class StringUtil
    {
    public:
        static void SplitString(const std::string &str, std::vector<std::string> *target, const std::string &sep)
        {
            //boost split
            boost::split((*target), str, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }

    };
   
}