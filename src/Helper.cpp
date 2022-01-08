#include "Helper.h"

void RolingAcc::reset() {
  for (size_t i=0; i<size_; i++) { buffer_[i] = 0.0; }
  sum_ = 0.0;
  fill_ = 0;
  n_ = 0;
}

float RolingAcc::avg(float v) {
  sum_ += v;
  sum_ -= buffer_[n_];
  buffer_[n_++] = v;
  if (n_ >= size_) { n_ = 0; }
  if (fill_ < size_) { fill_++; }
  return sum_ / fill_;
}
