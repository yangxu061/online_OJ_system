#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <ctemplate/template.h>
#include "oj_model.hpp"

namespace yyjs_view
{
    using namespace std;
    using namespace yyjs_model;
    using namespace ctemplate;

    const string template_path = "./template_html/";
    class View
    {
    public:
        View()
        {}
        ~View()
        {}

        void ExpandAll(const vector<Question>& questions, string * html)
        {
            // 题目的编号 题目的标题 题目的难度
            //1.形成路径
            string src_html = template_path + "all_questions.html";

            //2.形成数字典
            TemplateDictionary root("all_questions");
            for(const auto& qut : questions)
            {
                TemplateDictionary * sub = root.AddSectionDictionary("question_list");
                sub->SetValue("number", qut.number);
                sub->SetValue("title", qut.title);
                sub->SetValue("star", qut.star);
            }
            //3.获取要渲染的html
            Template *tpl = Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
            //4.渲染
            tpl->Expand(html, &root);
        }

        void ExpandOne(const Question &question, string *html)
        {
            //1.路径
            string src_html = template_path + "one_question.html";

            //2.形成数字典
            TemplateDictionary root("one_question");
            root.SetValue("number", question.number);
            root.SetValue("title", question.title);
            root.SetValue("star", question.star);
            root.SetValue("desc", question.desc);
            root.SetValue("pre_code", question.header);

            //3.获取要渲染的html
            Template *tpl = Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            //4.渲染
            tpl->Expand(html, &root);
        }
    };
}