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

#ifndef DEVICEINFO_DEVICEINFO_H
#define DEVICEINFO_DEVICEINFO_H

#include "Module.h"
#if ENABLE_LEGACY_INTERFACE_SUPPORT
#include <interfaces/IDeviceInfo.h>
#include <interfaces/json/JsonData_DeviceInfo.h>
#else
#include <interfaces/json/JAddressMetadata.h>
#include <interfaces/json/JDeviceInfoExt.h>
#include <interfaces/json/JDeviceAudioCapabilities.h>
#include <interfaces/json/JDeviceVideoCapabilities.h>
#include <interfaces/json/JDeviceMetadata.h>
#include <interfaces/json/JImageMetadata.h>
#include <interfaces/json/JSocketMetadata.h>
#include <interfaces/json/JSystemMetadata.h>
#include <interfaces/IConfiguration.h>
#endif

namespace Thunder {
namespace Plugin {

    class DeviceInfo : public PluginHost::IPlugin, public PluginHost::JSONRPC {

    public:

    private:
        class Notification : public RPC::IRemoteConnection::INotification
#if ENABLE_LEGACY_INTERFACE_SUPPORT
                             , public PluginHost::ISubSystem::INotification
#endif
        {
#if ENABLE_LEGACY_INTERFACE_SUPPORT
        private:
            class Job {
            public:
                Job() = delete;
                Job(const Job&) = delete;
                Job& operator=(const Job&) = delete;

                Job(DeviceInfo& parent) : _parent(parent) { }
                ~Job() = default;

            public:
                void Dispatch() {
                    _parent.UpdateDeviceIdentifier();
                }

            private:
                DeviceInfo& _parent;
            };
#endif

        public:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

            explicit Notification(DeviceInfo& parent)
                : _parent(parent)
#if ENABLE_LEGACY_INTERFACE_SUPPORT
                , _job(parent)
#endif
            {
            }
            ~Notification() override {
#if ENABLE_LEGACY_INTERFACE_SUPPORT
                _job.Revoke();
#endif
            }

        public:
#if ENABLE_LEGACY_INTERFACE_SUPPORT
            // Some changes happened in the subsystems
            void Updated() override {
                _job.Submit();
            }
#endif
            void Activated(RPC::IRemoteConnection* /* connection */) override {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override {
                _parent.Deactivated(connection);
            }
            void Terminated(RPC::IRemoteConnection* /* connection */) override {
            }

            BEGIN_INTERFACE_MAP(Notification)
#if ENABLE_LEGACY_INTERFACE_SUPPORT
                INTERFACE_ENTRY(PluginHost::ISubSystem::INotification)
#endif
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            DeviceInfo& _parent;
#if ENABLE_LEGACY_INTERFACE_SUPPORT
            Core::WorkerPool::JobType<Job> _job;
#endif
        };

    public:
        DeviceInfo(const DeviceInfo&) = delete;
        DeviceInfo& operator=(const DeviceInfo&) = delete;
        DeviceInfo()
            : _skipURL(0)
            , _service(nullptr)
#if ENABLE_LEGACY_INTERFACE_SUPPORT
            , _subSystem(nullptr)
            , _deviceId()
#endif
            , _deviceInfo(nullptr)
            , _deviceAudioCapabilityInterface(nullptr)
            , _deviceVideoCapabilityInterface(nullptr)
#if !defined(ENABLE_LEGACY_INTERFACE_SUPPORT) || (ENABLE_LEGACY_INTERFACE_SUPPORT == 0)
            , _addressMetadata(nullptr)
            , _deviceMetadata(nullptr)
            , _imageMetadata(nullptr)
            , _socketMetadata(nullptr)
            , _systemMetadata(nullptr)
#endif
            , _connectionId(0)
#if ENABLE_LEGACY_INTERFACE_SUPPORT
            , _adminLock()
#endif
            , _notification(*this)
        {
        }

        ~DeviceInfo() override = default;

        BEGIN_INTERFACE_MAP(DeviceInfo)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IDeviceInfo, _deviceInfo)
#if !defined(ENABLE_LEGACY_INTERFACE_SUPPORT) || (ENABLE_LEGACY_INTERFACE_SUPPORT == 0)
        INTERFACE_AGGREGATE(Exchange::IDeviceAudioCapabilities, _deviceAudioCapabilityInterface)
        INTERFACE_AGGREGATE(Exchange::IDeviceVideoCapabilities, _deviceVideoCapabilityInterface)
        INTERFACE_AGGREGATE(Exchange::IAddressMetadata, _addressMetadata)
        INTERFACE_AGGREGATE(Exchange::IDeviceMetadata, _deviceMetadata)
        INTERFACE_AGGREGATE(Exchange::IImageMetadata, _imageMetadata)
        INTERFACE_AGGREGATE(Exchange::ISocketMetadata, _socketMetadata)
        INTERFACE_AGGREGATE(Exchange::ISystemMetadata, _systemMetadata)
#endif
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

