#include "DeviceInfoImplementation.h"

namespace Thunder {
namespace Plugin {
    SERVICE_REGISTRATION(DeviceInfoImplementation, 1, 0)

    using AudioCapabilitiesJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceAudioCapabilities::AudioCapability>>;
    using MS12CapabilitiesJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceAudioCapabilities::MS12Capability>>;
    using MS12ProfilesJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceAudioCapabilities::MS12Profile>>;
#if ENABLE_LEGACY_INTERFACE_SUPPORT
    using AudioCapabilityIteratorImplementation = RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator>;
    using MS12CapabilityIteratorImplementation = RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator>;
    using MS12ProfileIteratorImplementation = RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IMS12ProfileIterator>;
    using ResolutionIteratorImplementation = RPC::IteratorType<Exchange::IDeviceVideoCapabilities::IScreenResolutionIterator>;
#endif
    using ResolutionJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceVideoCapabilities::ScreenResolution>>;

    uint32_t DeviceInfoImplementation::Configure(const PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

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

#if ENABLE_LEGACY_INTERFACE_SUPPORT
    uint32_t DeviceInfoImplementation::AudioOutputs(Exchange::IDeviceAudioCapabilities::IAudioOutputIterator*& audioOutputs) const
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
    uint32_t DeviceInfoImplementation::AudioOutputs(Exchange::IDeviceAudioCapabilities::AudioOutput& audioOutputs) const
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

#if ENABLE_LEGACY_INTERFACE_SUPPORT
    uint32_t DeviceInfoImplementation::AudioCapabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator*& audioCapabilities) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.AudioCapabilities.size() > 0)) {
             audioCapabilities = Core::ServiceType<AudioCapabilityIteratorImplementation>::Create<Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator>(index->second.AudioCapabilities);
        }
        return (audioCapabilities != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);

    }
#else
    uint32_t DeviceInfoImplementation::AudioCapabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::AudioCapability& audioCapabilities) const
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

#if ENABLE_LEGACY_INTERFACE_SUPPORT
    uint32_t DeviceInfoImplementation::MS12Capabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator*& ms12Capabilities) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.MS12Capabilities.size() > 0)) {
             ms12Capabilities = Core::ServiceType<MS12CapabilityIteratorImplementation>::Create<Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator>(index->second.MS12Capabilities);
        }
        return (ms12Capabilities != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    uint32_t DeviceInfoImplementation::MS12Capabilities(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::MS12Capability& ms12Capabilities) const
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

#if ENABLE_LEGACY_INTERFACE_SUPPORT
    uint32_t DeviceInfoImplementation::MS12AudioProfiles(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::IMS12ProfileIterator*& ms12AudioProfiles) const
    {
        AudioOutputMap::const_iterator index = _audioOutputMap.find(audioOutput);
        if ((index != _audioOutputMap.end()) && (index->second.MS12Profiles.size() > 0)) {
             ms12AudioProfiles = Core::ServiceType<MS12ProfileIteratorImplementation>::Create<Exchange::IDeviceAudioCapabilities::IMS12ProfileIterator>(index->second.MS12Profiles);
        }
        return (ms12AudioProfiles != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    uint32_t DeviceInfoImplementation::MS12AudioProfiles(const Exchange::IDeviceAudioCapabilities::AudioOutput audioOutput, Exchange::IDeviceAudioCapabilities::MS12Profile& ms12Profiles) const
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

#if ENABLE_LEGACY_INTERFACE_SUPPORT
    uint32_t DeviceInfoImplementation::VideoOutputs(Exchange::IDeviceVideoCapabilities::IVideoOutputIterator*& videoOutputs) const
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
    uint32_t DeviceInfoImplementation::VideoOutputs(Exchange::IDeviceVideoCapabilities::VideoOutput& videoOutputs) const
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

    uint32_t DeviceInfoImplementation::DefaultResolution(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, Exchange::IDeviceVideoCapabilities::ScreenResolution& defaultResolution) const
    {
        defaultResolution = Exchange::IDeviceVideoCapabilities::ScreenResolution_Unknown;
        VideoOutputMap::const_iterator index = _videoOutputMap.find(videoOutput);
        if ((index != _videoOutputMap.end()) && (index->second.DefaultResolution != Exchange::IDeviceVideoCapabilities::ScreenResolution_Unknown)) {
            defaultResolution = index->second.DefaultResolution;
        }
        return (Core::ERROR_NONE);
    }

#if ENABLE_LEGACY_INTERFACE_SUPPORT
    uint32_t DeviceInfoImplementation::Resolutions(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, Exchange::IDeviceVideoCapabilities::IScreenResolutionIterator*& resolutions) const
    {
        VideoOutputMap::const_iterator index = _videoOutputMap.find(videoOutput);
        if ((index != _videoOutputMap.end()) && (index->second.Resolutions.size() > 0)) {
            resolutions = Core::ServiceType<ResolutionIteratorImplementation>::Create<Exchange::IDeviceVideoCapabilities::IScreenResolutionIterator>(index->second.Resolutions);
        }

        return (resolutions != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
#else
    uint32_t DeviceInfoImplementation::Resolutions(const Exchange::IDeviceVideoCapabilities::VideoOutput videoOutput, Exchange::IDeviceVideoCapabilities::ScreenResolution& resolutions) const
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

    uint32_t DeviceInfoImplementation::Hdcp(const VideoOutput videoOutput, CopyProtection& hdcpVersion) const
    {
        hdcpVersion = Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_UNAVAILABLE;
        VideoOutputMap::const_iterator index = _videoOutputMap.find(videoOutput);
        if ((index != _videoOutputMap.end()) && (index->second.CopyProtection != HDCP_UNAVAILABLE)) {
            hdcpVersion = index->second.CopyProtection;
        }

        return (Core::ERROR_NONE);
    }

    uint32_t DeviceInfoImplementation::HDR(bool& supportsHDR) const
    {
        supportsHDR = _supportsHdr;
        return Core::ERROR_NONE;
    }

    uint32_t DeviceInfoImplementation::Atmos(bool& supportsAtmos) const
    {
        supportsAtmos = _supportsAtmos;
        return Core::ERROR_NONE;
    }

    uint32_t DeviceInfoImplementation::CEC(bool& supportsCEC) const
    {
        supportsCEC = _supportsCEC;
        return Core::ERROR_NONE;
    }

    uint32_t DeviceInfoImplementation::HostEDID(string& edid) const
    {
        edid = _hostEdid;
        return Core::ERROR_NONE;
    }

    uint32_t DeviceInfoImplementation::Make(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_make.empty() == false) {
            value = _make;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::SerialNumber(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_serialNumber.empty() == false) {
            value = _serialNumber;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::Sku(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_sku.empty() == false) {
            value = _sku;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::ModelName(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_modelName.empty() == false) {
            value = _modelName;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::ModelYear(uint16_t& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_modelYear != 0) {
            value = _modelYear;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::FriendlyName(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_friendlyName.empty() == false) {
            value = _friendlyName;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::DeviceType(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_deviceType.empty() == false) {
            value = _deviceType;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::DistributorId(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_distributorId.empty() == false) {
            value = _distributorId;
            result = Core::ERROR_NONE;
        }
        return result;
    }

    uint32_t DeviceInfoImplementation::PlatformName(string& value) const
    {
        uint32_t result = Core::ERROR_UNAVAILABLE;
        if (_platformName.empty() == false) {
            value = _platformName;
            result = Core::ERROR_NONE;
        }
        return result;
    }
}
}
