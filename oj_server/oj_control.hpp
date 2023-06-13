#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <cassert>
#include <jsoncpp/json/json.h>
#include "../comm/httplib.h"
#include "oj_model.hpp"
#include "oj_view.hpp"

namespace yyjs_control
{
    using namespace std;
    using namespace httplib;
    using namespace yyjs_model;
    using namespace yyjs_view;

    class Machine
    {
    public:
        string ip;  //编译服务的ip
        int port;        //编译服务的port
        uint64_t load;   //编译服务的负载
        mutex *mtx; // mutex禁止拷贝的，使用指针
    public:
        Machine() : ip(""), port(0), load(0), mtx(nullptr)
        {}
        ~Machine()
        {
            // delete mtx;
        }
        // 提升主机负载
        void IncLoad()
        {
            if (mtx) mtx->lock();
            ++load;
            if (mtx) mtx->unlock();
        }
        // 减少主机负载
        void DecLoad()
        {
            if (mtx) mtx->lock();
            --load;
            if (mtx) mtx->unlock();
        }
        // 获取主机负载,没有太大的意义，只是为了统一接口
        uint64_t Load()
        {
            uint64_t _load = 0;
            if (mtx) mtx->lock();
            _load = load;
            if (mtx) mtx->unlock();

            return _load;
        }
    };

    const std::string service_machine = "./conf/service_machine.conf";
    // 负载均衡模块
    class LoadBlance
    {
    private:
        // 可以给我们提供编译服务的所有的主机
        // 每一台主机都有自己的下标，充当当前主机的id
        vector<Machine> _machines;
        // 所有在线的主机id
        vector<int> _online;
        // 所有离线的主机id
        vector<int> _offline;
        // 保证LoadBlance它的数据安全
        mutex mtx;
    public:
        LoadBlance()
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << "加载 " << service_machine << " 成功"
                      << "\n";
        }
        ~LoadBlance()
        {}

        bool LoadConf(const std::string &machine_conf)
        {
            ifstream in(machine_conf);
            if(!in.is_open())
            {
                LOG(FATAL) << "加载: " << machine_conf << " 失败" << endl;
                return false;
            }

            string line;
            while(getline(in, line))
            {
                vector<string> content;
                StringUtil::SplitString(line, &content, ":");
                if(content.size() != 2)
                {
                    LOG(WARNING) << " 加载某台主机的配置信息错误" << endl;
                    continue;
                }
                Machine m;
                m.ip = content[0];
                m.port = stoi(content[1]);
                m.load = 0;
                m.mtx = new mutex();

                _online.push_back(_machines.size());
                _machines.push_back(m);
            }
            in.close();

            return true;
        }

        // id: 输出型参数
        // m : 输出型参数
        bool SmartChoice(int *id, Machine **m)
        {
            // 1. 使用选择好的主机(更新该主机的负载)
            // 2. 我们可能需要离线该主机
            mtx.lock();
            // 负载均衡的算法
            // 1. 随机数+hash
            // 2. 轮询+hash
            int online_num = _online.size();
            if(online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << "所有后端编译主机已离线!!" << endl;
                return false;
            }

            int temp_id = _online[0];
            //int temp_m = &_machines[_online[0]];
            uint64_t min_load = _machines[_online[0]].Load();
            for(int i = 1; i < online_num; ++i)
            {
                uint64_t curr_load = _machines[_online[i]].Load();
                if(curr_load < min_load)
                {
                    min_load = curr_load;
                    temp_id = i;
                }
            }
            (*id) = temp_id;
            (*m ) = &_machines[_online[temp_id]];
            mtx.unlock();
            return true;
        }
        void OfflineMachine(int which)
        {
            mtx.lock();
            for(auto iter = _online.begin(); iter != _online.end(); iter++)
            {
                if(*iter == which)
                {
                    // _machines[which].ResetLoad();
                    //要离线的主机已经找到啦
                    _online.erase(iter);
                    _offline.push_back(which);
                    break; //因为break的存在，所有我们暂时不考虑迭代器失效的问题
                }
            }
            mtx.unlock();
        }
        void OnlineMachine()
        {
            //我们统一上线，后面统一解决
            mtx.lock();
            _online.insert(_online.end(), _offline.begin(), _offline.end());
            _offline.erase(_offline.begin(), _offline.end());
            mtx.unlock();

            LOG(INFO) << "所有的主机有上线啦!" << "\n";
        }
         //for test
        void ShowMachines()
        {
             mtx.lock();
             std::cout << "当前在线主机列表: ";
             for(auto &id : _online)
             {
                 std::cout << id << " ";
             }
             std::cout << std::endl;
             std::cout << "当前离线主机列表: ";
             for(auto &id : _offline)
             {
                 std::cout << id << " ";
             }
             std::cout << std::endl;
             mtx.unlock();
        }

    };
    class Control
    {
    private:
        Model _model;
        View _view;
        LoadBlance _load_blance; //核心负载均衡器
    public:
        Control()
        {
            // cout << "Control success" << endl;
        }
        ~Control()
        {
        }

        bool AllQuestions(string *html)
        {
            vector<Question> all;
            if(!_model.GetAllQuestions(&all))
            {
                *html = "获取题目失败,形成题目列表失败";
                return false;
            }

            _view.ExpandAll(all, html);
        }

        bool OneQuestion(const string &number, string *html)
        {
            Question qut;
            if(!_model.GetOneQuestion(number, &qut))
            {
                *html = "题目: " + number + " 不存在!";
                return false;
            }

            _view.ExpandOne(qut, html);
            return true;
        }

        // code: #include...
        // input: ""
        bool Judge(const string &number, const std::string in_json, std::string *out_json)
        {
            
            //1. 根据题号,那到对于题目
            Question qut;
            _model.GetOneQuestion(number, &qut);

            //2.in_json反序列,得到用户的code
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            string code = in_value["code"].asString();

            //3.拼接用户代码+测试用例
            // Json::Value compile_value = in_value;
            // compile_value["code"] = code + "\n" + qut.tail;

            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + "\n" + qut.tail;
            compile_value["cpu_limit"] = qut.cpu_limit;
            compile_value["mem_limit"] = qut.mem_limit;
            Json::FastWriter writer;
            string compile_string = writer.write(compile_value);
            // std::cout << compile_string << std::endl;
            //4.选择负载最低的主机
            while(true)
            {
                int id = 0;
                Machine *m = nullptr;
                if(!_load_blance.SmartChoice(&id, &m))
                {
                    break;
                }

                //5.发起http请求
                Client cli(m->ip, m->port);
                m->IncLoad();
                LOG(INFO) << " 选择主机成功, 主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 当前主机的负载是: " << m->Load() << "\n";
                if(auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8"))
                {
                    //5.将结果给out_json
                    if(res->status == 200)
                    {
                        (*out_json) = res->body;
                        m->DecLoad();
                        LOG(INFO) << "请求编译和运行服务成功..." << "\n";
                        break;
                    }
                    m->DecLoad();
                }
                else
                {
                    //请求失败
                    LOG(ERROR) << " 当前请求的主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 可能已经离线"<< "\n";
                    _load_blance.OfflineMachine(id);
                    // _load_blance.ShowMachines(); //仅仅是为了用来调试
                }
            }
        }
    };
}