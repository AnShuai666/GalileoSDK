// GalileoSDKTest.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "GalileoSDK.h"
#include "galileo_serial_server/GalileoStatus.h"

int main()
{
	GalileoSDK::GalileoSDK sdk;
	while (true) {
		auto servers = sdk.GetServersOnline();
		for (auto it = servers.begin(); it < servers.end(); it++) {
			std::cout << it->getID() << std::endl;
			sdk.Connect("", true, false, 10000, NULL, NULL);
		}
		//sdk.PublishTest();
		galileo_serial_server::GalileoStatusPtr status;
		if (sdk.GetCurrentStatus(status) == GalileoSDK::OK) {
			std::cout << status->power << std::endl;
		}
		else {
			std::cout << "Get status failed" << std::endl;
		}
		Sleep(1000);
	}
    return 0;
}

