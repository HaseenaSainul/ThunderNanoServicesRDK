#include "DeviceInfoImplementation.h"

namespace Thunder {
namespace Plugin {
    SERVICE_REGISTRATION(DeviceInfoImplementation, 1, 0)

    using AudioCapabilitiesJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceAudioCapabilities::AudioCapability>>;
    using MS12CapabilitiesJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceAudioCapabilities::MS12Capability>>;
    using MS12ProfilesJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceAudioCapabilities::MS12Profile>>;
#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    using AudioCapabilityIteratorImplementation = RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator>;
    using MS12CapabilityIteratorImplementation = RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator>;
    using MS12ProfileIteratorImplementation = RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IMS12ProfileIterator>;
    using ResolutionIteratorImplementation = RPC::IteratorType<Exchange::IDeviceVideoCapabilities::IScreenResolutionIterator>;
#endif
    using ResolutionJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceVideoCapabilities::ScreenResolution>>;

    Core::hresult DeviceInfoImplementation::Configure(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

        _service = service;
        _service->AddRef();

        _config.FromString(service->ConfigLine());

        _supportsHdr = _config.Hdr.Value();
        _supportsAtmos = _config.Atmos.Value();
        _supportsCEC = _config.Cec.Value();
        _hostEdid = _config.Edid.Value();
        _make = _config.Make.Value();
        _deviceType = _config.DeviceType.Value();
        _distributorId = _config.DistributorId.Value();
        _modelName = _config.ModelName.Value();
        _modelYear = _config.ModelYear.Value();
        _friendlyName = _config.FriendlyName.Value();
        _platformName = _config.PlatformName.Value();
        _serialNumber = _config.SerialNumber.Value();
        _sku = _config.Sku.Value();

        Core::JSON::ArrayType<Config::AudioOutput>::Iterator audioOutputIterator(_config.AudioOutputs.Elements());
        while (audioOutputIterator.Next()) {
            AudioOutputCapability audioOutputCapability;

            AudioCapabilitiesJsonArray::Iterator audioCapabilitiesIterator(audioOutputIterator.Current().AudioCapabilities.Elements());
            while (audioCapabilitiesIterator.Next()) {
                audioOutputCapability.AudioCapabilities.push_back(audioCapabilitiesIterator.Current().Value());
            }
            MS12CapabilitiesJsonArray::Iterator ms12CapabilitiesIterator(audioOutputIterator.Current().MS12Capabilities.Elements());
            while (ms12CapabilitiesIterator.Next()) {
                audioOutputCapability.MS12Capabilities.push_back(ms12CapabilitiesIterator.Current().Value());
            }
            MS12ProfilesJsonArray::Iterator ms12ProfilesIterator(audioOutputIterator.Current().MS12Profiles.Elements());
            while (ms12ProfilesIterator.Next()) {
                audioOutputCapability.MS12Profiles.push_back(ms12ProfilesIterator.Current().Value());
            }
            _audioOutputMap.insert(std::pair<Exchange::IDeviceAudioCapabilities::AudioOutput, AudioOutputCapability>
                                   (audioOutputIterator.Current().Name.Value(), audioOutputCapability));
        }

        Core::JSON::ArrayType<Config::VideoOutput>::Iterator videoOutputIterator(_config.VideoOutputs.Elements());
        while (videoOutputIterator.Next()) {
            VideoOutputCapability videoOutputCapability;
            videoOutputCapability.CopyProtection = (videoOutputIterator.Current().CopyProtection.Value());
            videoOutputCapability.DefaultResolution = (videoOutputIterator.Current().DefaultResolution.Value());

            ResolutionJsonArray::Iterator resolutionIterator(videoOutputIterator.Current().Resolutions.Elements());
            while (resolutionIterator.Next()) {
                videoOutputCapability.Resolutions.push_back(resolutionIterator.Current().Value());
            }
            _videoOutputMap.insert(std::pair<Exchange::IDeviceVideoCapabilities::VideoOutput, VideoOutputCapability>
                                   (videoOutputIterator.Current().Name.Value(), videoOutputCapability));
        }

        return (Core::ERROR_NONE);
    }

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    Core::hresult DeviceInfoImplementation::AudioOutputs(Exchange::IDeviceAudioCapabilities::IAudioOutputIterator*& audioOutputs) const
    {
        AudioOutputList audioOutputList;

        std::transform(_audioOutputMap.begin(), _audioOutputMap.end(), std::front_inserter(audioOutputList),
        [](decltype(_audioOutputMap)::value_type const &pair) {
            return pair.first;
        });

        audioOutputs = Core::ServiceType<AudioOutputIteratorImplementation>::Create<Exchange::IDeviceAudioCapabilities::IAudioOutputIterator>(audioOutputList);
        return (audioOutputs != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    Core::hresult DeviceInfoImplementation::AudioOutputs(Exchange::IDeviceAudioCapabilities::AudioOutput& audioOutputs) const
    {
        if (_audioOutputMap.size()) {

            using T = std::underlying_type<Exchange::IDeviceAudioCapabilities::AudioOutput>::type;
            T intOutputs = 0;

            for (auto output : _audioOutputMap) {
                intOutputs |= output.first;
            }

            audioOutputs = static_cast<Exchange::IDeviceAudioCapabilities::AudioOutput>(intOutputs);
        }

        return (audioOutputs ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#endif

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    Core::hresult DeviceInfoImplementation::AudioCapabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator*& audioCapabilities) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.AudioCapabilities.size() > 0)) {
             audioCapabilities = Core::ServiceType<AudioCapabilityIteratorImplementation>::Create<Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator>(index->second.AudioCapabilities);
        }
        return (audioCapabilities != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);

    }
#else
    Core::hresult DeviceInfoImplementation::AudioCapabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::AudioCapability& audioCapabilities) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.AudioCapabilities.size() > 0)) {
            using T = std::underlying_type<Exchange::IDeviceAudioCapabilities::AudioCapability>::type;
            T intCapabilities = 0;

            for (auto capability : index->second.AudioCapabilities) {
                intCapabilities |= capability;
            }

            audioCapabilities = static_cast<Exchange::IDeviceAudioCapabilities::AudioCapability>(intCapabilities);
        }

        return (audioCapabilities ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#endif

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    Core::hresult DeviceInfoImplementation::MS12Capabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator*& ms12Capabilities) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.MS12Capabilities.size() > 0)) {
             ms12Capabilities = Core::ServiceType<MS12CapabilityIteratorImplementation>::Create<Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator>(index->second.MS12Capabilities);
        }
        return (ms12Capabilities != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    Core::hresult DeviceInfoImplementation::MS12Capabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::MS12Capability& ms12Capabilities) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.MS12Capabilities.size() > 0)) {
            using T = std::underlying_type<Exchange::IDeviceAudioCapabilities::MS12Capability>::type;
            T intCapabilities = 0;

            for (auto capability : index->second.MS12Capabilities) {
                intCapabilities |= capability;
            }

            ms12Capabilities = static_cast<Exchange::IDeviceAudioCapabilities::MS12Capability>(intCapabilities);
        }

        return (ms12Capabilities ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#endif

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    Core::hresult DeviceInfoImplementation::MS12AudioProfiles(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::IMS12ProfileIterator*& ms12AudioProfiles) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.MS12Profiles.size() > 0)) {
             ms12AudioProfiles = Core::ServiceType<MS12ProfileIteratorImplementation>::Create<Exchange::IDeviceAudioCapabilities::IMS12ProfileIterator>(index->second.MS12Profiles);
        }
        return (ms12AudioProfiles != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    Core::hresult DeviceInfoImplementation::MS12AudioProfiles(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::MS12Profile& ms12Profiles) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.MS12Profiles.size() > 0)) {
            using T = std::underlying_type<Exchange::IDeviceAudioCapabilities::MS12Profile>::type;
            T intProfiles = 0;

            for (auto profile : index->second.MS12Profiles) {
                intProfiles |= profile;
            }

        }

        return (ms12Profiles ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#endif

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    Core::hresult DeviceInfoImplementation::VideoOutputs(Exchange::IDeviceVideoCapabilities::IVideoOutputIterator*& videoOutputs) const
    {
        VideoOutputList videoOutputList;

        std::transform(_videoOutputMap.begin(), _videoOutputMap.end(), std::back_inserter(videoOutputList),
        [](decltype(_videoOutputMap)::value_type const &pair) {
            return pair.first;
        });

        videoOutputs = Core::ServiceType<VideoOutputIteratorImplementation>::Create<Exchange::IDeviceVideoCapabilities::IVideoOutputIterator>(videoOutputList);

        return (videoOutputs != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    Core::hresult DeviceInfoImplementation::VideoOutputs(Exchange::IDeviceVideoCapabilities::VideoOutput& videoOutputs) const
    {
        if (_videoOutputMap.size()) {

            using T = std::underlying_type<Exchange::IDeviceVideoCapabilities::VideoOutput>::type;
            T intOutputs = 0;

            for (auto output : _videoOutputMap) {
                intOutputs |= output.first;
            }

            videoOutputs = static_cast<Exchange::IDeviceVideoCapabilities::VideoOutput>(intOutputs);
        }

        return (videoOutputs ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#endif

    Core::hresult DeviceInfoImplementation::DefaultResolution(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, Exchange::IDeviceVideoCapabilities::ScreenResolution& defaultResolution) const
    {
        defaultResolution = Exchange::IDeviceVideoCapabilities::ScreenResolution_Unknown;
        VideoOutputMap::const_iterator index = _videoOutputMap.find(videoOutput);
        if ((index != _videoOutputMap.end()) && (index->second.DefaultResolution != Exchange::IDeviceVideoCapabilities::ScreenResolution_Unknown)) {
            defaultResolution = index->second.DefaultResolution;
        }
        return (Core::ERROR_NONE);
    }

#if defined(ENABLE_LEGACY_INTERFACE_SUPPORT)
    Core::hresult DeviceInfoImplementation::Resolutions(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, Exchange::IDeviceVideoCapabilities::IScreenResolutionIterator*& resolutions) const
    {
        VideoOutputMap::const_iterator index = _videoOutputMap.find(videoOutput);
        if ((index != _videoOutputMap.end()) && (index->second.Resolutions.size() > 0)) {
            resolutions = Core::ServiceType<ResolutionIteratorImplementation>::Create<Exchange::IDeviceVideoCapabilities::IScreenResolutionIterator>(index->second.Resolutions);
        }

        return (resolutions != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    Core::hresult DeviceInfoImplementation::Resolutions(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, Exchange::IDeviceVideoCapabilities::ScreenResolution& resolutions) const
    {
        VideoOutputMap::const_iterator index = _videoOutputMap.find(videoOutput);
        if ((index != _videoOutputMap.end()) && (index->second.Resolutions.size() > 0)) {
            using T = std::underlying_type<Exchange::IDeviceVideoCapabilities::ScreenResolution>::type;
            T intResolutions = 0;

            for (auto resolution : index->second.Resolutions) {
                intResolutions |= resolution;
            }

            resolutions = static_cast<Exchange::IDeviceVideoCapabilities::ScreenResolution>(intResolutions);
        }

        return (resolutions ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#endif

    Core::hresult DeviceInfoImplementation::Hdcp(const VideoOutput videoOutput, CopyProtection& hdcpVersion) const
    {
        hdcpVersion = Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_UNAVAILABLE;
        VideoOutputMap::const_iterator index = _videoOutputMap.find(videoOutput);
        if ((index != _videoOutputMap.end()) && (index->second.CopyProtection != HDCP_UNAVAILABLE)) {
            hdcpVersion = index->second.CopyProtection;
        }

        return (Core::ERROR_NONE);
    }

    Core::hresult DeviceInfoImplementation::HDR(bool& supportsHDR) const
    {
        supportsHDR = _supportsHdr;
        return Core::ERROR_NONE;
    }

    Core::hresult DeviceInfoImplementation::Atmos(bool& supportsAtmos) const
    {
        supportsAtmos = _supportsAtmos;
        return Core::ERROR_NONE;
    }

    Core::hresult DeviceInfoImplementation::CEC(bool& supportsCEC) const
    {
        supportsCEC = _supportsCEC;
        return Core::ERROR_NONE;
    }

    Core::hresult DeviceInfoImplementation::HostEDID(string& edid) const
    {
        edid = _hostEdid;
        return Core::ERROR_NONE;
    }

    Core::hresult DeviceInfoImplementation::Make(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_make.empty() == false) {
            value = _make;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::SerialNumber(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_serialNumber.empty() == false) {
            value = _serialNumber;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::Sku(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_sku.empty() == false) {
            value = _sku;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::ModelName(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_modelName.empty() == false) {
            value = _modelName;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::ModelYear(uint16_t& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_modelYear != 0) {
            value = _modelYear;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::FriendlyName(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_friendlyName.empty() == false) {
            value = _friendlyName;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::DeviceType(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_deviceType.empty() == false) {
            value = _deviceType;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::DistributorId(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_distributorId.empty() == false) {
            value = _distributorId;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::PlatformName(string& value) const
    {
        Core::hresult result = Core::ERROR_UNAVAILABLE;
        if (_platformName.empty() == false) {
            value = _platformName;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    Core::hresult DeviceInfoImplementation::DeviceAudioCapabilities(IAudioOutputCapsIterator*& audioOutputCaps) const
    {
        Exchange::IDeviceAudioCapabilities::AudioOutput audioOutputs;
        if ((AudioOutputs(audioOutputs) == Core::ERROR_NONE) &&
            ( audioOutputs != 0)) {
            uint8_t bit = 0x1;
            uint8_t value = audioOutputs;
            std::list<Exchange::IDeviceAudioCapabilities::AudioOutputCaps> audioOutputCapsList;
            while (value != 0) {
                if ((bit & value) != 0) {

                    Exchange::IDeviceAudioCapabilities::AudioOutputCaps audioOutputCap;
                    Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput = static_cast<Exchange::IDeviceAudioCapabilities::AudioOutput>(bit);
                    audioOutputCap.audioOutput = audioOutput;

                    Exchange::IDeviceAudioCapabilities::AudioCapability audioCapabilities = Exchange::IDeviceAudioCapabilities::AUDIOCAPABILITY_NONE;
                    if ((AudioCapabilities(audioOutput, audioCapabilities) == Core::ERROR_NONE) &&
                        (audioCapabilities != 0)) {
                        audioOutputCap.audioCapabilities = audioCapabilities;
                    }

                    Exchange::IDeviceAudioCapabilities::MS12Capability ms12Capabilities = Exchange::IDeviceAudioCapabilities::MS12CAPABILITY_NONE;
                    if ((MS12Capabilities(audioOutput, ms12Capabilities) != Core::ERROR_NONE) &&
                        (ms12Capabilities != 0)) {
                        audioOutputCap.ms12Capabilities = ms12Capabilities;
                    }

                    Exchange::IDeviceAudioCapabilities::MS12Profile ms12Profiles = Exchange::IDeviceAudioCapabilities::MS12PROFILE_NONE;
                    if ((MS12AudioProfiles(audioOutput, ms12Profiles) != Core::ERROR_NONE) &&
                        (ms12Profiles != 0)) {
                        audioOutputCap.ms12Profiles = ms12Profiles;
                    }

                    value &= ~bit;
                    audioOutputCapsList.push_back(audioOutputCap);
                }
                bit = (bit << 1);
            }

            if (audioOutputCapsList.empty() == false) {
                using Iterator = Exchange::IDeviceAudioCapabilities::IAudioOutputCapsIterator;
                audioOutputCaps = Core::ServiceType<RPC::IteratorType<Iterator>>::Create<Iterator>(audioOutputCapsList);
            }
        }

        return (audioOutputCaps != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    Core::hresult DeviceInfoImplementation::DeviceVideoCapabilities(string& edid, bool& hdr, bool& atmos, bool& cec, IVideoOutputCapsIterator*& videoOutputCaps) const
    {
        HDR(hdr);
        Atmos(atmos);
        CEC(cec);
        HostEDID(edid);

        Exchange::IDeviceVideoCapabilities::VideoOutput videoOutputs;
        if ((VideoOutputs(videoOutputs) == Core::ERROR_NONE) &&
            ( videoOutputs != 0)) {
            uint8_t bit = 0x1;
            uint8_t value = videoOutputs;
            std::list<Exchange::IDeviceVideoCapabilities::VideoOutputCaps> videoOutputCapsList;
            while (value != 0) {
                if ((bit & value) != 0) {
                    Exchange::IDeviceVideoCapabilities::VideoOutputCaps videoOutputCap;
                    Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput = static_cast<Exchange::IDeviceVideoCapabilities::VideoOutput>(bit);
                    videoOutputCap.videoOutput = videoOutput;

                    Exchange::IDeviceVideoCapabilities::CopyProtection hdcp = Exchange::IDeviceVideoCapabilities::HDCP_UNAVAILABLE;
                    if ((Hdcp(videoOutput, hdcp) == Core::ERROR_NONE) &&
                        (hdcp != 0)) {
                        videoOutputCap.hdcp = hdcp;
                    }
                    Exchange::IDeviceVideoCapabilities::ScreenResolution defaultResolution = Exchange::IDeviceVideoCapabilities::ScreenResolution_Unknown;
                    if ((DefaultResolution(videoOutput, defaultResolution) == Core::ERROR_NONE) &&
                        (defaultResolution != 0)) {
                        videoOutputCap.defaultResolution = defaultResolution;
                    }
                    Exchange::IDeviceVideoCapabilities::ScreenResolution resolutions = Exchange::IDeviceVideoCapabilities::ScreenResolution_Unknown;
                    if ((Resolutions(videoOutput, resolutions) == Core::ERROR_NONE) &&
                        (resolutions != 0)) {
                        videoOutputCap.outputResolutions = resolutions;
                    }
                    value &= ~bit;
                    videoOutputCapsList.push_back(videoOutputCap);
                }
                bit = (bit << 1);
            }
            if (videoOutputCapsList.empty() == false) {
                using Iterator = Exchange::IDeviceVideoCapabilities::IVideoOutputCapsIterator;
                videoOutputCaps = Core::ServiceType<RPC::IteratorType<Iterator>>::Create<Iterator>(videoOutputCapsList);
            }
        }
        return (videoOutputCaps != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    Core::hresult DeviceInfoImplementation::AddressInfo(IAddressIterator*& ip) const
    {
        // Get the point of entry on Thunder..
        Core::AdapterIterator interfaces;
        std::list<Exchange::IAddressMetadata::Address> addresses;

        while (interfaces.Next() == true) {
            Exchange::IAddressMetadata::Address address;
            address.name = interfaces.Name();
            address.mac = interfaces.MACAddress(':');

            // get an interface with a public IP address, then we will have a proper MAC address..
            Core::IPV4AddressIterator selectedNode(interfaces.IPV4Addresses());
            Core::JSON::ArrayType<Core::JSON::String> ip;

            while (selectedNode.Next() == true) {
                ip.Add() = selectedNode.Address().HostAddress();
//                Core::JSON::String nodeName;
//                nodeName = selectedNode.Address().HostAddress();

//                element.Ip.Add() = nodeName);
            }
            ip.ToString(address.ip);

            addresses.push_back(address);
        }

        if (addresses.empty() == false) {
            using Iterator = Exchange::IAddressMetadata::IAddressIterator;
            ip = Core::ServiceType<RPC::IteratorType<Iterator>>::Create<Iterator>(addresses);
        }

        return (ip != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);

    }

    Core::hresult DeviceInfoImplementation::DeviceData(Device& device) const
    {
        string value;
        if (DeviceType(value) == Core::ERROR_NONE) {
            device.deviceType = value;
        }
        if (DistributorId(value) == Core::ERROR_NONE) {
            device.distributorId = value;
        }
        if (FriendlyName(value) == Core::ERROR_NONE) {
            device.friendlyName = value;
        }
        if (Make(value) == Core::ERROR_NONE) {
            device.make = value;
        }
        if (ModelName(value) == Core::ERROR_NONE) {
            device.modelName = value;
        }
        uint16_t year = 0;
        if (ModelYear(year) == Core::ERROR_NONE) {
            device.modelYear = year;
        }
        if (PlatformName(value) == Core::ERROR_NONE) {
            device.platformName = year;
        }
        if (SerialNumber(value) == Core::ERROR_NONE) {
            device.serialNumber = value;
        }
        if (Sku(value) == Core::ERROR_NONE) {
            device.sku = value;
        }

        return Core::ERROR_NONE;
    }

    Core::hresult DeviceInfoImplementation::FirmwareInfo(Firmware& firmware VARIABLE_IS_NOT_USED) const
    {
        return Core::ERROR_UNAVAILABLE;
    }

    string DeviceInfoImplementation::DeviceIdentifier()
    {
        if (_deviceId.empty() == true) {

            PluginHost::ISubSystem* subSystem = _service->SubSystems();
            if (subSystem != nullptr) {
                if (subSystem->IsActive(PluginHost::ISubSystem::IDENTIFIER) == true) {

                    const PluginHost::ISubSystem::IIdentifier* identifier(subSystem->Get<PluginHost::ISubSystem::IIdentifier>());

                    if (identifier != nullptr) {
                        uint8_t buffer[64];

                        if ((buffer[0] = identifier->Identifier(sizeof(buffer) - 1, &(buffer[1]))) != 0) {
                            _adminLock.Lock();
                            _deviceId = Core::SystemInfo::Instance().Id(buffer, ~0);
                            _adminLock.Unlock();
                        }
                        identifier->Release();
                    }
                }
                subSystem->Release();
            }
        }
        return _deviceId;
    }

    Core::hresult DeviceInfoImplementation::SystemInfo(System& system) const
    {
        Core::SystemInfo& singleton(Core::SystemInfo::Instance());

        system.time = Core::Time::Now().ToRFC1123(true);
        system.uptime = singleton.GetUpTime();
        system.freeRAM = singleton.GetFreeRam();
        system.totalRAM = singleton.GetTotalRam();
        system.deviceName = singleton.GetHostName();
        system.cpuLoad = Core::NumberType<uint32_t>(static_cast<uint32_t>(singleton.GetCpuLoad())).Text();
        system.serialNumber = const_cast<DeviceInfoImplementation*>(this)->DeviceIdentifier();

        PluginHost::ISubSystem* subSystem = _service->SubSystems();
        if (subSystem == nullptr) {
            system.version = subSystem->Version() + _T("#") + subSystem->BuildTreeHash();
            subSystem->Release();
        }

        return Core::ERROR_NONE;
    }

    Core::hresult DeviceInfoImplementation::SocketInfo(Socket& socket) const
    {
        socket.runs = Core::ResourceMonitor::Instance().Runs();
        return Core::ERROR_NONE;
    }

}
}
