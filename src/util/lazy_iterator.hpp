#include <iostream>
#include <functional>
#include "../fhatos.hpp"

using namespace std;
namespace fhatos {

  template <typename T>
class LazyIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;

  LazyIterator(T start, Supplier<T> generator) : current(start), gen(generator) {}

  reference operator*()  {
    return current;
  }

  LazyIterator& operator++() {
    current = gen(current);
    return *this;
  }

  LazyIterator operator++(int) {
    LazyIterator old = *this;
    ++(*this);
    return old;
  }

  bool operator==(const LazyIterator& other) const {
    return current == other.current;
  }

  bool operator!=(const LazyIterator& other) const {
    return !(*this == other);
  }

private:
  int current;
  Supplier<T> gen;
};

  template <typename T>
class LazySequence {
public:
  LazySequence(int start, Supplier<T> generator) : start(start), gen(generator) {}

  LazyIterator<T> begin() {
    return LazyIterator<T>(start, gen);
  }

  LazyIterator<T> end() {
    return LazyIterator<T>(-1, gen); // End iterator with a sentinel value
  }

private:
  int start;
  Supplier<T> gen;
};
}
