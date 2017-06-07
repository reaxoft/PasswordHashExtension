//
// App.xaml.cpp
// Implementation of the App class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <atomic>
#include <string> 


using namespace PasswordHashExtension;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>

String ^extensionId = ref new String(L"EdgeExtension_9dc4e7230a7143cc81d472269d59e6a8_c7zhe02wczndm");
std::atomic<int> edgeConnectionIndex;
std::atomic<int> nativeAppConnectionIndex;
IMap<String^, AppServiceConnection^>^ nativeAppConnections = ref new Map<String^, AppServiceConnection^>();
IMap<String^, AppServiceConnection^>^ edgeConnections = ref new Map<String^, AppServiceConnection^>();
IMap<String^, BackgroundTaskDeferral^>^ edgeServiceDeferrals = ref new Map<String^, BackgroundTaskDeferral^>();
IMap<String^, BackgroundTaskDeferral^>^ nativeAppServiceDeferrals = ref new Map<String^, BackgroundTaskDeferral^>();
String ^EdgePackageFamalyName = ref new String(L"Microsoft.MicrosoftEdge_8wekyb3d8bbwe");

void completeDeferral(IMap<String^, BackgroundTaskDeferral^>^ deferrals, String ^serviceName)
{
	if (deferrals->HasKey(serviceName)) {
		BackgroundTaskDeferral ^dererral = deferrals->Lookup(serviceName);
		dererral->Complete();
		deferrals->Remove(serviceName);
	}
}

void completeDeferrals(String ^serviceName)
{
	completeDeferral(edgeServiceDeferrals, serviceName);
	completeDeferral(nativeAppServiceDeferrals, serviceName);
}

App::App()
{
    InitializeComponent();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
#if _DEBUG
    // Show graphics profiling information while debugging.
    if (IsDebuggerPresent())
    {
        // Display the current frame rate counters
         DebugSettings->EnableFrameRateCounter = true;
    }
#endif
    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and associate it with
        // a SuspensionManager key
        rootFrame = ref new Frame();

        rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

        if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
        {
            // TODO: Restore the saved session state only when appropriate, scheduling the
            // final launch steps after the restore is complete

        }

        if (e->PrelaunchActivated == false)
        {
            if (rootFrame->Content == nullptr)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
            }
            // Place the frame in the current Window
            Window::Current->Content = rootFrame;
            // Ensure the current window is active
            Window::Current->Activate();
        }
    }
    else
    {
        if (e->PrelaunchActivated == false)
        {
            if (rootFrame->Content == nullptr)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
            }
            // Ensure the current window is active
            Window::Current->Activate();
        }
    }
}

/// <summary>
/// Invoked when application execution is being suspended.  Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
    (void) sender;  // Unused parameter
    (void) e;   // Unused parameter

    //TODO: Save application state and stop any background activity
}

/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
    throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}

void App::OnBackgroundActivated(Windows::ApplicationModel::Activation::BackgroundActivatedEventArgs^ args)
{		
	IBackgroundTaskInstance ^taskInstance = args->TaskInstance;
	if (taskInstance->TriggerDetails->GetType()->GetTypeCode == AppServiceTriggerDetails::typeid->GetTypeCode) {
		AppServiceTriggerDetails ^appService = dynamic_cast<AppServiceTriggerDetails^>(taskInstance->TriggerDetails);
		String ^callerPackName = appService->CallerPackageFamilyName;
		if (callerPackName == Package::Current->Id->FamilyName) 
		{		
			BackgroundTaskDeferral^ nativeAppServiceDeferral = taskInstance->GetDeferral();
			taskInstance->Canceled += ref new BackgroundTaskCanceledEventHandler(this, &App::NativeAppServiceCanceled);
			AppServiceConnection ^nativeAppConnection = appService->AppServiceConnection;
			nativeAppConnection->RequestReceived += ref new TypedEventHandler<AppServiceConnection^, AppServiceRequestReceivedEventArgs^>(this, &App::NativeAppServiceRequestReceived);
			nativeAppConnection->ServiceClosed += ref new TypedEventHandler<AppServiceConnection^, AppServiceClosedEventArgs^>(this, &App::NativeAppServiceConnection_ServiceClosed);
			int index = nativeAppConnectionIndex++;
			nativeAppConnection->AppServiceName = ref new String(std::to_wstring(index).c_str());			
			nativeAppConnections->Insert(nativeAppConnection->AppServiceName, nativeAppConnection);
			nativeAppServiceDeferrals->Insert(nativeAppConnection->AppServiceName, nativeAppServiceDeferral);
			if (!edgeConnections->HasKey(nativeAppConnection->AppServiceName)) {
				throw Exception::CreateException(-30, "Edge connection not found.");
			}
			edgeConnections->Lookup(nativeAppConnection->AppServiceName)->RequestReceived += ref new TypedEventHandler<AppServiceConnection^, AppServiceRequestReceivedEventArgs^>(this, &App::OnEdgeServiceRequestReceived);
		}
		else if (callerPackName == EdgePackageFamalyName)
		{
			BackgroundTaskDeferral^ edgeServiceDeferral = taskInstance->GetDeferral();
			taskInstance->Canceled += ref new BackgroundTaskCanceledEventHandler(this, &App::OnEdgeServiceCanceled);
			AppServiceConnection ^edgeConnection = appService->AppServiceConnection;
			edgeConnection->ServiceClosed += ref new TypedEventHandler<AppServiceConnection^, AppServiceClosedEventArgs^>(this, &App::EdgeServiceConnection_ServiceClosed);
			int index = edgeConnectionIndex++;
			edgeConnection->AppServiceName = ref new String(std::to_wstring(index).c_str());
			edgeConnections->Insert(edgeConnection->AppServiceName, edgeConnection);
			edgeServiceDeferrals->Insert(edgeConnection->AppServiceName, edgeServiceDeferral);

			IAsyncAction ^act = Windows::ApplicationModel::FullTrustProcessLauncher::LaunchFullTrustProcessForCurrentAppAsync();
		}
		else {
			throw Exception::CreateException(-10, "Caller is not allowed.");
		}
	}
	
}

