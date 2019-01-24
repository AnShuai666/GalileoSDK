// GalileoSDKTest.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "GalileoSDK.h"
#include "galileo_serial_server/GalileoStatus.h"

/// 测试发布消息
void testPub() {
	GalileoSDK::GalileoSDK sdk;
	auto servers = sdk.GetServersOnline();
	for (auto it = servers.begin(); it < servers.end(); it++) {
		std::cout << it->getID() << std::endl;
		sdk.Connect("", true, false, 10000, NULL, NULL);
	}
	sdk.PublishTest();
}


/// 测试订阅消息
void testSub() {
	GalileoSDK::GalileoSDK sdk;
	sdk.Connect("", true, false, 10000, NULL, NULL);
	while (true) {
		galileo_serial_server::GalileoStatus* status = new galileo_serial_server::GalileoStatus();
		if (sdk.GetCurrentStatus(status) == GalileoSDK::OK) {
			std::cout << status->power << std::endl;
		}
		else {
			std::cout << "Get status failed" << std::endl;
		}
		Sleep(1000);
	}
}

bool connected = false;
void testConnectWithCallback() {
	connected = false;
	GalileoSDK::GalileoSDK sdk;
	sdk.Connect("", true, false, 10000, [](GalileoSDK::GALILEO_RETURN_CODE res, std::string id)->void {
		std::cout << "OnConnect Callback: result " << res << std::endl;
		std::cout << "OnConnect Callback: connected to " << id << std::endl;
		connected = true;
	}, NULL);
	while (!connected)
	{
		Sleep(1);
	}
}

void testReconnect() {
	GalileoSDK::GalileoSDK sdk;
	sdk.Connect("", true, true, 10000, [](GalileoSDK::GALILEO_RETURN_CODE res, std::string id)->void {
		std::cout << "OnConnect Callback: result " << res << std::endl;
		std::cout << "OnConnect Callback: connected to " << id << std::endl;
		connected = true;
	}, [](GalileoSDK::GALILEO_RETURN_CODE res, std::string id)->void {
		std::cout << "OnDisconnect Callback: result " << res << std::endl;
		std::cout << "OnDisconnect Callback: server " << id << std::endl;
	});
	while (true)
	{
		Sleep(1);
	}
}

int main()
{
	testReconnect();
    return 0;
}

