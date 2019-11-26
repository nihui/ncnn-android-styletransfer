// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>

#include <string>
#include <vector>

// ncnn
#include "net.h"
#include "benchmark.h"

#include "styletransfer.id.h"
#include "styletransfer.param.bin.h"

static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

static ncnn::Net styletransfernet[5];

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "StyleTransferNcnn", "JNI_OnLoad");

    ncnn::create_gpu_instance();

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "StyleTransferNcnn", "JNI_OnUnload");

    ncnn::destroy_gpu_instance();
}

// public native boolean Init(AssetManager mgr);
JNIEXPORT jboolean JNICALL Java_com_tencent_styletransferncnn_StyleTransferNcnn_Init(JNIEnv* env, jobject thiz, jobject assetManager)
{
    ncnn::Option opt;
    opt.lightmode = true;
    opt.num_threads = 4;
    opt.blob_allocator = &g_blob_pool_allocator;
    opt.workspace_allocator = &g_workspace_pool_allocator;

    // use vulkan compute
    if (ncnn::get_gpu_count() != 0)
        opt.use_vulkan_compute = true;

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

    const char* model_paths[5] = {"candy.bin", "mosaic.bin", "pointilism.bin", "rain_princess.bin", "udnie.bin"};
    for (int i=0; i<5; i++)
    {
        styletransfernet[i].opt = opt;

        int ret0 = styletransfernet[i].load_param(styletransfer_param_bin);
        int ret1 = styletransfernet[i].load_model(mgr, model_paths[i]);

        __android_log_print(ANDROID_LOG_DEBUG, "StyleTransferNcnn", "load %d %d", ret0, ret1);
    }

    return JNI_TRUE;
}

// public native Bitmap StyleTransfer(Bitmap bitmap, int style_type, boolean use_gpu);
JNIEXPORT jboolean JNICALL Java_com_tencent_styletransferncnn_StyleTransferNcnn_StyleTransfer(JNIEnv* env, jobject thiz, jobject bitmap, jint style_type, jboolean use_gpu)
{
    if (style_type < 0 || style_type >= 5)
        return JNI_FALSE;

    if (use_gpu == JNI_TRUE && ncnn::get_gpu_count() == 0)
        return JNI_FALSE;

    double start_time = ncnn::get_current_time();

    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env, bitmap, &info);
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
        return JNI_FALSE;

    int width = info.width;
    int height = info.height;

    const int downscale_ratio = 2;

    // ncnn from bitmap
    ncnn::Mat in = ncnn::Mat::from_android_bitmap_resize(env, bitmap, ncnn::Mat::PIXEL_RGB, width / downscale_ratio, height / downscale_ratio);

    // styletransfer
    ncnn::Mat out;
    {
        ncnn::Extractor ex = styletransfernet[style_type].create_extractor();

        ex.set_vulkan_compute(use_gpu);

        ex.input(styletransfer_param_id::BLOB_input1, in);

        ex.extract(styletransfer_param_id::BLOB_output1, out);
    }

    // ncnn to bitmap
    out.to_android_bitmap(env, bitmap, ncnn::Mat::PIXEL_RGB);

    double elasped = ncnn::get_current_time() - start_time;
    __android_log_print(ANDROID_LOG_DEBUG, "StyleTransferNcnn", "%.2fms   styletransfer", elasped);

    return JNI_TRUE;
}

}
