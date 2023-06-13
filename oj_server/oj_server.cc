#include <iostream>
#include <string>
#include "oj_control.hpp"
#include "../comm/httplib.h"

using namespace yyjs_control;
using namespace httplib;

int main()
{
    Server svr;
    Control ctl;
    

    //返回所有题目列表的网页
    svr.Get("/all_questions", [&ctl](const Request &req, Response &resp){
         //返回一张包含有所有题目的html网页
        std::string html;
        ctl.AllQuestions(&html);
        //用户看到的是什么呢？？网页数据 + 拼上了题目相关的数据
        resp.set_content(html, "text/html;charset=utf-8");
    });
    std::cout << "fine1" << std::endl;

    //返回具体某道题的页面
    svr.Get(R"(/question/(\d+))", [&ctl](const Request &req, Response &resp){
        //通过(\d+)正则表达式来获取题号
        std::string number = req.matches[1];

        std::string html;
        ctl.OneQuestion(number, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });
    std::cout << "fine2" << std::endl;

    svr.Post(R"(/judge/(\d+))", [&ctl](const Request &req, Response &resp){
        //获取题号
        std::string number = req.matches[1];
        std::string out_json;
        ctl.Judge(number, req.body, &out_json);

        resp.set_content(out_json, "application/json;charset=utf-8");
    });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8080);
    return 0;
}
