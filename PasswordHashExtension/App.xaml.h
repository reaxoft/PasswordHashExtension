//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"

namespace PasswordHashExtension
{
	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;
		virtual void OnBackgroundActivated(Windows::ApplicationModel::Activation::BackgroundActivatedEventArgs^ args) override;

	internal:
		App();

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
	public:
		void OnEdgeServiceCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);
		void NativeAppServiceCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);
		void EdgeServiceConnection_ServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection ^sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs ^args);
		void NativeAppServiceConnection_ServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection ^sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs ^args);
		void OnEdgeServiceRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection ^sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs ^args);
		void NativeAppServiceRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection ^sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs ^args);
	};
}
