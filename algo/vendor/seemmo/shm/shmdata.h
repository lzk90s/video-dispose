#pragma once

#include <iostream>
#include <stdint.h>
#include <sys/shm.h>
#include <memory>
#include <map>
#include <mutex>

#include "common/helper/singleton.h"
#include "common/helper/logger.h"

//共享图片buffer
typedef struct {
    uint8_t *bgr24Buff1;	// bgr24 图片缓存1（检测跟踪）
    uint8_t *bgr24Buff2;	// bgr24 图片缓存2（识别）
} SharedImageBuffer;


namespace video {
namespace algo {
namespace seemmo {

class SharedImageMemory {
public:
    SharedImageMemory(int32_t channelId, uint32_t width, uint32_t height, bool autoDelete = false) {
        this->channelId_ = channelId;
        shm_ = nullptr;
        autoDelete_ = autoDelete;
        width_ = width;
        height_ = height;

        //末尾留一字节
        uint32_t frameSize = width * height * 3 + 1;

        //加一个大数，避免冲突
        key_t key = (key_t)(channelId_ + 20000);

        //framesize*2
        shmid_ = shmget(key, frameSize * 2, 0666 | IPC_CREAT | IPC_EXCL);
        if (shmid_ == -1) {
            LOG_WARN("Shared memory already exist");
            //已经存在，直接get
            shmid_ = shmget(key, sizeof(SharedImageBuffer), 0666 | IPC_CREAT);
            if (shmid_ == -1) {
                throw std::runtime_error("Create shared memory failed");
            }
        }

        shm_ = shmat(shmid_, 0, 0);
        if (shm_ == (void *)-1) {
            throw std::runtime_error("shmmat error");
        }

        LOG_INFO("Allocate shared memory {} for channel {}, w={}, h={}", shmid_, channelId_, width, height);

        //修改指针位置
        buffer_.bgr24Buff1 = (uint8_t*)shm_;
        buffer_.bgr24Buff2 = (uint8_t*)shm_ + frameSize;
    }

    ~SharedImageMemory() {
        // 把共享内存从当前进程中分离
        shmdt(shm_);
        if (autoDelete_) {
            shmctl(shmid_, IPC_RMID, 0);
        }
    }

    SharedImageBuffer& GetBuffer() {
        return this->buffer_;
    }

private:
    int32_t shmid_;
    int32_t channelId_;
    uint32_t width_;
    uint32_t height_;
    bool autoDelete_;
    SharedImageBuffer buffer_;
    void *shm_;
};

class SharedImageMemoryMng {
public:
    ~SharedImageMemoryMng() {
        std::unique_lock<std::mutex> lck(mutex_);
        sharedMemMap_.clear();
    }

    void Delete(int32_t channelId) {
        std::unique_lock<std::mutex> lck(mutex_);
        sharedMemMap_.erase(channelId);
    }

    std::shared_ptr<SharedImageMemory> CreateAndGet(int32_t channelId, uint32_t width, uint32_t height,
            bool autoDelete = false) {
        std::unique_lock<std::mutex> lck(mutex_);
        if (sharedMemMap_.find(channelId) == sharedMemMap_.end()) {
            auto m = std::make_shared<SharedImageMemory>(channelId, width, height, autoDelete);
            sharedMemMap_[channelId] = m;
            return m;
        } else {
            return sharedMemMap_[channelId];
        }
    }

private:
    std::mutex mutex_;
    std::map <int32_t, std::shared_ptr<SharedImageMemory>> sharedMemMap_;
};

typedef Singleton<SharedImageMemoryMng> SIMMNG;

}
}
}