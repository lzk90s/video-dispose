#pragma once

#include <iostream>
#include <stdint.h>
#include <sys/shm.h>
#include <memory>
#include <map>
#include <mutex>

#include "common/helper/singleton.h"

//ͼƬ���20M
#define IMAGE_MAX_SIZE 20*1024*1024

//����ͼƬbuffer
typedef struct {
    uint8_t bgr24Buff1[IMAGE_MAX_SIZE];	// bgr24 ͼƬ����1�������٣�
    uint8_t bgr24Buff2[IMAGE_MAX_SIZE];	// bgr24 ͼƬ����2��ʶ��
} SharedImageBuffer;

using namespace  std;

namespace algo {
namespace seemmo {

class SharedImageMemory {
public:
    SharedImageMemory(int32_t channelId) {
        this->channelId_ = channelId;
        shm_ = nullptr;

        //��һ�������������ͻ
        key_t key = (key_t)(channelId_ + 20000);

        shmid_ = shmget(key, sizeof(SharedImageBuffer), 0666 | IPC_CREAT);
        if (shmid_ == -1) {
            throw runtime_error("Get shared memory failed");
        }

        shm_ = shmat(shmid_, 0, 0);
        if (shm_ == (void *)-1) {
            throw runtime_error("shmmat error");
        }

        cout << "Allocate shared memory " << shmid_ << " for channel " << channelId <<  endl;

        buffer_ = (SharedImageBuffer*)shm_;
    }

    ~SharedImageMemory() {
        // �ѹ����ڴ�ӵ�ǰ�����з���
        shmdt(shm_);
        shmctl(shmid_, IPC_RMID, 0);
    }

    SharedImageBuffer& GetBuffer() {
        return *this->buffer_;
    }

private:
    int32_t shmid_;
    int32_t channelId_;
    SharedImageBuffer *buffer_;
    void *shm_;
};

class SharedImageMemoryMng {
public:
    ~SharedImageMemoryMng() {
        unique_lock<mutex> lck(mutex_);
        sharedMemMap_.clear();
    }

    void Create(int32_t channelId) {
        unique_lock<mutex> lck(mutex_);
        if (sharedMemMap_.find(channelId) != sharedMemMap_.end()) {
            return;
        }
        sharedMemMap_[channelId] = make_shared<SharedImageMemory>(channelId);
    }

    void Delete(int32_t channelId) {
        unique_lock<mutex> lck(mutex_);
        sharedMemMap_.erase(channelId);
    }

    shared_ptr<SharedImageMemory> Get(int32_t channelId) {
        unique_lock<mutex> lck(mutex_);
        return sharedMemMap_[channelId];
    }

    bool Exist(int32_t channelId) {
        unique_lock<mutex> lck(mutex_);
        return sharedMemMap_.find(channelId) != sharedMemMap_.end();
    }

private:
    mutex mutex_;
    map <int32_t, shared_ptr<SharedImageMemory>> sharedMemMap_;
};

typedef Singleton<SharedImageMemoryMng> SIMMNG;

}
}