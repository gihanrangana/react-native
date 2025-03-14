/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JReactMarker.h"
#include <cxxreact/ReactMarker.h>
#include <fbjni/fbjni.h>
#include <glog/logging.h>
#include <mutex>

namespace facebook::react {

void JReactMarker::setLogPerfMarkerIfNeeded() {
  static std::once_flag flag{};
  std::call_once(flag, []() {
    ReactMarker::logTaggedMarkerImpl = JReactMarker::logPerfMarker;
    ReactMarker::logTaggedMarkerBridgelessImpl =
        JReactMarker::logPerfMarkerBridgeless;
    ReactMarker::getAppStartTimeImpl = JReactMarker::getAppStartTime;
  });
}

void JReactMarker::logMarker(const std::string &marker) {
  static auto cls = javaClassStatic();
  static auto meth = cls->getStaticMethod<void(std::string)>("logMarker");
  meth(cls, marker);
}

void JReactMarker::logMarker(
    const std::string &marker,
    const std::string &tag) {
  static auto cls = javaClassStatic();
  static auto meth =
      cls->getStaticMethod<void(std::string, std::string)>("logMarker");
  meth(cls, marker, tag);
}

void JReactMarker::logMarker(
    const std::string &marker,
    const std::string &tag,
    const int instanceKey) {
  static auto cls = javaClassStatic();
  static auto meth =
      cls->getStaticMethod<void(std::string, std::string, int)>("logMarker");
  meth(cls, marker, tag, instanceKey);
}

void JReactMarker::logPerfMarker(
    const ReactMarker::ReactMarkerId markerId,
    const char *tag) {
  const int bridgeInstanceKey = 0;
  logPerfMarkerWithInstanceKey(markerId, tag, bridgeInstanceKey);
}

void JReactMarker::logPerfMarkerBridgeless(
    const ReactMarker::ReactMarkerId markerId,
    const char *tag) {
  const int bridgelessInstanceKey = 1;
  logPerfMarkerWithInstanceKey(markerId, tag, bridgelessInstanceKey);
}

void JReactMarker::logPerfMarkerWithInstanceKey(
    const ReactMarker::ReactMarkerId markerId,
    const char *tag,
    const int instanceKey) {
  switch (markerId) {
    case ReactMarker::RUN_JS_BUNDLE_START:
      JReactMarker::logMarker("RUN_JS_BUNDLE_START", tag, instanceKey);
      break;
    case ReactMarker::RUN_JS_BUNDLE_STOP:
      JReactMarker::logMarker("RUN_JS_BUNDLE_END", tag, instanceKey);
      break;
    case ReactMarker::CREATE_REACT_CONTEXT_STOP:
      JReactMarker::logMarker("CREATE_REACT_CONTEXT_END");
      break;
    case ReactMarker::JS_BUNDLE_STRING_CONVERT_START:
      JReactMarker::logMarker("loadApplicationScript_startStringConvert");
      break;
    case ReactMarker::JS_BUNDLE_STRING_CONVERT_STOP:
      JReactMarker::logMarker("loadApplicationScript_endStringConvert");
      break;
    case ReactMarker::NATIVE_MODULE_SETUP_START:
      JReactMarker::logMarker("NATIVE_MODULE_SETUP_START", tag, instanceKey);
      break;
    case ReactMarker::NATIVE_MODULE_SETUP_STOP:
      JReactMarker::logMarker("NATIVE_MODULE_SETUP_END", tag, instanceKey);
      break;
    case ReactMarker::REGISTER_JS_SEGMENT_START:
      JReactMarker::logMarker("REGISTER_JS_SEGMENT_START", tag, instanceKey);
      break;
    case ReactMarker::REGISTER_JS_SEGMENT_STOP:
      JReactMarker::logMarker("REGISTER_JS_SEGMENT_STOP", tag, instanceKey);
      break;
    case ReactMarker::NATIVE_REQUIRE_START:
    case ReactMarker::NATIVE_REQUIRE_STOP:
    case ReactMarker::REACT_INSTANCE_INIT_START:
    case ReactMarker::REACT_INSTANCE_INIT_STOP:
      // These are not used on Android.
      break;
  }
}

double JReactMarker::getAppStartTime() {
  static auto cls = javaClassStatic();
  static auto meth = cls->getStaticMethod<double()>("getAppStartTime");
  return meth(cls);
}

void JReactMarker::nativeLogMarker(
    jni::alias_ref<jclass> /* unused */,
    std::string markerNameStr,
    jlong markerTime) {
  // TODO: refactor this to a bidirectional map along with
  // logPerfMarkerWithInstanceKey
  if (markerNameStr == "RUN_JS_BUNDLE_START") {
    ReactMarker::logMarkerDone(
        ReactMarker::RUN_JS_BUNDLE_START, (double)markerTime);
  } else if (markerNameStr == "RUN_JS_BUNDLE_END") {
    ReactMarker::logMarkerDone(
        ReactMarker::RUN_JS_BUNDLE_STOP, (double)markerTime);
  } else if (markerNameStr == "CREATE_REACT_CONTEXT_END") {
    ReactMarker::logMarkerDone(
        ReactMarker::CREATE_REACT_CONTEXT_STOP, (double)markerTime);
  } else if (markerNameStr == "loadApplicationScript_startStringConvert") {
    ReactMarker::logMarkerDone(
        ReactMarker::JS_BUNDLE_STRING_CONVERT_START, (double)markerTime);
  } else if (markerNameStr == "loadApplicationScript_endStringConvert") {
    ReactMarker::logMarkerDone(
        ReactMarker::JS_BUNDLE_STRING_CONVERT_STOP, (double)markerTime);
  } else if (markerNameStr == "NATIVE_MODULE_SETUP_START") {
    ReactMarker::logMarkerDone(
        ReactMarker::NATIVE_MODULE_SETUP_START, (double)markerTime);
  } else if (markerNameStr == "NATIVE_MODULE_SETUP_END") {
    ReactMarker::logMarkerDone(
        ReactMarker::NATIVE_MODULE_SETUP_STOP, (double)markerTime);
  } else if (markerNameStr == "REGISTER_JS_SEGMENT_START") {
    ReactMarker::logMarkerDone(
        ReactMarker::REGISTER_JS_SEGMENT_START, (double)markerTime);
  } else if (markerNameStr == "REGISTER_JS_SEGMENT_STOP") {
    ReactMarker::logMarkerDone(
        ReactMarker::REGISTER_JS_SEGMENT_STOP, (double)markerTime);
  }
}

void JReactMarker::registerNatives() {
  javaClassLocal()->registerNatives({
      makeNativeMethod("nativeLogMarker", JReactMarker::nativeLogMarker),
  });
}

} // namespace facebook::react
