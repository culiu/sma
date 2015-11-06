#pragma once

#include <jni.h>
#include <memory>


extern "C" {
JNIEXPORT jboolean JNICALL Java_edu_asu_sma_NodeContainer_create(JNIEnv*, jobject, jint);
JNIEXPORT void JNICALL Java_edu_asu_sma_NodeContainer_dispose(JNIEnv*, jobject);
}

namespace sma
{
struct Context;
class LinkLayerImpl;
class NeighborHelperImpl;
class InterestHelperImpl;
class ContentHelperImpl;
class BehaviorHelperImpl;
class CcnNode;

extern std::unique_ptr<Context> ctx;
extern std::unique_ptr<LinkLayerImpl> linklayer;

extern std::unique_ptr<NeighborHelperImpl> neighbor_helper;
extern std::unique_ptr<InterestHelperImpl> interest_helper;
extern std::unique_ptr<ContentHelperImpl> content_helper;
extern std::unique_ptr<BehaviorHelperImpl> behavior_helper;

extern std::unique_ptr<CcnNode> node;
}
