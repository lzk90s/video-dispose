#pragma once

#include <iostream>
#include <stdint.h>
#include <sys/shm.h>
#include <memory>
#include <map>
#include <mutex>

#include "common/helper/singleton.h"


//����ͼƬbuffer
typedef struct {
    uint8_t *bgr24Buff1;	// bgr24 ͼƬ����1�������٣�
    uint8_t *bgr24Buff2;	// bgr24 ͼƬ����2��ʶ��
} SharedImageBuffer;

using namespace  std;

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

        //ĩβ��һ�ֽ�
        uint32_t frameSize = width * height * 3 + 1;

        //��һ�������������ͻ
        key_t key = (key_t)(channelId_ + 20000);

        //framesize*2
        shmid_ = shmget(key, frameSize*2,  0666 | IPC_CREAT | IPC_EXCL);
        if (shmid_ == -1) {
            cout << "Shared memory already exist" << endl;
            //�Ѿ����ڣ�ֱ��get
            shmid_ = shmget(key, sizeof(SharedImageBuffer), 0666 | IPC_CREAT);
            if (shmid_ == -1) {
                throw runtime_error("Create shared memory failed");
            }
        }

        shm_ = shmat(shmid_, 0, 0);
        if (shm_ == (void *)-1) {
            throw runtime_error("shmmat error");
        }

        cout << "Allocate shared memory " << shmid_ << " for channel " << channelId << ", frameSize " << frameSize <<  endl;

        //�޸�ָ��λ��
        buffer_.bgr24Buff1 = (uint8_t*)shm_;
        buffer_.bgr24Buff2 = (uint8_t*)shm_ + frameSize;
    }

    ~SharedImageMemory() {
        // �ѹ����ڴ�ӵ�ǰ�����з���
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
        unique_lock<mutex> lck(mutex_);
        sharedMemMap_.clear();
    }

    void Create(int32_t channelId, uint32_t width, uint32_t height, bool autoDelete=false) {
        unique_lock<mutex> lck(mutex_);
        if (sharedMemMap_.find(channelId) != sharedMemMap_.end()) {
            return;
        }
        sharedMemMap_[channelId] = make_shared<SharedImageMemory>(channelId, width, height, autoDelete);
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