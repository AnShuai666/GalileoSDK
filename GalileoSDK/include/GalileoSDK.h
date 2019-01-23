#pragma once
#ifndef __GALILEO_SDK_H__
#define __GALILEO_SDK_H__

#include "stdafx.h"
#include <iostream>

#ifdef _WIN32
#define WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

#include "Dll.h"
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "galileo_serial_server/GalileoNativeCmds.h"
#include "galileo_serial_server/GalileoStatus.h"
#include "models/ServerInfo.h"
#include "tasks/BroadcastReceiver.h"
#include <thread>

namespace GalileoSDK {

	enum GALILEO_RETURN_CODE {
		OK, // ִ�гɹ�
		NOT_CONNECTED, // û�����ӵ�������
		INVALIDE_STATE, // ��ǰ״̬�޷�ִ�д˲���
		NO_SERVER_FOUND, // û�з��ֻ�����
		MULTI_SERVER_FOUND, // ���ֶ��������
	};

	class DLL_PUBLIC GalileoSDK {
	public:
		GalileoSDK();
		GALILEO_RETURN_CODE Connect(std::string targetID, bool auto_connect, bool reconnect, int timeout,
			GALILEO_RETURN_CODE(*OnConnect)(std::string), GALILEO_RETURN_CODE (*OnDisconnect)(std::string));
		GALILEO_RETURN_CODE WaitForConnect(std::string targetID, bool auto_connect, bool reconnect, int timeout,
			GALILEO_RETURN_CODE(*OnConnect)(std::string), GALILEO_RETURN_CODE(*OnDisconnect)(std::string));
		GALILEO_RETURN_CODE StartNavigation(GALILEO_RETURN_CODE(*OnConnect)(std::string));
		GALILEO_RETURN_CODE WaitForStartNavigation(GALILEO_RETURN_CODE(*OnConnect)(std::string));
		//DLL_PUBLIC std::vector<ServerInfo>  GetServersOnline();
		std::vector<ServerInfo> GetServersOnline();
		ServerInfo currentServer;
	private:
		BroadcastReceiver broadcastReceiver;
	};

	extern "C" DLL_PUBLIC GalileoSDK* GetSDK();
};

#endif // !__GALILEO_SDK_H__
