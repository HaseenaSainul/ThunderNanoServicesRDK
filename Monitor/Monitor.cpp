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
 
#include "Monitor.h"

namespace Thunder {
namespace Plugin {

    namespace {

        static Metadata<Monitor> metadata(
            // Version
            1, 0, 1,
            // Preconditions
            { subsystem::TIME },
            // Terminations
            {},
            // Controls
            {}
        );
    }

    /* virtual */ const string Monitor::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

        _config.FromString(service->ConfigLine());

        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

        Core::JSON::ArrayType<Config::Entry>::Iterator index(_config.Observables.Elements());

        // Create a list of plugins to monitor..
        _monitor.Open(service, index);

        // During the registartion, all Plugins, currently active are reported to the sink.
        service->Register(&_monitor);

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
        RegisterAll();
#else
        Register(&_notification);
        Exchange::JMonitor::Register(*this, this);
#endif

        // On succes return a name as a Callsign to be used in the URL, after the "service"prefix
        return (_T(""));
    }

    /* virtual */ void Monitor::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
        UnregisterAll();
#else
        Unregister(&_notification);
        Exchange::JMonitor::Unregister(*this);
#endif

        service->Unregister(&_monitor);

        _monitor.Close();
    }

    /* virtual */ string Monitor::Information() const
    {
        // No additional info to report.
        return (nullptr);
    }

#if !defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    void Monitor::NotifyAction(const string& callsign, const Exchange::IMonitor::INotification::action action, const string& reason)
    {
        _adminLock.Lock();
        for (auto* notification : _notifications) {
            notification->Action(callsign, action, reason);
        }
        _adminLock.Unlock();
    }

    /* virtual */ Core::hresult Monitor::Register(Exchange::IMonitor::INotification* notification)
    {
        ASSERT(notification);

        _adminLock.Lock();
        auto item = std::find(_notifications.begin(), _notifications.end(), notification);
        ASSERT(item == _notifications.end());
        if (item == _notifications.end()) {
            notification->AddRef();
            _notifications.push_back(notification);
        }
        _adminLock.Unlock();

        return Core::ERROR_NONE;
    }
    /* virtual */ Core::hresult Monitor::Unregister(Exchange::IMonitor::INotification* notification)
    {
        ASSERT(notification);

        _adminLock.Lock();
        auto item = std::find(_notifications.begin(), _notifications.end(), notification);
        ASSERT(item != _notifications.end());
        _notifications.erase(item);
        (*item)->Release();
        _adminLock.Unlock();
        return Core::ERROR_NONE;
    }

    /* virtual */ Core::hresult Monitor::RestartLimits(const string& callsign, const Exchange::IMonitor::RestartInfo& params)
    {
        _monitor.RestartInfo(callsign, params);

        return Core::ERROR_NONE;
    }

    /* virtual */ Core::hresult Monitor::RestartLimits(const string& callsign, Exchange::IMonitor::RestartInfo& params) const
    {
        _monitor.RestartInfo(callsign, params);

        return Core::ERROR_NONE;
    }
    /* virtual */ Core::hresult Monitor::Reset(const string& callsign)
    {
        if (callsign.empty() != false) {
            _monitor.Reset(callsign);
        }
        return Core::ERROR_NONE;
    }
    /* virtual */ Core::hresult Monitor::Observables(Exchange::IMonitor::IStringIterator*& observables) const
    {
        std::list<string> observableList;
        _monitor.Observables(observableList);
        if (observableList.empty() != false) {
            using Iterator = Exchange::IMonitor::IStringIterator;
            observables = Core::ServiceType<RPC::IteratorType<Iterator>>::Create<Iterator>(observableList);
	}
        return (observables != nullptr ? Core::ERROR_NONE : Core::ERROR_UNAVAILABLE);

    }
    /* virtual */ Core::hresult Monitor::StatisticsInfo(const string& callsign, Exchange::IMonitor::Statistics& statistics) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (callsign.empty() == false) {
            result = _monitor.Statistics(callsign, statistics);
        }
        return result;
    }
#endif

}
}
