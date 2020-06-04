// Copyright Aeva 2020

#pragma once

#include <algorithm>
#include <vector>

#include <vsomeip/vsomeip.hpp>
#include "msg_header.hpp"

struct MessageFragmenter
{
  void Feed(const std::string &message, const bool use_tcp)
  {
    message_ = message;
    is_tcp_ = use_tcp;
  }

  std::vector<std::shared_ptr<vsomeip::payload>> GetFragmentedMessages()
  {
    std::vector<std::shared_ptr<vsomeip::payload>> payloads;
    static std::size_t session_id = 0;
    session_id++;
    this->FragmentSegment(reinterpret_cast<const std::uint8_t *>(message_.data()), message_.size(), payloads, session_id, 0UL, 1UL, is_tcp_);
    return payloads;
  }

  void FragmentSegment(const std::uint8_t *data, const std::size_t size,
                       std::vector<std::shared_ptr<vsomeip::payload>> &payloads,
                       const std::size_t session_id,
                       const std::size_t segment_id,
                       const std::size_t segment_size,
                       const bool is_tcp) const
  {

    if (segment_size == 0U || size == 0U)
    {
      return;
    }

    std::size_t head = 0;
    std::int32_t remaining = static_cast<std::int32_t>(size);

    std::uint8_t fragment_id = 0;
    MultipartMessageHeader header{0U};
    header.session_id_ = session_id;
    header.segment_id_ = segment_id;

    const std::int32_t max_payload_size =
        is_tcp ? std::numeric_limits<std::int32_t>::max()
               : static_cast<std::int32_t>(UdpMessageSize);
    int payload_size = 0;
    for (; remaining > 0; remaining -= payload_size, head += payload_size)
    {
      auto payload = vsomeip::runtime::get()->create_payload();
      header.fragment_id_ = fragment_id++;
      header.start_of_segment_ = head == 0;
      header.end_of_segment_ = remaining - max_payload_size <= 0;
      header.start_of_session_ = segment_id == 0U && header.start_of_segment_;
      header.end_of_session_ =
          segment_id == segment_size - 1 && header.end_of_segment_;
      payload_size =
          std::min(remaining, max_payload_size - MultipartMessageHeaderSize);
      std::vector<vsomeip::byte_t> payload_bytes;
      payload_bytes.resize(payload_size + MultipartMessageHeaderSize);
      auto tail_iter =
          std::copy_n(reinterpret_cast<const vsomeip::byte_t *>(&header),
                      MultipartMessageHeaderSize, payload_bytes.begin());
      std::copy_n(reinterpret_cast<const vsomeip::byte_t *>(&data[head]),
                  payload_size, tail_iter);

      payload->set_data(std::move(payload_bytes));
      payloads.push_back(std::move(payload));
    }
  }

  std::string message_;
  bool is_tcp_{true};
};
