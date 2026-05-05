#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>

namespace rate_limiter {

// TODO: split functionallity
class SlidingWindowCounter {
 public:
  struct Result {
    bool allowed;
    int limit;
    int remaining;
    std::chrono::seconds reset_in;
  };

  SlidingWindowCounter(int max_requests, std::chrono::milliseconds window_size)
      : max_requests_(max_requests),
        window_size_(window_size),
        window_start_(std::chrono::steady_clock::now()) {}

  Result TryAcquire() {
    std::lock_guard<std::mutex> lock(mtx_);
    auto now = std::chrono::steady_clock::now();
    AdvanceWindows(now);

    const auto elapsed = now - window_start_;
    const double weight = std::chrono::duration<double>(elapsed) / window_size_;
    const auto estimated = prev_count_ * (1.0 - weight) + curr_count_;

    auto remaining =
        std::max(0, max_requests_ - static_cast<int>(std::ceil(estimated)));

    const auto reset_in = std::chrono::duration_cast<std::chrono::seconds>(
        window_size_ - (now - window_start_));

    std::cout << "[SlidingWindowCounter] Prev count: " << prev_count_
              << std::endl;
    std::cout << "[SlidingWindowCounter] Current count: " << curr_count_
              << std::endl;
    std::cout << "[SlidingWindowCounter] Weight: " << weight << std::endl;
    std::cout << "[SlidingWindowCounter] Estimated requests: " << estimated
              << std::endl;

    const auto allowed = estimated < max_requests_;

    if (allowed) {
      ++curr_count_;
      --remaining;
    }

    return {
        .allowed = allowed,
        .limit = max_requests_,
        .remaining = remaining,
        .reset_in = reset_in,
    };
  }

 private:
  void AdvanceWindows(std::chrono::steady_clock::time_point now) {
    auto elapsed = now - window_start_;
    if (elapsed >= window_size_) {
      auto passed =
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed) /
          window_size_;
      prev_count_ = (passed >= 2) ? 0 : curr_count_;
      curr_count_ = 0;
      window_start_ += passed * window_size_;
    }
  }

  const int max_requests_;
  const std::chrono::milliseconds window_size_;
  std::chrono::steady_clock::time_point window_start_;
  int prev_count_ = 0, curr_count_ = 0;
  std::mutex mtx_;
};

}  // namespace rate_limiter