#pragma once

#include "stdafx.h"

using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;

void ServiceClosed(AppServiceConnection^ connection, AppServiceClosedEventArgs^ args);
void RequestReceived(AppServiceConnection^ connection, AppServiceRequestReceivedEventArgs^ args);
IAsyncAction ^ConnectToService();