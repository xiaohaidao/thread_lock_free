#include "gtest/gtest.h"
#include "interruptible_thread.h"

std::mutex config_mutex;
std::vector<thread::interruptible_thread<std::function<void()>>> background_threads;

void background_thread(int disk_id) {
  while (true) {
    thread::interruption_point();
    std::this_thread::yield();
  }
}

void start_background_processing() {
  background_threads.push_back(
    thread::interruptible_thread<std::function<void()>>(
      std::bind(background_thread, 1)));

  background_threads.push_back(
    thread::interruptible_thread<std::function<void()>>(
      std::bind(background_thread, 2)));
}

TEST(InterruptibleThread, InterruptibleThreadFunction1) {
    start_background_processing();
    // process_gui_until_exit();
    std::unique_lock<std::mutex> lk(config_mutex);
    for (auto &iter : background_threads) {
      iter.interrupt();
    }
    for (auto &iter : background_threads) {
      iter.join();
    }

}