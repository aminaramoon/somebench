// Copyright Aeva 2018

#pragma once

#include <cstdint>

union MultipartMessageHeader {
  struct
  {
    unsigned session_id_ : 8;
    unsigned segment_id_ : 8;
    unsigned fragment_id_ : 8;
    bool start_of_session_ : 1;
    bool end_of_session_ : 1;
    bool start_of_segment_ : 1;
    bool end_of_segment_ : 1;
    unsigned shim_ : 4;
    // pad is needed for capnproto for correct alignment of payload
    unsigned padding_;
  };
  std::uint64_t raw_;

  constexpr MultipartMessageHeader(const std::uint64_t raw) : raw_(raw) {}

  constexpr MultipartMessageHeader &operator=(const std::uint64_t raw)
  {
    raw_ = raw;
    return *this;
  }

  inline constexpr bool IsNewSession(const MultipartMessageHeader &incoming) const noexcept
  {
    return (session_id_ + 1 <= incoming.session_id_);
  }

  inline constexpr bool IsBeginningOfNewSession(
      const MultipartMessageHeader &incoming) const noexcept
  {
    return IsNewSession(incoming) && incoming.IsBeginningOfMessage();
  }

  inline constexpr bool IsEndOfMessage() const noexcept
  {
    return end_of_segment_ && end_of_session_;
  }

  inline constexpr bool IsEndOfSegment() const noexcept { return end_of_segment_; }

  inline constexpr bool IsBeginningOfMessage() const noexcept
  {
    return start_of_session_ && start_of_segment_;
  }

  inline constexpr bool IsBeginningOfSegment() const noexcept { return start_of_segment_; }

  inline constexpr bool IsNextFragment(const MultipartMessageHeader &incoming) const noexcept
  {
    if (session_id_ == incoming.session_id_)
    {
      if (segment_id_ == incoming.segment_id_)
      {
        return fragment_id_ + 1 == incoming.fragment_id_;
      }
      return IsEndOfSegment() && ((segment_id_ + 1) == incoming.segment_id_) &&
             incoming.IsBeginningOfSegment();
    }
    return false;
  }
};

static constexpr std::int32_t UdpMessageSize = 1400;
static constexpr std::int32_t TcpMessageSize = 65535;
static constexpr std::int32_t MultipartMessageHeaderSize = sizeof(MultipartMessageHeader);
static constexpr std::int32_t MaxUdpPayloadSize = UdpMessageSize - MultipartMessageHeaderSize;
