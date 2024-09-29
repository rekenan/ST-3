// Copyright 2024 MamedovKenan

#include "gtest/gtest.h"
#include "TimedDoor.h"
#include <thread> // NOLINT [build/c++11]
#include <chrono> // NOLINT [build/c++11]
#include <stdexcept>
#include <gmock/gmock.h>

using ::testing::Mock;

class MockDoor : public Door {
public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
protected:
    TimedDoor* door;
    Timer* timer;
    DoorTimerAdapter* adapter;

    void SetUp() override {
        door = new TimedDoor(1);
        timer = new Timer();
        adapter = new DoorTimerAdapter(*door);
    }

    void TearDown() override {
        delete door;
        delete timer;
        delete adapter;
    }
};

TEST_F(TimedDoorTest, TimeoutThrowsWhenDoorOpenedTooLong) {
    door->unlock();
    EXPECT_THROW(timer->tregister(1, adapter), std::runtime_error);
}

TEST_F(TimedDoorTest, TimeoutDoesNotThrowWhenDoorClosed) {
    door->lock();
    EXPECT_NO_THROW(timer->tregister(1, adapter));
}

TEST_F(TimedDoorTest, AdapterTimeoutCallsThrowState) {
    door->unlock();
    timer->tregister(1, adapter);
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, DoorIsInitiallyClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorCanBeOpenedAndClosed) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, TimeoutWithMultipleRegisters) {
    door->unlock();
    timer->tregister(1, adapter);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, TimeoutDoesNotThrowWhenDoorLocked) {
    door->lock();
    EXPECT_NO_THROW(timer->tregister(1, adapter));
}

TEST_F(TimedDoorTest, AdapterCallsThrowStateOnTimeout) {
    door->unlock();
    EXPECT_CALL(*door, throwState()).Times(1);
    adapter->Timeout();
}

TEST_F(TimedDoorTest, TimerWorksCorrectly) {
    door->lock();
    EXPECT_NO_THROW(timer->tregister(2, adapter));
}

TEST_F(TimedDoorTest, TimerDoesNotCallWhenDoorClosed) {
    door->lock();
    EXPECT_NO_THROW(timer->tregister(1, adapter));
}