    private:
#if ENABLE_LEGACY_INTERFACE_SUPPORT
        // JsonRpc
        void RegisterAll();
        void UnregisterAll();
        uint32_t endpoint_supportedresolutions(const JsonData::DeviceInfo::SupportedresolutionsParamsInfo& params, JsonData::DeviceInfo::SupportedresolutionsResultData& response);
        uint32_t endpoint_defaultresolution(const JsonData::DeviceInfo::SupportedresolutionsParamsInfo& params, JsonData::DeviceInfo::DefaultresolutionResultData& response);
        uint32_t endpoint_supportedhdcp(const JsonData::DeviceInfo::SupportedresolutionsParamsInfo& params, JsonData::DeviceInfo::SupportedhdcpResultData& response);
        uint32_t endpoint_audiocapabilities(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::AudiocapabilitiesResultData& response);
        uint32_t endpoint_ms12capabilities(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::Ms12capabilitiesResultData& response);
        uint32_t endpoint_supportedms12audioprofiles(const JsonData::DeviceInfo::AudiocapabilitiesParamsInfo& params, JsonData::DeviceInfo::Supportedms12audioprofilesResultData& response);
        uint32_t endpoint_get_deviceaudiocapabilities(JsonData::DeviceInfo::DeviceaudiocapabilitiesData& response) const;
        uint32_t endpoint_get_devicevideocapabilities(JsonData::DeviceInfo::DevicevideocapabilitiesData& response) const;
        uint32_t endpoint_get_deviceinfo(JsonData::DeviceInfo::DeviceinfoData& response) const;
        uint32_t endpoint_get_systeminfo(JsonData::DeviceInfo::SysteminfoData& response) const;
        uint32_t endpoint_get_addresses(Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& response) const;
        uint32_t endpoint_get_socketinfo(JsonData::DeviceInfo::SocketinfoData& response) const;
        uint32_t endpoint_get_supportedaudioports(JsonData::DeviceInfo::SupportedaudioportsData& response) const;
        uint32_t endpoint_get_supportedvideodisplays(JsonData::DeviceInfo::SupportedvideodisplaysData& response) const;
        uint32_t endpoint_get_hostedid(JsonData::DeviceInfo::HostedidData& response) const;
        uint32_t endpoint_get_firmwareversion(JsonData::DeviceInfo::FirmwareversionData& response) const;
        uint32_t endpoint_get_serialnumber(JsonData::DeviceInfo::SerialnumberData& response) const;
        uint32_t endpoint_get_modelid(JsonData::DeviceInfo::ModelidData& response) const;
        uint32_t endpoint_get_make(JsonData::DeviceInfo::MakeData& response) const;
        uint32_t endpoint_get_modelname(JsonData::DeviceInfo::ModelnameData& response) const;
        uint32_t endpoint_get_modelyear(JsonData::DeviceInfo::ModelyearData& response) const;
        uint32_t endpoint_get_friendlyname(JsonData::DeviceInfo::FriendlynameInfo& response) const;
        uint32_t endpoint_get_platformname(JsonData::DeviceInfo::FriendlynameInfo& response) const;
        uint32_t endpoint_get_devicetype(JsonData::DeviceInfo::DevicetypeData& response) const;
        uint32_t endpoint_get_distributorid(JsonData::DeviceInfo::DistributoridData& response) const;