void App::EdgeServiceConnection_ServiceClosed(AppServiceConnection ^sender, AppServiceClosedEventArgs ^args) {
	if (nativeAppConnections->HasKey(sender->AppServiceName)) {
		nativeAppConnections->Remove(sender->AppServiceName);
	}
	completeDeferrals(sender->AppServiceName);
}

void App::NativeAppServiceConnection_ServiceClosed(AppServiceConnection ^sender, AppServiceClosedEventArgs ^args) {
	if (edgeConnections->HasKey(sender->AppServiceName)) {
		edgeConnections->Remove(sender->AppServiceName);
	}
	completeDeferrals(sender->AppServiceName);
}

void App::OnEdgeServiceCanceled(IBackgroundTaskInstance ^sender, BackgroundTaskCancellationReason reason)
{
	AppServiceTriggerDetails ^appService = dynamic_cast<AppServiceTriggerDetails^>(sender->TriggerDetails);
	String ^serviceName = appService->AppServiceConnection->AppServiceName;
	if (nativeAppConnections->HasKey(serviceName)) {
		nativeAppConnections->Remove(serviceName);
	}
	completeDeferrals(serviceName);	
}

void App::NativeAppServiceCanceled(IBackgroundTaskInstance ^sender, BackgroundTaskCancellationReason reason) {
	AppServiceTriggerDetails ^appService = dynamic_cast<AppServiceTriggerDetails^>(sender->TriggerDetails);
	String ^serviceName = appService->AppServiceConnection->AppServiceName;
	if (edgeConnections->HasKey(serviceName)) {
		edgeConnections->Remove(serviceName);
	}
	completeDeferrals(serviceName);
}

void App::OnEdgeServiceRequestReceived(AppServiceConnection ^sender, AppServiceRequestReceivedEventArgs ^args) {
	AppServiceDeferral^ messageDeferral = args->GetDeferral();
	try {
		String^ serviceName = sender->AppServiceName;
		if (!nativeAppConnections->HasKey(serviceName)) {
			throw Exception::CreateException(-30, "Native app connection not found.");
		}
		AppServiceConnection^ nativeAppConnection = nativeAppConnections->Lookup(serviceName);
		IAsyncOperation<AppServiceResponse ^> ^op = nativeAppConnection->SendMessageAsync(args->Request->Message);
		try
		{
			concurrency::create_task(op).then([args](AppServiceResponse^ asr) {
				concurrency::create_task(args->Request->SendResponseAsync(asr->Message)).get();
			}).get();
		}
		catch (const std::exception&)
		{
			throw Exception::CreateException(-40, "Error occurred while transmitting a message to native app.");
		}
		messageDeferral->Complete();
	}
	catch (...) {
		messageDeferral->Complete();
		throw;
	}
}

void App::NativeAppServiceRequestReceived(AppServiceConnection ^sender, AppServiceRequestReceivedEventArgs ^args) {
	AppServiceDeferral^ messageDeferral = args->GetDeferral();
	try 
	{
		if (!edgeConnections->HasKey(sender->AppServiceName)) {
			throw Exception::CreateException(-20, "Edge connection not found.");
		}
		AppServiceConnection ^edgeConnection = edgeConnections->Lookup(sender->AppServiceName);
		IAsyncOperation<AppServiceResponse ^> ^op = edgeConnection->SendMessageAsync(args->Request->Message);
		
		try {
			concurrency::create_task(op).get();
		}
		catch (const std::exception&)
		{
			throw Exception::CreateException(-10, "Error occurred while sending message.");
		}
		messageDeferral->Complete();
	}
	catch (...) {
		messageDeferral->Complete();
		throw;		
	}
}