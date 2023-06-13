#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>



int main()
{
    Json::Value root;
    root["name"] = "小明";
    root["age"] = 10;
    root["email"] = "12345@qq.com";

    Json::StyledWriter writer;
    std::string str = writer.write(root);
    std::cout << str << std::endl;

    Json::Value in_value;
    Json::Reader reader;
    reader.parse(str, in_value);
    std::string name = in_value["name"].asString();
    std::string email = in_value["email"].asString();
    int age = in_value["age"].asInt();
    std::cout << name << " " << age << " " << email << std::endl;

    return 0;
}