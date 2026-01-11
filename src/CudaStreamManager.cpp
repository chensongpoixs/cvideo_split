/**
 * @file CudaStreamManager.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief CUDA Stream 统一管理器实现
 * @see CudaStreamManager.h
 */

#include "CudaStreamManager.h"
#include "Logger.h"

bool CudaStreamManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }

    LOG_INFO("Initializing CudaStreamManager - unified CUDA stream management");

    // 确保在正确的 GPU 上下文
    cudaError_t err = cudaSetDevice(0);
    if (err != cudaSuccess) {
        LOG_ERROR("cudaSetDevice(0) failed: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    // 创建 streams（使用 NonBlocking 标志，允许与默认 stream 并行）
    err = cudaStreamCreateWithFlags(&decode_stream_, cudaStreamNonBlocking);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create decode stream: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    err = cudaStreamCreateWithFlags(&stitch_stream_, cudaStreamNonBlocking);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create stitch stream: " + std::string(cudaGetErrorString(err)));
        cleanup();
        return false;
    }

    err = cudaStreamCreateWithFlags(&display_stream_, cudaStreamNonBlocking);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create display stream: " + std::string(cudaGetErrorString(err)));
        cleanup();
        return false;
    }

    err = cudaStreamCreateWithFlags(&encode_stream_, cudaStreamNonBlocking);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create encode stream: " + std::string(cudaGetErrorString(err)));
        cleanup();
        return false;
    }

    // 创建 events（DisableTiming 标志提高性能）
    err = cudaEventCreateWithFlags(&decode_complete_event_, cudaEventDisableTiming);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create decode complete event: " + std::string(cudaGetErrorString(err)));
        cleanup();
        return false;
    }

    err = cudaEventCreateWithFlags(&stitch_complete_event_, cudaEventDisableTiming);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create stitch complete event: " + std::string(cudaGetErrorString(err)));
        cleanup();
        return false;
    }

    err = cudaEventCreateWithFlags(&display_complete_event_, cudaEventDisableTiming);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create display complete event: " + std::string(cudaGetErrorString(err)));
        cleanup();
        return false;
    }

    err = cudaEventCreateWithFlags(&encode_complete_event_, cudaEventDisableTiming);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create encode complete event: " + std::string(cudaGetErrorString(err)));
        cleanup();
        return false;
    }

    initialized_ = true;
    LOG_INFO("CudaStreamManager initialized successfully with 4 streams and 4 events (async pipeline ready)");
    return true;
}

void CudaStreamManager::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        return;
    }

    LOG_INFO("Cleaning up CudaStreamManager");

    // 先同步所有 stream
    if (decode_stream_) cudaStreamSynchronize(decode_stream_);
    if (stitch_stream_) cudaStreamSynchronize(stitch_stream_);
    if (display_stream_) cudaStreamSynchronize(display_stream_);
    if (encode_stream_) cudaStreamSynchronize(encode_stream_);

    // 销毁 events
    if (decode_complete_event_) {
        cudaEventDestroy(decode_complete_event_);
        decode_complete_event_ = nullptr;
    }
    if (stitch_complete_event_) {
        cudaEventDestroy(stitch_complete_event_);
        stitch_complete_event_ = nullptr;
    }
    if (display_complete_event_) {
        cudaEventDestroy(display_complete_event_);
        display_complete_event_ = nullptr;
    }
    if (encode_complete_event_) {
        cudaEventDestroy(encode_complete_event_);
        encode_complete_event_ = nullptr;
    }

    // 销毁 streams
    if (decode_stream_) {
        cudaStreamDestroy(decode_stream_);
        decode_stream_ = nullptr;
    }
    if (stitch_stream_) {
        cudaStreamDestroy(stitch_stream_);
        stitch_stream_ = nullptr;
    }
    if (display_stream_) {
        cudaStreamDestroy(display_stream_);
        display_stream_ = nullptr;
    }
    if (encode_stream_) {
        cudaStreamDestroy(encode_stream_);
        encode_stream_ = nullptr;
    }

    initialized_ = false;
    LOG_INFO("CudaStreamManager cleaned up");
}

