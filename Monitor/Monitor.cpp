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
#endif

        // On succes return a name as a Callsign to be used in the URL, after the "service"prefix
        return (_T(""));
    }

    /* virtual */ void Monitor::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
        UnregisterAll();
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
    /* virtual */ uint32_t Monitor::RestartLimits(const Exchange::IMonitor::RestartLimitsInfo& params)
    {
        _monitor.Update(params.callsign, params.restart.window, params.restart.limit);

        return Core::ERROR_NONE;
    }

    /* virtual */ uint32_t Monitor::ResetStats(const string& callsign, Exchange::IMonitor::Statistics& statistics)
    {
        std::list<Exchange::IMonitor::Statistics> statisticsList;
	_monitor.Snapshot(callsign, statisticsList);
        if (statisticsList.size() == 1) {
            _monitor.Reset(callsign);
            statistics = statisticsList.front();
        }
        return Core::ERROR_NONE;
    }

    /* virtual */ uint32_t Monitor::Status(const string& callsign, Exchange::IMonitor::IStatisticsIterator*& statistics) const
    {
        std::list<Exchange::IMonitor::Statistics> statisticsList;
	_monitor.Snapshot(callsign, statisticsList);
	using Iterator = Exchange::IMonitor::IStatisticsIterator;
        statistics = Core::ServiceType<RPC::IteratorType<Iterator>>::Create<Iterator>(statisticsList); 
        return Core::ERROR_NONE;
    }
#endif

}
}
