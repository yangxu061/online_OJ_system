#pragma once
// MySQL版本
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "include/mysql.h"

namespace yyjs_model
{
    using namespace std;
    using namespace yyjs_util;
    using namespace yyjs_log;

    class Question
    {
    public:
        string number; // 题目编号
        string title;  // 题目标题
        string star;   // 难度: 简单 中级 困难
        string desc;   // 题目描述
        string header; // 预设代码
        string tail;   // 测试用例 ,最后会和header进行拼接
        int cpu_limit; // 时间限制(s)
        int mem_limit; // 空间限制(kb)
    };

    const string oj_questions_table = "oj_questions";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string passwd = "123456";
    const std::string db = "oj";
    const int port = 3306;
    class Model
    {
    private:
    public:
        Model()
        {
        }
        ~Model()
        {
        }

        bool GetAllQuestions(vector<Question> *questions)
        {
            string sql = "select * from " + oj_questions_table;

            return QueryMysql(sql, questions);
        }

        bool GetOneQuestion(const string &number, Question *question)
        {
            string sql = "select * from " + oj_questions_table;
            sql += " where number=" + number;
            vector<Question> questions;
            if (QueryMysql(sql, &questions))
            {
                if (questions.size() == 1)
                {
                    (*question) = questions[0];
                    return true;
                }
            }
            return false;
        }

    private:
        bool QueryMysql(const string &sql, vector<Question> *out)
        {
            // 创建mysql句柄
            MYSQL *my = mysql_init(nullptr);
            // 连接数据库
            if (nullptr == mysql_real_connect(my, host.c_str(), user.c_str(),
                passwd.c_str(), db.c_str(), port, nullptr, 0))
            {
                LOG(FATAL) << "连接数据库失败!" << "\n";
                return false;
            }

            // 一定要设置该链接的编码格式,要不然会出现乱码问题
            mysql_set_character_set(my, "utf8");
            LOG(INFO) << "连接数据库成功!" << "\n";
            // 执行sql语句
            if (0 != mysql_query(my, sql.c_str()))
            {
                LOG(WARNING) << sql << " execute error!" << "\n";
                return false;
            }

            // 提取结果
            MYSQL_RES *res = mysql_store_result(my);
            // 分析结果
            int rows = mysql_num_rows(res);   // 获得行数量
            int cols = mysql_num_fields(res); // 获得列数量
            Question q;
            for (int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                q.number = row[0];
                q.title = row[1];
                q.star = row[2];
                q.desc = row[3];
                q.header = row[4];
                q.tail = row[5];
                q.cpu_limit = atoi(row[6]);
                q.mem_limit = atoi(row[7]);
                out->push_back(q);
            }

            // 释放结果空间
            free(res);
            // 关闭mysql连接
            mysql_close(my);
            return true;
        }
    };
}