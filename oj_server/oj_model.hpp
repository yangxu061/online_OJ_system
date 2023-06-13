#pragma once
//文件版本
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "../comm/util.hpp"
#include "../comm/log.hpp"

namespace yyjs_model
{
    using namespace std;
    using namespace yyjs_util;
    using namespace yyjs_log;

    class Question
    {
    public:
        string number;  //题目编号
        string title;   //题目标题
        string star;    //难度: 简单 中级 困难
        int cpu_limit;  //时间限制(s)
        int mem_limit;  //空间限制(kb)
        string desc;    //题目描述
        string header;  //预设代码
        string tail;    //测试用例 ,最后会和header进行拼接
    };

    const string question_path = "./question/";
    const string questions_list = "./question/questions.list";

    class Model
    {
    private:
        map<string, Question> _questions;
    public:
        Model()
        {
            assert(LoadALLQuestions());
        }
        ~Model()
        {
        }

        bool GetAllQuestions(vector<Question> *questions)
        {
            if(_questions.size() == 0)
            {
                LOG(ERROR) << "用户获取题库失败" << endl;
                return false;
            }

            for(auto & qut : _questions)
            {
                questions->push_back(qut.second);
            }
            return true;
        }

        bool GetOneQuestion(const string &number, Question *question)
        {
            const auto &iter = _questions.find(number);
            if(iter == _questions.end())
            {
                LOG(ERROR) << "用户获取题目" << number << "失败" << endl;
                return false;
            }
            (*question) = iter->second;
            return true;
        }
    // private:
        bool LoadALLQuestions()
        {
            //加载配置文件questions.list
            ifstream in(questions_list);
            if(!in.is_open())
            {
                LOG(FATAL) << "题库加载失败, 检查配置文件" << endl;
                return false;
            }

            string line;
            while(getline(in, line))
            {
                vector<string> content;
                StringUtil::SplitString(line, &content, " ");
                // 1 判断回文数 简单 1 30000
                if(content.size() != 5)
                {
                    LOG(WARNING) << "部分题目加载失败,请检查配置文件内容格式" << endl;
                    continue;
                }

                Question qut;
                qut.number = content[0];
                qut.title = content[1];
                qut.star = content[2];
                qut.cpu_limit = stoi(content[3]);
                qut.mem_limit = stoi(content[4]);

                string detail_path = question_path + qut.number + "/";
                FileUtil::ReadFile(detail_path + "desc.txt", &(qut.desc), true);
                FileUtil::ReadFile(detail_path + "header.cpp", &(qut.header), true);
                FileUtil::ReadFile(detail_path + "tail.cpp", &(qut.tail), true);

                _questions.insert({qut.number, qut});
            }
            
            LOG(INFO) << "加载题库成功" << endl;
            in.close();
            return true;
        }
    };
}