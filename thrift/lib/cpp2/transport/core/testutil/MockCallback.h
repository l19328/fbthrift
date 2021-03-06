/*
 * Copyright 2018-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache {
namespace thrift {

class MockCallback : public RequestCallback {
 public:
  MockCallback(bool clientError, bool serverError)
      : clientError_(clientError), serverError_(serverError) {}
  virtual ~MockCallback() {
    EXPECT_TRUE(callbackReceived_);
  }
  void requestSent() override {
    EXPECT_FALSE(requestSentCalled_);
    requestSentCalled_ = true;
  }
  void replyReceived(ClientReceiveState&& crs) override {
    EXPECT_FALSE(crs.isException());
    EXPECT_TRUE(requestSentCalled_);
    EXPECT_FALSE(callbackReceived_);
    EXPECT_FALSE(clientError_);
    auto reply = crs.buf()->cloneAsValue().moveToFbString();
    bool expired = (reply.find("Task expired") != folly::fbstring::npos) ||
        (reply.find("Queue Timeout") != folly::fbstring::npos);
    EXPECT_EQ(serverError_, expired);
    callbackReceived_ = true;
  }
  void requestError(ClientReceiveState&& crs) override {
    // If clientError_ is expected, then request should not be send!
    EXPECT_NE(clientError_, requestSentCalled_);
    EXPECT_TRUE(crs.isException());
    EXPECT_TRUE(
        crs.exception().is_compatible_with<transport::TTransportException>());
    EXPECT_FALSE(callbackReceived_);
    EXPECT_TRUE(clientError_ || serverError_);
    callbackReceived_ = true;
  }
  bool callbackReceived() {
    return callbackReceived_;
  }

 private:
  bool clientError_;
  bool serverError_;
  bool requestSentCalled_{false};
  bool callbackReceived_{false};
};
} // namespace thrift
} // namespace apache
