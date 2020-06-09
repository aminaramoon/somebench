// Copyright Aeva 2018

#pragma once

#include <atomic>
#include <algorithm>
#include <limits>
#include <vector>
#include <cstdint>
#include <cstring>

#include <vsomeip/vsomeip.hpp>

#include "msg_header.hpp"

/// class SegmentedMessage
/// \brief this class implements the reassemblying behavior of aeva messages fragmented
/// using MessageFragmenter.
///
class SegmentedMessage
{
public:
    enum class PacketStatus : std::uint8_t
    {
        WAIT = 0,
        NOTIFY = 1
    };

    PacketStatus Append(
        const std::shared_ptr<vsomeip::message> &msg)
    {
        if (msg->get_payload()->get_length() < MultipartMessageHeaderSize)
        {
            return PacketStatus::WAIT;
        }

        MultipartMessageHeader incoming_header = GetHeader(msg);

        auto action = PacketStatus::WAIT;
        if (incoming_header.session_id_ != ignore_session_id_)
        {
            if (current_header_.IsNextFragment(incoming_header))
            {
                Queue(msg, incoming_header.IsBeginningOfSegment());
                action = incoming_header.IsEndOfMessage() ? PacketStatus::NOTIFY : PacketStatus::WAIT;
            }
            else if (current_header_.IsBeginningOfNewSession(incoming_header))
            {
                Drop();
                Queue(msg, true);
                action = incoming_header.IsEndOfMessage() ? PacketStatus::NOTIFY : PacketStatus::WAIT;
            }
            else
            {
                Drop();
                ignore_session_id_ = incoming_header.session_id_;
            }
        }

        current_header_ = incoming_header;

        return action;
    }

    void Reset()
    {
        segments_payloads_.clear();
        current_header_ = 0x0;
        ignore_session_id_ = std::numeric_limits<std::uint16_t>::signaling_NaN();
    }

    std::size_t Size() const noexcept
    {
        std::size_t size = 0;
        for (const auto &seg_payloads : segments_payloads_)
        {
            for (const auto &payload : seg_payloads)
            {
                size += payload->get_length() - MultipartMessageHeaderSize;
            }
        }
        return size;
    }

    std::shared_ptr<vsomeip::payload> Data() const noexcept
    {
        auto segments = FlattenSegments();
        return segments.front();
    }

    MultipartMessageHeader GetHeader(
        const std::shared_ptr<vsomeip::message> &msg) const noexcept
    {
        std::uint64_t raw{0};
        std::memcpy(&raw, msg->get_payload()->get_data(), MultipartMessageHeaderSize);
        return MultipartMessageHeader{raw};
    }

    void Drop() { segments_payloads_.clear(); }

    void Queue(const std::shared_ptr<vsomeip::message> &msg, const bool new_segment)
    {
        if (new_segment)
        {
            segments_payloads_.emplace_back();
        }
        segments_payloads_.back().push_back(msg->get_payload());
    }

    std::vector<std::shared_ptr<vsomeip::payload>> FlattenSegments() const noexcept
    {
        std::vector<std::shared_ptr<vsomeip::payload>> segments;
        segments.reserve(segments_payloads_.size());

        for (const auto &payloads : segments_payloads_)
        {
            const bool is_framented = payloads.size() > 1;
            if (is_framented)
            {
                std::size_t segment_size = MultipartMessageHeaderSize;
                std::for_each(payloads.cbegin(), payloads.cend(),
                              [&](auto &p) { segment_size += p->get_length() - MultipartMessageHeaderSize; });
                auto flat_payload = vsomeip::runtime::get()->create_payload();
                std::vector<vsomeip::byte_t> payload_bytes;
                payload_bytes.resize(segment_size);
                vsomeip::byte_t *destination = payload_bytes.data();
                const auto &front = payloads.front();
                destination = std::copy_n(front->get_data(), front->get_length(), destination);
                for (auto i = 1U; i != payloads.size(); ++i)
                {
                    const auto &fragment = payloads[i];
                    const std::size_t len = fragment->get_length() - MultipartMessageHeaderSize;
                    destination =
                        std::copy_n(fragment->get_data() + MultipartMessageHeaderSize, len, destination);
                }
                flat_payload->set_data(std::move(payload_bytes));
                segments.emplace_back(std::move(flat_payload));
            }
            else
            {
                segments.emplace_back(payloads.front());
            }
        }

        return segments;
    }

    std::int32_t SessionId() const noexcept
    {
        return current_header_.session_id_;
    }

    std::vector<std::vector<std::shared_ptr<vsomeip::payload>>> segments_payloads_;
    MultipartMessageHeader current_header_{0x0};
    std::uint8_t ignore_session_id_{std::numeric_limits<std::uint16_t>::signaling_NaN()};
};
