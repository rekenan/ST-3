// Copyright 2024 MamedovKenan

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include "TimedDoor.h"

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

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

  void SetUp() override {
    door = new TimedDoor(5);
    timer = new Timer();
  }

  void TearDown() override {
    delete door;
    delete timer;
  }
};

TEST_F(TimedDoorTest, DoorInitiallyClosed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorCanBeUnlocked) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorCanBeLocked) {
  door->unlock();
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, TimeoutThrowsWhenDoorOpenedTooLong) {
  door->unlock();
  EXPECT_THROW(timer->tregister(1, door), std::runtime_error);
}

TEST_F(TimedDoorTest, TimeoutDoesNotThrowWhenDoorClosed) {
  door->lock();
  EXPECT_NO_THROW(timer->tregister(1, door));
}

TEST_F(TimedDoorTest, AdapterTimeoutCallsThrowState) {
  MockDoor mockDoor;
  EXPECT_CALL(mockDoor, isDoorOpened())
      .WillOnce(testing::Return(true));

  DoorTimerAdapter adapter(mockDoor);
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, TimerCallsTimeout) {
  MockTimerClient mockClient;
  EXPECT_CALL(mockClient, Timeout()).Times(1);

  timer->tregister(1, &mockClient);
  std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST_F(TimedDoorTest, MultipleTimeouts) {
  MockTimerClient mockClient;
  EXPECT_CALL(mockClient, Timeout()).Times(2);

  timer->tregister(1, &mockClient);
  timer->tregister(2, &mockClient);
  std::this_thread::sleep_for(std::chrono::seconds(3));
}

TEST_F(TimedDoorTest, DoorTimeoutAfterUnlock) {
  door->unlock();
  EXPECT_THROW(timer->tregister(1, door), std::runtime_error);
}

TEST_F(TimedDoorTest, DoorDoesNotThrowAfterLock) {
  door->lock();
  EXPECT_NO_THROW(timer->tregister(1, door));
}
