/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BROWSER_H
#define __BROWSER_H

#include "Module.h"
#if ENABLE_LEGACY_INTERFACE_SUPPORT
#include <interfaces/IBrowser.h>
#else
#include <interfaces/IBrowserExt.h>
#endif
#include <interfaces/IApplication.h>
#include <interfaces/IMemory.h>

#include <interfaces/json/JsonData_Browser.h>
#include <interfaces/json/JBrowserScripting.h>
#include <interfaces/json/JBrowserCookieJar.h>
#include <interfaces/json/JWebBrowser.h>

#if ENABLE_LEGACY_INTERFACE_SUPPORT
#include <interfaces/json/JsonData_StateControl.h>
#include <interfaces/json/JsonData_WebKitBrowser.h>
#else
#include <plugins/json/JsonData_StateControl.h>
#include <plugins/json/JStateControl.h>
#endif
namespace Thunder {

namespace WebKitBrowser {
    // An implementation file needs to implement this method to return an operational browser, wherever that would be :-)
    Exchange::IMemory* MemoryObserver(const RPC::IRemoteConnection* connection);
}

namespace Plugin {

    class WebKitBrowser : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    private:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public PluginHost::IStateControl::INotification,
                             public Exchange::IWebBrowser::INotification,
                             public Exchange::IBrowserCookieJar::INotification {
        private:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            explicit Notification(WebKitBrowser* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification() override = default;

        public:
            void LoadFinished(const string& URL, int32_t code) override
            {
                _parent.LoadFinished(URL, code);
            }
            void LoadFailed(const string& URL) override
            {
                _parent.LoadFailed(URL);
            }
            void URLChange(const string& URL, bool loaded) override
            {
                _parent.URLChange(URL, loaded);
            }
            void VisibilityChange(const bool hidden) override
            {
                _parent.VisibilityChange(hidden);
            }
            void PageClosure() override
            {
                _parent.PageClosure();
            }
            void BridgeQuery(const string& message) override
            {
                _parent.BridgeQuery(message);
            }
            void StateChange(const PluginHost::IStateControl::state state) override
            {
                _parent.StateChange(state);
            }
            void Activated(RPC::IRemoteConnection* /* connection */) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }
            void CookieJarChanged() override
            {
                _parent.CookieJarChanged();
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(Exchange::IWebBrowser::INotification)
            INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            INTERFACE_ENTRY(Exchange::IBrowserCookieJar::INotification)
            END_INTERFACE_MAP

        private:
            WebKitBrowser& _parent;
        };

    public:
        class Data : public Core::JSON::Container {
        private:
            Data(const Data&) = delete;
            Data& operator=(const Data&) = delete;

        public:
            Data()
                : Core::JSON::Container()
                , URL()
                , FPS()
                , Suspended(false)
                , Hidden(false)
                , Path()
            {
                Add(_T("url"), &URL);
                Add(_T("fps"), &FPS);
                Add(_T("suspended"), &Suspended);
                Add(_T("hidden"), &Hidden);
                Add(_T("path"), &Path);
            }
            ~Data() = default;

        public:
            Core::JSON::String URL;
            Core::JSON::DecUInt32 FPS;
            Core::JSON::Boolean Suspended;
            Core::JSON::Boolean Hidden;
            Core::JSON::String Path;
        };

    public:
        WebKitBrowser(const WebKitBrowser&) = delete;
        WebKitBrowser& operator=(const WebKitBrowser&) = delete;
        WebKitBrowser()
            : _skipURL(0)
            , _connectionId(0)
            , _service(nullptr)
            , _browser(nullptr)
            , _memory(nullptr)
            , _application(nullptr)
            , _stateControl(nullptr)
            , _browserScripting(nullptr)
            , _cookieJar(nullptr)
            , _notification(this)
            , _jsonBodyDataFactory(2)
        {
        }

        ~WebKitBrowser() override = default;

        inline static bool EnvironmentOverride(const bool configFlag)
        {
            bool result = configFlag;

            if (result == false) {
                string value;
                Core::SystemInfo::GetEnvironment(_T("WPE_ENVIRONMENT_OVERRIDE"), value);
                result = (value == "1");
            }
            return (result);
        }

    public:
        BEGIN_INTERFACE_MAP(WebKitBrowser)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(PluginHost::IStateControl, _stateControl)
        INTERFACE_AGGREGATE(Exchange::IBrowser, _browser)
        INTERFACE_AGGREGATE(Exchange::IApplication, _application)
        INTERFACE_AGGREGATE(Exchange::IWebBrowser, _browser)
        INTERFACE_AGGREGATE(Exchange::IBrowserScripting, _browserScripting)
        INTERFACE_AGGREGATE(Exchange::IBrowserCookieJar, _cookieJar)
        INTERFACE_AGGREGATE(Exchange::IMemory, _memory)
        END_INTERFACE_MAP

    public:
        //  IPlugin methods
        // -------------------------------------------------------------------------------------------------------

        // First time initialization. Whenever a plugin is loaded, it is offered a Service object with relevant
        // information and services for this particular plugin. The Service object contains configuration information that
        // can be used to initialize the plugin correctly. If Initialization succeeds, return nothing (empty string)
        // If there is an error, return a string describing the issue why the initialisation failed.
        // The Service object is *NOT* reference counted, lifetime ends if the plugin is deactivated.
        // The lifetime of the Service object is guaranteed till the deinitialize method is called.
        const string Initialize(PluginHost::IShell* service) override;

        // The plugin is unloaded from Thunder. This is call allows the module to notify clients
        // or to persist information if needed. After this call the plugin will unlink from the service path
        // and be deactivated. The Service object is the same as passed in during the Initialize.
        // After theis call, the lifetime of the Service object ends.
        void Deinitialize(PluginHost::IShell* service) override;

        // Returns an interface to a JSON struct that can be used to return specific metadata information with respect
        // to this plugin. This Metadata can be used by the MetData plugin to publish this information to the ouside world.
        string Information() const override;

    private:
        void Deactivated(RPC::IRemoteConnection* connection);
        void LoadFinished(const string& URL, int32_t code);
        void LoadFailed(const string& URL);
        void URLChange(const string& URL, bool loaded);
        void VisibilityChange(const bool hidden);
        void PageClosure();
        void BridgeQuery(const string& message);
        void StateChange(const PluginHost::IStateControl::state state);
        void CookieJarChanged();

#if ENABLE_LEGACY_INTERFACE_SUPPORT
        uint32_t DeleteDir(const string& path);

        // JsonRpc
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_state(Core::JSON::EnumType<JsonData::StateControl::StateType>& response) const; // StateControl
        uint32_t set_state(const Core::JSON::EnumType<JsonData::StateControl::StateType>& param); // StateControl
        uint32_t endpoint_delete(const JsonData::Browser::DeleteParamsData& params);
        uint32_t get_languages(Core::JSON::ArrayType<Core::JSON::String>& response) const;
        uint32_t set_languages(const Core::JSON::ArrayType<Core::JSON::String>& param);
        uint32_t get_headers(Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& response) const;
        uint32_t set_headers(const Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& param);
        void event_bridgequery(const string& message);
        void event_statechange(const bool& suspended); // StateControl
#endif

    private:
        uint8_t _skipURL;
        uint32_t _connectionId;
        PluginHost::IShell* _service;
        Exchange::IWebBrowser* _browser;
        Exchange::IMemory* _memory;
        Exchange::IApplication* _application;
        PluginHost::IStateControl* _stateControl;
        Exchange::IBrowserScripting* _browserScripting;
        Exchange::IBrowserCookieJar* _cookieJar;
        Core::SinkType<Notification> _notification;
        Core::ProxyPoolType<Web::JSONBodyType<WebKitBrowser::Data>> _jsonBodyDataFactory;
        string _persistentStoragePath;
    };
}
}

#endif // __BROWSER_H
