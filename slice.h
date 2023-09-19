#pragma once

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <optional>

// A slice represents an immutable substring.
// Assumptions:
//      * the underlying string outlives the slice
//      * the underlying string is long enough
//      * the underlying string can be represented with single byte
//        characters (e.g. ASCII)
//
//      Slice representing "cde"
//          +---+---+
//          | o | 3 |
//          +-|-+---+
//            |
//            v
//       ...abcdefg...
//
// This class is intended as a light-weight wrapper around pointer and length
// and should be passed around by value.
//
class Slice {
public:
  char const *const start; // where does the string start in memory?
                           // The 2 uses of 'const' express different ideas:
                           //    * the first says that we can't change the
                           //      character we point to
                           //    * the second says that we can't change the
                           //      place we point to
  size_t const len;        // How many characters in the string

  // Constructor
  inline Slice(char const *const s, size_t const l) : start{s}, len{l} {}

  inline Slice(char const *const start, char const *const end)
      : start{start}, len{size_t(end - start)} {}

  inline bool operator==(char const *p) const {
    for (size_t i = 0; i < len; i++) {
      if (p[i] != start[i])
        return false;
    }
    return p[len] == 0;
  }

  bool operator==(const Slice &other) const {
    if (len != other.len)
      return false;
    for (size_t i = 0; i < len; i++) {
      if (other.start[i] != start[i])
        return false;
    }
    return true;
  }

  bool is_identifier() {
    if (len == 0)
      return 0;
    if (!isalpha(start[0]))
      return false;
    for (size_t i = 1; i < len; i++)
      if (!isalnum(start[i]))
        return false;
    return true;
  }

  void print() const {
    for (size_t i = 0; i < len; i++) {
      printf("%c", start[i]);
    }
  }
};

template <> struct std::hash<Slice> {
  std::size_t operator()(const Slice &key) const {
    // djb2
    size_t out = 5381;
    for (size_t i = 0; i < key.len; i++) {
      char const c = key.start[i];
      out = out * 33 + c;
    }
    return out;
  }
};

// vim: ts=4 sw=2 et