        void SysInfo(JsonData::DeviceInfo::SysteminfoData& systemInfo) const;
        void AddressInfo(Core::JSON::ArrayType<JsonData::DeviceInfo::AddressesData>& addressInfo) const;
        void SocketPortInfo(JsonData::DeviceInfo::SocketinfoData& socketPortInfo) const;
        void AudioCapabilitiesInfo(JsonData::DeviceInfo::DeviceaudiocapabilitiesData& response) const;
        void VideoCapabilitiesInfo(JsonData::DeviceInfo::DevicevideocapabilitiesData& response) const;
        void DeviceMetaData(JsonData::DeviceInfo::DeviceinfoData& response) const;

        using VideoOutputTypes = Core::JSON::ArrayType<Core::JSON::EnumType<JsonData::DeviceInfo::VideodisplayType>>;
        using ScreenResolutionType = Core::JSON::EnumType<JsonData::DeviceInfo::Output_resolutionType>;
        using ScreenResolutionTypes = Core::JSON::ArrayType<ScreenResolutionType>;
        using CopyProtectionType = Core::JSON::EnumType<JsonData::DeviceInfo::CopyprotectionType>;
        uint32_t VideoOutputs(VideoOutputTypes& videoOutputs) const;
        uint32_t DefaultResolution(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, ScreenResolutionType& screenResolutionType) const;
        uint32_t Resolutions(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, ScreenResolutionTypes& screenResolutionTypes) const;
        uint32_t Hdcp(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, CopyProtectionType& copyProtectionType) const;

        using AudioOutputTypes = Core::JSON::ArrayType<Core::JSON::EnumType<JsonData::DeviceInfo::AudioportType>>;
        using AudioCapabilityTypes = Core::JSON::ArrayType<Core::JSON::EnumType<JsonData::DeviceInfo::AudiocapabilityType>>;
        using Ms12CapabilityTypes = Core::JSON::ArrayType<Core::JSON::EnumType<JsonData::DeviceInfo::Ms12capabilityType>>;
        using Ms12ProfileTypes = Core::JSON::ArrayType<Core::JSON::EnumType<JsonData::DeviceInfo::Ms12profileType>>;
        uint32_t AudioOutputs(AudioOutputTypes& audioOutputs) const;
        uint32_t AudioCapabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, AudioCapabilityTypes& audioCapabilityTypes) const;
        uint32_t Ms12Capabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Ms12CapabilityTypes& ms12CapabilityTypes) const;
        uint32_t Ms12Profiles(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Ms12ProfileTypes& ms12ProfileTypes) const;

        inline uint32_t HostEDID(Core::JSON::String& Hostedid) const
        {
            string edid;
            uint32_t status = _deviceVideoCapabilityInterface->HostEDID(edid);
            if (status == Core::ERROR_NONE) {
                Hostedid = edid;
            }
            return status;
        }
#endif
        void UpdateDeviceIdentifier();
        void Deactivated(RPC::IRemoteConnection* connection);

    private:
        uint8_t _skipURL;
        PluginHost::IShell* _service;
#if ENABLE_LEGACY_INTERFACE_SUPPORT
        PluginHost::ISubSystem* _subSystem;
        string _deviceId;
#endif
        Exchange::IDeviceInfo* _deviceInfo;
        Exchange::IDeviceAudioCapabilities* _deviceAudioCapabilityInterface;
        Exchange::IDeviceVideoCapabilities* _deviceVideoCapabilityInterface;
#if !defined(ENABLE_LEGACY_INTERFACE_SUPPORT) || (ENABLE_LEGACY_INTERFACE_SUPPORT == 0)
        Exchange::IAddressMetadata* _addressMetadata;
        Exchange::IDeviceMetadata* _deviceMetadata;
        Exchange::IImageMetadata* _imageMetadata;
        Exchange::ISocketMetadata* _socketMetadata;
        Exchange::ISystemMetadata* _systemMetadata;
#endif

        uint32_t _connectionId;
#if ENABLE_LEGACY_INTERFACE_SUPPORT
        mutable Core::CriticalSection _adminLock;
#endif
        Core::SinkType<Notification> _notification;
    };

} // namespace Plugin
} // namespace Thunder

#endif // DEVICEINFO_DEVICEINFO_H
