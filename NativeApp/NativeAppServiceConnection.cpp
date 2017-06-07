#include "stdafx.h"

#include "NativeAppServiceConnection.h"
#include "NativeApp.h"
#include "Hash.h"

using namespace concurrency;
using namespace Windows::Foundation::Collections;

AppServiceConnection^ _connection = nullptr;
extern std::condition_variable cond_var;

IAsyncAction ^ConnectToService()
{
	return create_async([]
	{
		auto connection = ref new AppServiceConnection();
		connection->PackageFamilyName = Windows::ApplicationModel::Package::Current->Id->FamilyName;;
		connection->AppServiceName = "ru.reaxoft.PasswordHashExtension";

		create_task(connection->OpenAsync()).then([connection](AppServiceConnectionStatus status)
		{
			if (status == AppServiceConnectionStatus::Success)
			{
				_connection = connection;
				_connection->RequestReceived += ref new TypedEventHandler<AppServiceConnection^, AppServiceRequestReceivedEventArgs^>(RequestReceived);
				_connection->ServiceClosed += ref new TypedEventHandler<AppServiceConnection^, AppServiceClosedEventArgs^>(ServiceClosed);
			}
			else if (status == AppServiceConnectionStatus::AppUnavailable || status == AppServiceConnectionStatus::AppServiceUnavailable)
			{
				MessageBox(NULL, L"Help, Something went wrong.", L"Error", MB_ICONERROR | MB_OK);
			}
		});
	});
}


void RequestReceived(AppServiceConnection^ connection, AppServiceRequestReceivedEventArgs^ args)
{
	auto deferral = args->GetDeferral();
	auto key = args->Request->Message->First()->Current->Key;
	auto value = args->Request->Message->First()->Current->Value->ToString();

	std::thread mn{ [deferral, args, value] {
		std::unique_ptr<PasswordWindow> pw(new PasswordWindow());
		if (pw->Run() == 1) {
			wchar_t hash[65];
			if (CalcPasswordHash(hash, 65, value, pw->GetPassword())) {
				auto response = ref new ValueSet();
				response->Insert("response", ref new Platform::String(L"error"));
				create_task(args->Request->SendResponseAsync(response)).then([deferral](AppServiceResponseStatus status)
				{
					deferral->Complete();
				});
				return;
			}

			auto response = ref new ValueSet();
			response->Insert("response", ref new Platform::String(hash));
			create_task(args->Request->SendResponseAsync(response)).then([deferral](AppServiceResponseStatus status)
			{
				deferral->Complete();
			});
		};
	} };
	mn.detach();
}

void ServiceClosed(AppServiceConnection^ connection, AppServiceClosedEventArgs^ args)
{
	cond_var.notify_one();
}