// GalileoSDK.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#define BUILDING_DLL
#include "GalileoSDK.h"

namespace GalileoSDK {

	// Implementation of class GalileoSDK
	// ////////////////////////////////

	GalileoSDK::GalileoSDK() {
		new std::thread(&BroadcastReceiver::Run, broadcastReceiver);
	}

	std::vector<ServerInfo> GalileoSDK::GetServersOnline() {
		return BroadcastReceiver::GetServers();
	}

	GALILEO_RETURN_CODE GalileoSDK::Connect(std::string targetID="", bool auto_connect=true, bool reconnect=true, int timeout=10*1000,
		GALILEO_RETURN_CODE(*OnConnect)(std::string)=NULL, GALILEO_RETURN_CODE(*OnDisconnect)(std::string)=NULL) {
		if (targetID.empty() && auto_connect) {
			int timecount = 0;
			std::vector<ServerInfo> servers;
			while (timecount < timeout)
			{
				servers = BroadcastReceiver::GetServers();
				if (servers.size() == 1)
					break;
				Sleep(100);
				timecount += 100;
			}
			if (servers.size() == 1) {
				currentServer = servers.at(0);
			}
			if (servers.size() == 0) {
				return GALILEO_RETURN_CODE::NO_SERVER_FOUND;
			}
			if (servers.size() > 1) {
				return GALILEO_RETURN_CODE::MULTI_SERVER_FOUND;
			}
			std::map<std::string, std::string> params;
			// 设置 ros master地址
			std::stringstream masterURI;
			masterURI << "http://" << currentServer.getIP() << ":11311";
			params.insert(std::pair<std::string, std::string>("__master", masterURI.str()));
			// 设置自己的地址，否则默认使用hostname会无法通信
			// Utils::getMyIP()
			params.insert(std::pair<std::string, std::string>("__ip", "192.168.0.8"));
			ros::init(params, "galileo_sdk_client");
			ROS_INFO_STREAM("Connected to server: " << currentServer.getID());
		}
	}


	// Implementation of class ServerInfo
	// ////////////////////////////////

	ServerInfo::ServerInfo() {}

	ServerInfo::ServerInfo(Json::Value serverInfoJson) {
		ID = serverInfoJson["id"].asString();
		mac = serverInfoJson["mac"].asString();
		port = serverInfoJson["port"].asInt();
	}

	std::string ServerInfo::getMac() {
		return mac;
	}

	void ServerInfo::setMac(std::string mac) {
		this->mac = mac;
	}

	std::string ServerInfo::getPassword() {
		return password;
	}

	void ServerInfo::setPassword(std::string password) {
		this->password = password;
	}

	std::string ServerInfo::getIP() {
		return ip;
	}

	void ServerInfo::setIP(std::string ip) {
		this->ip = ip;
	}

	std::string ServerInfo::getID() {
		return ID;
	}

	void ServerInfo::setID(std::string ID) {
		this->ID = ID;
	}

	size_t ServerInfo::getTimestamp() {
		return timestamp;
	}

	void ServerInfo::setTimestamp(size_t timestamp) {
		this->timestamp = timestamp;
	}

	uint32_t ServerInfo::getPort() {
		return port;
	}

	void ServerInfo::setPort(uint32_t port) {
		this->port = port;
	}

	// Implementation of class BroadcastReceiver
	// ////////////////////////////////

	BroadcastReceiver* BroadcastReceiver::instance = NULL;

	BroadcastReceiver::BroadcastReceiver() {

		ROS_INFO_STREAM("BroadcastReceiver created");

		// 初始化广播udp socket
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			ROS_INFO_STREAM("Failed. Error Code : " << WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		ROS_INFO_STREAM("Initialised.");
		if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			ROS_INFO_STREAM("Could not create socket : " << WSAGetLastError());
		}
		ROS_INFO_STREAM("Socket created.");

		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(BROADCAST_PORT);
		if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
		{
			ROS_INFO_STREAM("Bind failed with error code : " << WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		ROS_INFO_STREAM("Bind done");
		instance = this;
	}

	void BroadcastReceiver::StopAll() {
		if (instance == NULL)
			return;
		instance->StopTask();
	}

	void BroadcastReceiver::StopTask() {
		runningFlag = false;
		instance = NULL;
	}

	std::vector<ServerInfo> BroadcastReceiver::GetServers() {
		if (instance == NULL) {
			return std::vector<ServerInfo>();
		}
		else {
			return instance->serverList;
		}
	}

	void replaceAll(std::string& str, const std::string& from, const std::string& to) {
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}

	void BroadcastReceiver::Run() {
		runningFlag = true;
		Json::CharReaderBuilder builder;
		Json::CharReader * reader = builder.newCharReader();
		instance = this;
		while (runningFlag)
		{
			ServerInfo* serverInfo = NULL;
			try
			{
				int timeout = 1000;
				if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
					ROS_ERROR_STREAM("Set socket timeout failed");
				}
				char buf[BUFLEN];
				int slen, recv_len;
				slen = sizeof(si_other);
				fflush(stdout);
				memset(buf, '\0', BUFLEN);
				if ((recv_len = recvfrom(serverSocket, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
				{
					//ROS_INFO_STREAM("recvfrom() failed with error code : " << WSAGetLastError());
				}
				char ipStr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(si_other.sin_addr), ipStr, INET_ADDRSTRLEN);
				std::string data = std::string(buf, 0, recv_len);
				if (!data.empty()) {
					Json::Value serverInfoJson;
					std::string errors;
					bool parsingSuccessful = reader->parse(data.c_str(), data.c_str() + data.size(), &serverInfoJson, &errors);
					if (!parsingSuccessful) {
						ROS_INFO_STREAM(errors);
					}
					else {
						serverInfo = new ServerInfo(serverInfoJson);
						std::string addrStr = std::string(ipStr);
						replaceAll(addrStr, "/", "");
						serverInfo->setIP(addrStr);
					}
				}
			}
			catch (const std::exception& e)
			{
				ROS_ERROR_STREAM(e.what());
			}

			std::vector<ServerInfo> result;
			for (auto it = serverList.begin(); it < serverList.end(); it++) {
				if (Utils::GetCurrentTimestamp() - it->getTimestamp() > 1000 * 10) {
					ROS_INFO_STREAM("Server: " << it->getID() << " offline");
				}
				else {
					result.push_back(*it);
				}
			}
			serverList = result;

			if (serverInfo == NULL) {
				continue;
			}

			// 更新服务器时间戳
			bool newServerFlag = true;
			for (auto it = serverList.begin(); it < serverList.end(); it++) {
				if (it->getID() == serverInfo->getID()) {
					it->setTimestamp(Utils::GetCurrentTimestamp());
					it->setIP(serverInfo->getIP());
					it->setMac(serverInfo->getMac());
					newServerFlag = false;
				}
			}

			// 更新服务器列表
			if (newServerFlag) {
				serverInfo->setTimestamp(Utils::GetCurrentTimestamp());
				ROS_INFO_STREAM(Utils::GetCurrentTimestamp());
				serverList.push_back(*serverInfo);
			}
			free(serverInfo);
		}
		closesocket(serverSocket);
		WSACleanup();
	}
}

