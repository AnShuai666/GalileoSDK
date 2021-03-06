#pragma once
#ifndef __SERVER_INFO_H__
#define __SERVER_INFO_H__

#include "../Dll.h"
#include <iostream>
#include "../json.hpp"
#include <string>

namespace GalileoSDK
{
class DLL_PUBLIC ServerInfo
{
public:
    ServerInfo();
    ServerInfo(nlohmann::json);
    std::string getMac();
    void setMac(std::string);
    std::string getPassword();
    void setPassword(std::string);
    std::string getIP();
    void setIP(std::string);
    std::string getID();
    void setID(std::string);
    int64_t getTimestamp();
    void setTimestamp(int64_t);
    uint32_t getPort();
    void setPort(uint32_t);
    nlohmann::json toJson();
    std::string toJsonString();
    bool operator==(const ServerInfo &p2);

private:
    std::string ID;
    uint32_t port;
    int64_t timestamp;
    std::string ip = "";
    std::string password = "xiaoqiang";
    std::string mac;
};
} // namespace GalileoSDK

#endif // !__SERVER_INFO_H__
