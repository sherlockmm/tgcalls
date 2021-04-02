#include "v2/Signaling.h"

#include "third-party/json11.hpp"

#include "rtc_base/checks.h"

#include <sstream>

namespace tgcalls {
namespace signaling {

static std::string uint32ToString(uint32_t value) {
    std::ostringstream stringStream;
    stringStream << value;
    return stringStream.str();
}

static uint32_t stringToUInt32(std::string const &string) {
    std::stringstream stringStream(string);
    uint32_t value = 0;
    stringStream >> value;
    return value;
}

json11::Json::object SsrcGroup_serialize(SsrcGroup const &ssrcGroup) {
    json11::Json::object object;

    json11::Json::array ssrcs;
    for (auto ssrc : ssrcGroup.ssrcs) {
        ssrcs.push_back(json11::Json(uint32ToString(ssrc)));
    }
    object.insert(std::make_pair("semantics", json11::Json(ssrcGroup.semantics)));
    object.insert(std::make_pair("ssrcs", json11::Json(std::move(ssrcs))));

    return object;
}

absl::optional<SsrcGroup> SsrcGroup_parse(json11::Json::object const &object) {
    SsrcGroup result;

    const auto semantics = object.find("semantics");
    if (semantics == object.end() || !semantics->second.is_string()) {
        return absl::nullopt;
    }
    result.semantics = semantics->second.string_value();

    const auto ssrcs = object.find("ssrcs");
    if (ssrcs == object.end() || !ssrcs->second.is_array()) {
        return absl::nullopt;
    }
    for (const auto &ssrc : ssrcs->second.array_items()) {
        if (!ssrc.is_string()) {
            return absl::nullopt;
        }
        uint32_t parsedSsrc = stringToUInt32(ssrc.string_value());
        if (parsedSsrc == 0) {
            return absl::nullopt;
        }
        result.ssrcs.push_back(parsedSsrc);
    }

    return result;
}

json11::Json::object FeedbackType_serialize(FeedbackType const &feedbackType) {
    json11::Json::object object;

    object.insert(std::make_pair("type", json11::Json(feedbackType.type)));
    object.insert(std::make_pair("subtype", json11::Json(feedbackType.subtype)));

    return object;
}

absl::optional<FeedbackType> FeedbackType_parse(json11::Json::object const &object) {
    FeedbackType result;

    const auto type = object.find("type");
    if (type == object.end() || !type->second.is_string()) {
        return absl::nullopt;
    }
    result.type = type->second.string_value();

    const auto subtype = object.find("subtype");
    if (subtype == object.end() || !subtype->second.is_string()) {
        return absl::nullopt;
    }
    result.subtype = subtype->second.string_value();

    return result;
}

json11::Json::object RtpExtension_serialize(RtpExtension const &rtpExtension) {
    json11::Json::object object;

    object.insert(std::make_pair("id", json11::Json(rtpExtension.id)));
    object.insert(std::make_pair("uri", json11::Json(rtpExtension.uri)));

    return object;
}

absl::optional<RtpExtension> RtpExtension_parse(json11::Json::object const &object) {
    const auto id = object.find("id");
    if (id == object.end() || !id->second.is_number()) {
        return absl::nullopt;
    }

    const auto uri = object.find("uri");
    if (uri == object.end() || !uri->second.is_string()) {
        return absl::nullopt;
    }

    return RtpExtension(id->second.int_value(), uri->second.string_value());
}

json11::Json::object PayloadType_serialize(PayloadType const &payloadType) {
    json11::Json::object object;

    object.insert(std::make_pair("id", json11::Json((int)payloadType.id)));
    object.insert(std::make_pair("name", json11::Json(payloadType.name)));
    object.insert(std::make_pair("clockrate", json11::Json((int)payloadType.clockrate)));
    object.insert(std::make_pair("channels", json11::Json((int)payloadType.channels)));

    json11::Json::array feedbackTypes;
    for (const auto &feedbackType : payloadType.feedbackTypes) {
        feedbackTypes.push_back(FeedbackType_serialize(feedbackType));
    }
    object.insert(std::make_pair("feedbackTypes", json11::Json(std::move(feedbackTypes))));

    json11::Json::object parameters;
    for (auto it : payloadType.parameters) {
        parameters.insert(std::make_pair(it.first, json11::Json(it.second)));
    }
    object.insert(std::make_pair("parameters", json11::Json(std::move(parameters))));

    return object;
}

absl::optional<PayloadType> PayloadType_parse(json11::Json::object const &object) {
    PayloadType result;

    const auto id = object.find("id");
    if (id == object.end() || !id->second.is_number()) {
        return absl::nullopt;
    }
    result.id = id->second.int_value();

    const auto name = object.find("name");
    if (name == object.end() || !name->second.is_string()) {
        return absl::nullopt;
    }
    result.name = name->second.string_value();

    const auto clockrate = object.find("clockrate");
    if (clockrate == object.end() || !clockrate->second.is_number()) {
        return absl::nullopt;
    }
    result.clockrate = clockrate->second.int_value();

    const auto channels = object.find("channels");
    if (channels != object.end()) {
        if (!channels->second.is_number()) {
            return absl::nullopt;
        }
        result.channels = channels->second.int_value();
    }

    const auto feedbackTypes = object.find("feedbackTypes");
    if (feedbackTypes != object.end()) {
        if (!feedbackTypes->second.is_array()) {
            return absl::nullopt;
        }
        for (const auto &feedbackType : feedbackTypes->second.array_items()) {
            if (!feedbackType.is_object()) {
                return absl::nullopt;
            }
            if (const auto parsedFeedbackType = FeedbackType_parse(feedbackType.object_items())) {
                result.feedbackTypes.push_back(parsedFeedbackType.value());
            } else {
                return absl::nullopt;
            }
        }
    }

    const auto parameters = object.find("parameters");
    if (parameters != object.end()) {
        if (!parameters->second.is_object()) {
            return absl::nullopt;
        }
        for (const auto &item : parameters->second.object_items()) {
            if (!item.second.is_string()) {
                return absl::nullopt;
            }
            result.parameters.push_back(std::make_pair(item.first, item.second.string_value()));
        }
    }

    return result;
}

json11::Json::object MediaContent_serialize(MediaContent const &mediaContent) {
    json11::Json::object object;

    object.insert(std::make_pair("ssrc", json11::Json(uint32ToString(mediaContent.ssrc))));

    if (mediaContent.ssrcGroups.size() != 0) {
        json11::Json::array ssrcGroups;
        for (const auto &group : mediaContent.ssrcGroups) {
            ssrcGroups.push_back(SsrcGroup_serialize(group));
        }
        object.insert(std::make_pair("ssrcGroups", json11::Json(std::move(ssrcGroups))));
    }

    if (mediaContent.payloadTypes.size() != 0) {
        json11::Json::array payloadTypes;
        for (const auto &payloadType : mediaContent.payloadTypes) {
            payloadTypes.push_back(PayloadType_serialize(payloadType));
        }
        object.insert(std::make_pair("payloadTypes", json11::Json(std::move(payloadTypes))));
    }

    json11::Json::array rtpExtensions;
    for (const auto &rtpExtension : mediaContent.rtpExtensions) {
        rtpExtensions.push_back(RtpExtension_serialize(rtpExtension));
    }
    object.insert(std::make_pair("rtpExtensions", json11::Json(std::move(rtpExtensions))));

    return object;
}

absl::optional<MediaContent> MediaContent_parse(json11::Json::object const &object) {
    MediaContent result;

    const auto type = object.find("ssrc");
    if (type == object.end() || !type->second.is_string()) {
        return absl::nullopt;
    }
    result.ssrc = stringToUInt32(type->second.string_value());

    const auto ssrcGroups = object.find("ssrcGroups");
    if (ssrcGroups != object.end()) {
        if (!ssrcGroups->second.is_array()) {
            return absl::nullopt;
        }
        for (const auto &ssrcGroup : ssrcGroups->second.array_items()) {
            if (!ssrcGroup.is_object()) {
                return absl::nullopt;
            }
            if (const auto parsedSsrcGroup = SsrcGroup_parse(ssrcGroup.object_items())) {
                result.ssrcGroups.push_back(parsedSsrcGroup.value());
            } else {
                return absl::nullopt;
            }
        }
    }

    const auto payloadTypes = object.find("payloadTypes");
    if (payloadTypes != object.end()) {
        if (!payloadTypes->second.is_array()) {
            return absl::nullopt;
        }
        for (const auto &payloadType : payloadTypes->second.array_items()) {
            if (!payloadType.is_object()) {
                return absl::nullopt;
            }
            if (const auto parsedPayloadType = PayloadType_parse(payloadType.object_items())) {
                result.payloadTypes.push_back(parsedPayloadType.value());
            } else {
                return absl::nullopt;
            }
        }
    }

    const auto rtpExtensions = object.find("rtpExtensions");
    if (rtpExtensions != object.end()) {
        if (!rtpExtensions->second.is_array()) {
            return absl::nullopt;
        }
        for (const auto &rtpExtension : rtpExtensions->second.array_items()) {
            if (!rtpExtension.is_object()) {
                return absl::nullopt;
            }
            if (const auto parsedRtpExtension = RtpExtension_parse(rtpExtension.object_items())) {
                result.rtpExtensions.push_back(parsedRtpExtension.value());
            } else {
                return absl::nullopt;
            }
        }
    }

    return result;
}

std::vector<uint8_t> InitialSetupMessage_serialize(const InitialSetupMessage * const message) {
    json11::Json::object object;
    
    object.insert(std::make_pair("@type", json11::Json("InitialSetup")));
    object.insert(std::make_pair("ufrag", json11::Json(message->ufrag)));
    object.insert(std::make_pair("pwd", json11::Json(message->pwd)));

    json11::Json::array jsonFingerprints;
    for (const auto &fingerprint : message->fingerprints) {
        json11::Json::object jsonFingerprint;
        jsonFingerprint.insert(std::make_pair("hash", json11::Json(fingerprint.hash)));
        jsonFingerprint.insert(std::make_pair("setup", json11::Json(fingerprint.setup)));
        jsonFingerprint.insert(std::make_pair("fingerprint", json11::Json(fingerprint.fingerprint)));
        jsonFingerprints.emplace_back(std::move(jsonFingerprint));
    }
    object.insert(std::make_pair("fingerprints", json11::Json(std::move(jsonFingerprints))));

    if (const auto audio = message->audio) {
        object.insert(std::make_pair("audio", json11::Json(MediaContent_serialize(audio.value()))));
    }

    if (const auto video = message->video) {
        object.insert(std::make_pair("video", json11::Json(MediaContent_serialize(video.value()))));
    }

    auto json = json11::Json(std::move(object));
    std::string result = json.dump();
    return std::vector<uint8_t>(result.begin(), result.end());
}

absl::optional<InitialSetupMessage> InitialSetupMessage_parse(json11::Json::object const &object) {
    const auto ufrag = object.find("ufrag");
    if (ufrag == object.end() || !ufrag->second.is_string()) {
        return absl::nullopt;
    }
    const auto pwd = object.find("pwd");
    if (pwd == object.end() || !pwd->second.is_string()) {
        return absl::nullopt;
    }
    const auto fingerprints = object.find("fingerprints");
    if (fingerprints == object.end() || !fingerprints->second.is_array()) {
        return absl::nullopt;
    }
    std::vector<DtlsFingerprint> parsedFingerprints;
    for (const auto &fingerprintObject : fingerprints->second.array_items()) {
        if (!fingerprintObject.is_object()) {
            return absl::nullopt;
        }
        const auto hash = fingerprintObject.object_items().find("hash");
        if (hash == fingerprintObject.object_items().end() || !hash->second.is_string()) {
            return absl::nullopt;
        }
        const auto setup = fingerprintObject.object_items().find("setup");
        if (setup == fingerprintObject.object_items().end() || !setup->second.is_string()) {
            return absl::nullopt;
        }
        const auto fingerprint = fingerprintObject.object_items().find("fingerprint");
        if (fingerprint == fingerprintObject.object_items().end() || !fingerprint->second.is_string()) {
            return absl::nullopt;
        }
        
        DtlsFingerprint parsedFingerprint;
        parsedFingerprint.hash = hash->second.string_value();
        parsedFingerprint.setup = setup->second.string_value();
        parsedFingerprint.fingerprint = fingerprint->second.string_value();

        parsedFingerprints.push_back(std::move(parsedFingerprint));
    }

    InitialSetupMessage message;
    message.ufrag = ufrag->second.string_value();
    message.pwd = pwd->second.string_value();
    message.fingerprints = std::move(parsedFingerprints);

    const auto audio = object.find("audio");
    if (audio != object.end()) {
        if (!audio->second.is_object()) {
            return absl::nullopt;
        }
        if (const auto parsedAudio = MediaContent_parse(audio->second.object_items())) {
            message.audio = parsedAudio.value();
        } else {
            return absl::nullopt;
        }
    }

    const auto video = object.find("video");
    if (video != object.end()) {
        if (!video->second.is_object()) {
            return absl::nullopt;
        }
        if (const auto parsedVideo = MediaContent_parse(video->second.object_items())) {
            message.video = parsedVideo.value();
        } else {
            return absl::nullopt;
        }
    }

    return message;
}

json11::Json::object ConnectionAddress_serialize(ConnectionAddress const &connectionAddress) {
    json11::Json::object object;

    object.insert(std::make_pair("ip", json11::Json(connectionAddress.ip)));
    object.insert(std::make_pair("port", json11::Json(connectionAddress.port)));

    return object;
}

absl::optional<ConnectionAddress> ConnectionAddress_parse(json11::Json::object const &object) {
    const auto ip = object.find("ip");
    if (ip == object.end() || !ip->second.is_string()) {
        return absl::nullopt;
    }

    const auto port = object.find("port");
    if (port == object.end() || !port->second.is_number()) {
        return absl::nullopt;
    }

    ConnectionAddress address;
    address.ip = ip->second.string_value();
    address.port = port->second.int_value();
    return address;
}

std::vector<uint8_t> CandidatesMessage_serialize(const CandidatesMessage * const message) {
    json11::Json::array candidates;
    for (const auto &candidate : message->iceCandidates) {
        json11::Json::object candidateObject;

        candidateObject.insert(std::make_pair("component", json11::Json(candidate.component)));
        candidateObject.insert(std::make_pair("protocol", json11::Json(candidate.protocol)));

        candidateObject.insert(std::make_pair("address", json11::Json(ConnectionAddress_serialize(candidate.connectionAddress))));

        if (const auto relAddress = candidate.relAddress) {
            candidateObject.insert(std::make_pair("relAddress", json11::Json(ConnectionAddress_serialize(relAddress.value()))));
        }

        if (candidate.tcpType.size() != 0) {
            candidateObject.insert(std::make_pair("tcpType", json11::Json(candidate.tcpType)));
        }

        candidateObject.insert(std::make_pair("priority", json11::Json(uint32ToString(candidate.priority))));

        candidateObject.insert(std::make_pair("username", json11::Json(candidate.username)));
        candidateObject.insert(std::make_pair("password", json11::Json(candidate.password)));

        candidateObject.insert(std::make_pair("type", json11::Json(candidate.type)));

        candidateObject.insert(std::make_pair("generation", json11::Json(uint32ToString(candidate.generation))));
        candidateObject.insert(std::make_pair("foundation", json11::Json(candidate.foundation)));

        candidateObject.insert(std::make_pair("networkId", json11::Json((int)candidate.networkId)));
        candidateObject.insert(std::make_pair("networkCost", json11::Json((int)candidate.networkCost)));

        candidates.emplace_back(std::move(candidateObject));
    }

    json11::Json::object object;

    object.insert(std::make_pair("@type", json11::Json("Candidates")));
    object.insert(std::make_pair("candidates", json11::Json(std::move(candidates))));

    auto json = json11::Json(std::move(object));
    std::string result = json.dump();
    return std::vector<uint8_t>(result.begin(), result.end());
}

absl::optional<CandidatesMessage> CandidatesMessage_parse(json11::Json::object const &object) {
    const auto candidates = object.find("candidates");
    if (candidates == object.end() || !candidates->second.is_array()) {
        return absl::nullopt;
    }

    std::vector<IceCandidate> parsedCandidates;
    for (const auto &candidateObject : candidates->second.array_items()) {
        if (!candidateObject.is_object()) {
            return absl::nullopt;
        }

        IceCandidate candidate;

        const auto component = candidateObject.object_items().find("component");
        if (component == candidateObject.object_items().end() || !component->second.is_number()) {
            return absl::nullopt;
        }
        candidate.component = component->second.int_value();

        const auto protocol = candidateObject.object_items().find("protocol");
        if (protocol == candidateObject.object_items().end() || !protocol->second.is_string()) {
            return absl::nullopt;
        }
        candidate.protocol = protocol->second.string_value();

        const auto address = candidateObject.object_items().find("address");
        if (address == candidateObject.object_items().end() || !address->second.is_object()) {
            return absl::nullopt;
        }
        if (const auto parsedAddress = ConnectionAddress_parse(address->second.object_items())) {
            candidate.connectionAddress = parsedAddress.value();
        } else {
            return absl::nullopt;
        }

        const auto relAddress = candidateObject.object_items().find("relAddress");
        if (relAddress != candidateObject.object_items().end()) {
            if (!relAddress->second.is_object()) {
                return absl::nullopt;
            }
            if (const auto parsedRelAddress = ConnectionAddress_parse(relAddress->second.object_items())) {
                candidate.relAddress = parsedRelAddress.value();
            } else {
                return absl::nullopt;
            }
        }

        const auto tcpType = candidateObject.object_items().find("tcpType");
        if (tcpType != candidateObject.object_items().end()) {
            if (!tcpType->second.is_string()) {
                return absl::nullopt;
            }
            candidate.tcpType = tcpType->second.string_value();
        }

        const auto priority = candidateObject.object_items().find("priority");
        if (priority == candidateObject.object_items().end() || !priority->second.is_string()) {
            return absl::nullopt;
        }
        candidate.priority = stringToUInt32(priority->second.string_value());

        const auto username = candidateObject.object_items().find("username");
        if (username == candidateObject.object_items().end() || !username->second.is_string()) {
            return absl::nullopt;
        }
        candidate.username = username->second.string_value();

        const auto password = candidateObject.object_items().find("password");
        if (password == candidateObject.object_items().end() || !password->second.is_string()) {
            return absl::nullopt;
        }
        candidate.password = password->second.string_value();

        const auto type = candidateObject.object_items().find("type");
        if (type == candidateObject.object_items().end() || !type->second.is_string()) {
            return absl::nullopt;
        }
        candidate.type = type->second.string_value();

        const auto generation = candidateObject.object_items().find("generation");
        if (generation == candidateObject.object_items().end() || !generation->second.is_string()) {
            return absl::nullopt;
        }
        candidate.generation = stringToUInt32(generation->second.string_value());

        const auto foundation = candidateObject.object_items().find("foundation");
        if (foundation == candidateObject.object_items().end() || !foundation->second.is_string()) {
            return absl::nullopt;
        }
        candidate.foundation = foundation->second.string_value();

        const auto networkId = candidateObject.object_items().find("networkId");
        if (networkId == candidateObject.object_items().end() || !networkId->second.is_number()) {
            return absl::nullopt;
        }
        candidate.networkId = networkId->second.int_value();

        const auto networkCost = candidateObject.object_items().find("networkCost");
        if (networkCost == candidateObject.object_items().end() || !networkCost->second.is_number()) {
            return absl::nullopt;
        }
        candidate.networkCost = networkCost->second.int_value();

        parsedCandidates.push_back(std::move(candidate));
    }

    CandidatesMessage message;
    message.iceCandidates = std::move(parsedCandidates);

    return message;
}

std::vector<uint8_t> MediaStateMessage_serialize(const MediaStateMessage * const message) {
    json11::Json::object object;

    object.insert(std::make_pair("@type", json11::Json("MediaState")));
    object.insert(std::make_pair("muted", json11::Json(message->isMuted)));
    object.insert(std::make_pair("lowBattery", json11::Json(message->isBatteryLow)));

    std::string videoStateValue;
    switch (message->videoState) {
        case MediaStateMessage::VideoState::Inactive: {
            videoStateValue = "inactive";
            break;
        }
        case MediaStateMessage::VideoState::Suspended: {
            videoStateValue = "suspended";
            break;
        }
        case MediaStateMessage::VideoState::Active: {
            videoStateValue = "active";
            break;
        }
        default: {
            RTC_FATAL() << "Unknown videoState";
            break;
        }
    }
    object.insert(std::make_pair("videoState", json11::Json(videoStateValue)));

    auto json = json11::Json(std::move(object));
    std::string result = json.dump();
    return std::vector<uint8_t>(result.begin(), result.end());
}

absl::optional<MediaStateMessage> MediaStateMessage_parse(json11::Json::object const &object) {
    MediaStateMessage message;

    const auto muted = object.find("muted");
    if (muted != object.end()) {
        if (!muted->second.is_bool()) {
            return absl::nullopt;
        }
        message.isMuted = muted->second.bool_value();
    }

    const auto lowBattery = object.find("lowBattery");
    if (lowBattery != object.end()) {
        if (!lowBattery->second.is_bool()) {
            return absl::nullopt;
        }
        message.isBatteryLow = lowBattery->second.bool_value();
    }

    const auto videoState = object.find("videoState");
    if (videoState != object.end()) {
        if (!videoState->second.is_string()) {
            return absl::nullopt;
        }
        if (videoState->second.string_value() == "inactive") {
            message.videoState = MediaStateMessage::VideoState::Inactive;
        } else if (videoState->second.string_value() == "suspended") {
            message.videoState = MediaStateMessage::VideoState::Suspended;
        } else if (videoState->second.string_value() == "active") {
            message.videoState = MediaStateMessage::VideoState::Active;
        }
    } else {
        message.videoState = MediaStateMessage::VideoState::Inactive;
    }

    return message;
}

std::vector<uint8_t> Message::serialize() const {
    if (const auto initialSetup = absl::get_if<InitialSetupMessage>(&data)) {
        return InitialSetupMessage_serialize(initialSetup);
    } else if (const auto candidates = absl::get_if<CandidatesMessage>(&data)) {
        return CandidatesMessage_serialize(candidates);
    } else if (const auto mediaState = absl::get_if<MediaStateMessage>(&data)) {
        return MediaStateMessage_serialize(mediaState);
    } else {
        return {};
    }
}

absl::optional<Message> Message::parse(const std::vector<uint8_t> &data) {
    std::string parsingError;
    auto json = json11::Json::parse(std::string(data.begin(), data.end()), parsingError);
    if (json.type() != json11::Json::OBJECT) {
        return absl::nullopt;
    }

    auto type = json.object_items().find("@type");
    if (type == json.object_items().end()) {
        return absl::nullopt;
    }
    if (!type->second.is_string()) {
        return absl::nullopt;
    }
    if (type->second.string_value() == "InitialSetup") {
        auto parsed = InitialSetupMessage_parse(json.object_items());
        if (!parsed) {
            return absl::nullopt;
        }
        Message message;
        message.data = std::move(parsed.value());
        return message;
    } else if (type->second.string_value() == "Candidates") {
        auto parsed = CandidatesMessage_parse(json.object_items());
        if (!parsed) {
            return absl::nullopt;
        }
        Message message;
        message.data = std::move(parsed.value());
        return message;
    } else if (type->second.string_value() == "MediaState") {
        auto parsed = MediaStateMessage_parse(json.object_items());
        if (!parsed) {
            return absl::nullopt;
        }
        Message message;
        message.data = std::move(parsed.value());
        return message;
    } else {
        return absl::nullopt;
    }
}

} // namespace signaling

} // namespace tgcalls
