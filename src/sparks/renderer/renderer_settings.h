#pragma once

namespace sparks {
struct RendererSettings {
  int num_samples{1};
  int num_bounces{16};
  float rr_probability{0.73};
};
}  // namespace sparks