cudaEvent_t CudaStreamManager::recordDecodeComplete() {
    if (decode_stream_ && decode_complete_event_) {
        cudaEventRecord(decode_complete_event_, decode_stream_);
    }
    return decode_complete_event_;
}

cudaEvent_t CudaStreamManager::recordStitchComplete() {
    if (stitch_stream_ && stitch_complete_event_) {
        cudaEventRecord(stitch_complete_event_, stitch_stream_);
    }
    return stitch_complete_event_;
}

void CudaStreamManager::waitForDecode(cudaStream_t waiting_stream) {
    if (waiting_stream && decode_complete_event_) {
        cudaStreamWaitEvent(waiting_stream, decode_complete_event_, 0);
    }
}

void CudaStreamManager::waitForStitch(cudaStream_t waiting_stream) {
    if (waiting_stream && stitch_complete_event_) {
        cudaStreamWaitEvent(waiting_stream, stitch_complete_event_, 0);
    }
}

void CudaStreamManager::syncDecode() {
    if (decode_stream_) {
        cudaStreamSynchronize(decode_stream_);
    }
}

void CudaStreamManager::syncStitch() {
    if (stitch_stream_) {
        cudaStreamSynchronize(stitch_stream_);
    }
}

void CudaStreamManager::syncDisplay() {
    if (display_stream_) {
        cudaStreamSynchronize(display_stream_);
    }
}

void CudaStreamManager::syncEncode() {
    if (encode_stream_) {
        cudaStreamSynchronize(encode_stream_);
    }
}

void CudaStreamManager::syncAll() {
    syncDecode();
    syncStitch();
    syncDisplay();
    syncEncode();
}

// ===== 异步管道扩展实现 =====

cudaEvent_t CudaStreamManager::recordDisplayComplete() {
    if (display_stream_ && display_complete_event_) {
        cudaEventRecord(display_complete_event_, display_stream_);
    }
    return display_complete_event_;
}

cudaEvent_t CudaStreamManager::recordEncodeComplete() {
    if (encode_stream_ && encode_complete_event_) {
        cudaEventRecord(encode_complete_event_, encode_stream_);
    }
    return encode_complete_event_;
}

void CudaStreamManager::waitForDisplay(cudaStream_t waiting_stream) {
    if (waiting_stream && display_complete_event_) {
        cudaStreamWaitEvent(waiting_stream, display_complete_event_, 0);
    }
}

void CudaStreamManager::waitForEncode(cudaStream_t waiting_stream) {
    if (waiting_stream && encode_complete_event_) {
        cudaStreamWaitEvent(waiting_stream, encode_complete_event_, 0);
    }
}

bool CudaStreamManager::isDecodeComplete() {
    if (!decode_complete_event_) return true;
    cudaError_t status = cudaEventQuery(decode_complete_event_);
    return (status == cudaSuccess);
}

bool CudaStreamManager::isStitchComplete() {
    if (!stitch_complete_event_) return true;
    cudaError_t status = cudaEventQuery(stitch_complete_event_);
    return (status == cudaSuccess);
}

bool CudaStreamManager::isDisplayComplete() {
    if (!display_complete_event_) return true;
    cudaError_t status = cudaEventQuery(display_complete_event_);
    return (status == cudaSuccess);
}

bool CudaStreamManager::isEncodeComplete() {
    if (!encode_complete_event_) return true;
    cudaError_t status = cudaEventQuery(encode_complete_event_);
    return (status == cudaSuccess);
}

cudaEvent_t CudaStreamManager::createAndRecordEvent(cudaStream_t stream) {
    cudaEvent_t event = nullptr;
    cudaError_t err = cudaEventCreateWithFlags(&event, cudaEventDisableTiming);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to create event: " + std::string(cudaGetErrorString(err)));
        return nullptr;
    }
    if (stream) {
        cudaEventRecord(event, stream);
    }
    return event;
}

void CudaStreamManager::streamWaitEvent(cudaStream_t stream, cudaEvent_t event) {
    if (stream && event) {
        cudaStreamWaitEvent(stream, event, 0);
    }
}

