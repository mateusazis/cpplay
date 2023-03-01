#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <type_traits>

namespace {
int nextId = 0;
}

class Buffer {
public:
  Buffer(size_t size) : data(malloc(size)), id(nextId++), data_size(size) {}
  Buffer(const Buffer &other)
      : data(malloc(other.data_size)), id(nextId++),
        data_size(other.data_size) {
    memcpy(const_cast<void *>(data), other.data, other.data_size);
  }

  Buffer(Buffer &&other)
      : data(other.data), id(other.id), data_size(other.data_size) {
    other.data = nullptr;
    other.data_size = 0;
  }

  ~Buffer() {
    printf("Freeing buffer: %d\n", id);
    if (data != nullptr) {
      free(data);
    }
  }

  // private:
  void *data;
  const int id;
  size_t data_size;
};

void print_buffer(const Buffer &b) {
  printf("Buffer %d starts at: 0x%llx, with %lu bytes\n", b.id,
         (uint64_t)b.data, b.data_size);
}

void print_buffer2(Buffer b) { print_buffer(b); }

int main(int, char **) {
  Buffer b(4096);
  print_buffer(b);
  print_buffer(std::move(b));
  print_buffer2(std::move(b));
  print_buffer(b);
  print_buffer2(b);
  return 0;
}
