#include <cstdlib>
#include <deque>
#include <functional>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

template <typename T> class WorkManager;

namespace {
bool isPrime(int64_t v) {
  if (v <= 2) {
    return true;
  }
  int64_t root = (int64_t)sqrt(v);
  for (int64_t div = 3; div <= root; div += 2) {
    if (v % div == 0) {
      return false;
    }
  }
  return true;
}

template <typename T> struct ThreadArgs {
  WorkManager<T> *workMgr;
  int threadId;
};
} // namespace

template <typename T> void *handle(void *arg) {
  ThreadArgs<T> *args = static_cast<ThreadArgs<T> *>(arg);
  char threadname[256];
  snprintf(threadname, sizeof(threadname), "Thread#%d", args->threadId);
  pthread_setname_np(threadname);
  WorkManager<T> *mgr = args->workMgr;
  delete args;
  mgr->handleForever();
  return nullptr;
}

template <typename T> class WorkManager {
public:
  WorkManager(int workerCount, size_t capacity,
              std::function<void(T work)> handler)
      : tasks(0), handler(handler), capacity(capacity) {
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_cond_init(&capacity_cond, nullptr);

    for (int i = 0; i < workerCount; i++) {
      pthread_t thread;

      ThreadArgs<T> *args = new ThreadArgs<T>;
      args->workMgr = this;
      args->threadId = i;

      pthread_create(&thread, nullptr, &handle<T>, args);
    }
  }
  WorkManager(const WorkManager<T> &other) = delete;

  void handleForever() {
    while (true) {
      T t;
      bool atCapacity = false;
      pthread_mutex_lock(&lock);
      while (!getNextTask(&t, atCapacity)) {
        pthread_cond_wait(&cond, &lock);
      }
      pthread_mutex_unlock(&lock);

      if (atCapacity) {
        printf("Siiiignal\n");
        pthread_cond_signal(&capacity_cond);
      }
      handler(t);
    }
  }

  void addWork(const T &work) {
    pthread_mutex_lock(&lock);

    while (isFull()) {
      printf("Waiting for capacity: %lld, size: %ld, cap: %ld...\n", work,
             tasks.size(), capacity);
      int ret;
      if ((ret = pthread_cond_wait(&capacity_cond, &lock))) {
        perror("pthread_cond_wait");
        printf("Failed to wait: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
      }
    }
    tasks.push_back(work);

    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&cond);
  }

private:
  bool getNextTask(T *out, bool &atCapacity) {
    auto front = tasks.begin();
    if (front == tasks.end()) {
      return false;
    }
    *out = *front;
    atCapacity = isFull();
    tasks.erase(front);
    return true;
  }

  bool isFull() const { return tasks.size() == capacity; }

  pthread_cond_t cond;

  pthread_cond_t capacity_cond;
  pthread_mutex_t lock;
  std::deque<T> tasks;
  std::function<void(T work)> handler;
  const size_t capacity;
};

void handle_work(int64_t v) {
  if (isPrime(v)) {
    char name[256];
    pthread_t self = pthread_self();
    pthread_getname_np(self, name, sizeof(name));
    printf("%lld is prime (%s)\n", v, name);
  }
}

int main(int, char **) {
  fclose(stdin);
  fclose(stderr);
  WorkManager<int64_t> mgr(/* workers= */ 10, /* capacity */ 1000, handle_work);
  int64_t n = 0;
  while (true) {
    for (int i = 0; i < 1000; i++) {
      mgr.addWork(n++);
    }
    // sleep(1);
  }
  return 0;
}
