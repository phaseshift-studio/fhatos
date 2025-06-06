//
//  peglib.h
//
//  Copyright (c) 2022 Yuji Hirose. All rights reserved.
//  MIT License
//

#pragma once

/*
 * Configuration
 */

#ifndef CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT
#define CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT 32
#endif

// to reduce memory footprint via -fno-rtti (see platformio.ini's unbuild-flags)
#ifndef FOS_CAST
#ifdef ESP32
#define FOS_CAST static_cast
#else
#define FOS_CAST dynamic_cast
#endif
#endif

#include <algorithm>
#include <any>
#include <cassert>
#include <cctype>
#if __has_include(<charconv>)
#include <charconv>
#endif
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#if !defined(__cplusplus) || __cplusplus < 201703L
#error "Requires complete C++17 support"
#endif

namespace peg {
  /*-----------------------------------------------------------------------------
   *  scope_exit
   *---------------------------------------------------------------------------*/

  // This is based on
  // "http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4189".

  template<typename EF>
  struct scope_exit {
    explicit scope_exit(EF &&f) : exit_function(std::move(f)), execute_on_destruction{true} {}

    scope_exit(scope_exit &&rhs) :
        exit_function(std::move(rhs.exit_function)), execute_on_destruction{rhs.execute_on_destruction} {
      rhs.release();
    }

    ~scope_exit() {
      if(execute_on_destruction) {
        this->exit_function();
      }
    }

    void release() { this->execute_on_destruction = false; }

  private:
    scope_exit(const scope_exit &) = delete;

    void operator=(const scope_exit &) = delete;

    scope_exit &operator=(scope_exit &&) = delete;

    EF exit_function;
    bool execute_on_destruction;
  };

  /*-----------------------------------------------------------------------------
   *  UTF8 functions
   *---------------------------------------------------------------------------*/

  inline size_t codepoint_length(const char *s8, size_t l) {
    if(l) {
      auto b = static_cast<uint8_t>(s8[0]);
      if((b & 0x80) == 0) {
        return 1;
      } else if((b & 0xE0) == 0xC0 && l >= 2) {
        return 2;
      } else if((b & 0xF0) == 0xE0 && l >= 3) {
        return 3;
      } else if((b & 0xF8) == 0xF0 && l >= 4) {
        return 4;
      }
    }
    return 0;
  }

  inline size_t codepoint_count(const char *s8, size_t l) {
    size_t count = 0;
    for(size_t i = 0; i < l; i += codepoint_length(s8 + i, l - i)) {
      count++;
    }
    return count;
  }

  inline size_t encode_codepoint(char32_t cp, char *buff) {
    if(cp < 0x0080) {
      buff[0] = static_cast<char>(cp & 0x7F);
      return 1;
    } else if(cp < 0x0800) {
      buff[0] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
      buff[1] = static_cast<char>(0x80 | (cp & 0x3F));
      return 2;
    } else if(cp < 0xD800) {
      buff[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0xF));
      buff[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      buff[2] = static_cast<char>(0x80 | (cp & 0x3F));
      return 3;
    } else if(cp < 0xE000) {
      // D800 - DFFF is invalid...
      return 0;
    } else if(cp < 0x10000) {
      buff[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0xF));
      buff[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      buff[2] = static_cast<char>(0x80 | (cp & 0x3F));
      return 3;
    } else if(cp < 0x110000) {
      buff[0] = static_cast<char>(0xF0 | ((cp >> 18) & 0x7));
      buff[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
      buff[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      buff[3] = static_cast<char>(0x80 | (cp & 0x3F));
      return 4;
    }
    return 0;
  }

  inline std::string encode_codepoint(char32_t cp) {
    char buff[4];
    auto l = encode_codepoint(cp, buff);
    return std::string(buff, l);
  }

  inline bool decode_codepoint(const char *s8, size_t l, size_t &bytes, char32_t &cp) {
    if(l) {
      auto b = static_cast<uint8_t>(s8[0]);
      if((b & 0x80) == 0) {
        bytes = 1;
        cp = b;
        return true;
      } else if((b & 0xE0) == 0xC0) {
        if(l >= 2) {
          bytes = 2;
          cp = ((static_cast<char32_t>(s8[0] & 0x1F)) << 6) | (static_cast<char32_t>(s8[1] & 0x3F));
          return true;
        }
      } else if((b & 0xF0) == 0xE0) {
        if(l >= 3) {
          bytes = 3;
          cp = ((static_cast<char32_t>(s8[0] & 0x0F)) << 12) | ((static_cast<char32_t>(s8[1] & 0x3F)) << 6) |
               (static_cast<char32_t>(s8[2] & 0x3F));
          return true;
        }
      } else if((b & 0xF8) == 0xF0) {
        if(l >= 4) {
          bytes = 4;
          cp = ((static_cast<char32_t>(s8[0] & 0x07)) << 18) | ((static_cast<char32_t>(s8[1] & 0x3F)) << 12) |
               ((static_cast<char32_t>(s8[2] & 0x3F)) << 6) | (static_cast<char32_t>(s8[3] & 0x3F));
          return true;
        }
      }
    }
    return false;
  }

  inline size_t decode_codepoint(const char *s8, size_t l, char32_t &cp) {
    size_t bytes;
    if(decode_codepoint(s8, l, bytes, cp)) {
      return bytes;
    }
    return 0;
  }

  inline char32_t decode_codepoint(const char *s8, size_t l) {
    char32_t cp = 0;
    decode_codepoint(s8, l, cp);
    return cp;
  }

  inline std::u32string decode(const char *s8, size_t l) {
    std::u32string out;
    size_t i = 0;
    while(i < l) {
      auto beg = i++;
      while(i < l && (s8[i] & 0xc0) == 0x80) {
        i++;
      }
      out += decode_codepoint(&s8[beg], (i - beg));
    }
    return out;
  }

  template<typename T>
  const char *u8(const T *s) {
    return reinterpret_cast<const char *>(s);
  }

  /*-----------------------------------------------------------------------------
   *  escape_characters
   *---------------------------------------------------------------------------*/

  inline std::string escape_characters(const char *s, size_t n) {
    std::string str;
    for(size_t i = 0; i < n; i++) {
      auto c = s[i];
      switch(c) {
        case '\f':
          str += "\\f";
          break;
        case '\n':
          str += "\\n";
          break;
        case '\r':
          str += "\\r";
          break;
        case '\t':
          str += "\\t";
          break;
        case '\v':
          str += "\\v";
          break;
        default:
          str += c;
          break;
      }
    }
    return str;
  }

  inline std::string escape_characters(std::string_view sv) { return escape_characters(sv.data(), sv.size()); }

  /*-----------------------------------------------------------------------------
   *  resolve_escape_sequence
   *---------------------------------------------------------------------------*/

  inline bool is_hex(char c, int &v) {
    if('0' <= c && c <= '9') {
      v = c - '0';
      return true;
    } else if('a' <= c && c <= 'f') {
      v = c - 'a' + 10;
      return true;
    } else if('A' <= c && c <= 'F') {
      v = c - 'A' + 10;
      return true;
    }
    return false;
  }

  inline bool is_digit(char c, int &v) {
    if('0' <= c && c <= '9') {
      v = c - '0';
      return true;
    }
    return false;
  }

  inline std::pair<int, size_t> parse_hex_number(const char *s, size_t n, size_t i) {
    int ret = 0;
    int val;
    while(i < n && is_hex(s[i], val)) {
      ret = static_cast<int>(ret * 16 + val);
      i++;
    }
    return std::pair(ret, i);
  }

  inline std::pair<int, size_t> parse_octal_number(const char *s, size_t n, size_t i) {
    int ret = 0;
    int val;
    while(i < n && is_digit(s[i], val)) {
      ret = static_cast<int>(ret * 8 + val);
      i++;
    }
    return std::pair(ret, i);
  }

  inline std::string resolve_escape_sequence(const char *s, size_t n) {
    std::string r;
    r.reserve(n);

    size_t i = 0;
    while(i < n) {
      auto ch = s[i];
      if(ch == '\\') {
        i++;
        if(i == n) {
          throw std::runtime_error("Invalid escape sequence...");
        }
        switch(s[i]) {
          case 'f':
            r += '\f';
            i++;
            break;
          case 'n':
            r += '\n';
            i++;
            break;
          case 'r':
            r += '\r';
            i++;
            break;
          case 't':
            r += '\t';
            i++;
            break;
          case 'v':
            r += '\v';
            i++;
            break;
          case '\'':
            r += '\'';
            i++;
            break;
          case '"':
            r += '"';
            i++;
            break;
          case '[':
            r += '[';
            i++;
            break;
          case ']':
            r += ']';
            i++;
            break;
          case '\\':
            r += '\\';
            i++;
            break;
          case 'x':
          case 'u': {
            char32_t cp;
            std::tie(cp, i) = parse_hex_number(s, n, i + 1);
            r += encode_codepoint(cp);
            break;
          }
          default: {
            char32_t cp;
            std::tie(cp, i) = parse_octal_number(s, n, i);
            r += encode_codepoint(cp);
            break;
          }
        }
      } else {
        r += ch;
        i++;
      }
    }
    return r;
  }

  /*-----------------------------------------------------------------------------
   *  token_to_number_ - This function should be removed eventually
   *---------------------------------------------------------------------------*/

  template<typename T>
  T token_to_number_(std::string_view sv) {
    T n = 0;
#if __has_include(<charconv>)
    if constexpr(!std::is_floating_point<T>::value) {
      std::from_chars(sv.data(), sv.data() + sv.size(), n);
#else
    if constexpr(false) {
#endif
    } else {
      auto s = std::string(sv);
      std::istringstream ss(s);
      ss >> n;
    }
    return n;
  }

  /*-----------------------------------------------------------------------------
   *  Trie
   *---------------------------------------------------------------------------*/

  class Trie {
  public:
    Trie(const std::vector<std::string> &items, bool ignore_case) : ignore_case_(ignore_case) {
      size_t id = 0;
      for(const auto &item: items) {
        const auto &s = ignore_case ? to_lower(item) : item;
        for(size_t len = 1; len <= item.size(); len++) {
          auto last = len == item.size();
          std::string_view sv(s.data(), len);
          auto it = dic_.find(sv);
          if(it == dic_.end()) {
            dic_.emplace(sv, Info{last, last, id});
          } else if(last) {
            it->second.match = true;
          } else {
            it->second.done = false;
          }
        }
        id++;
      }
    }

    size_t match(const char *text, size_t text_len, size_t &id) const {
      std::string lower_text;
      if(ignore_case_) {
        lower_text = to_lower(text);
        text = lower_text.data();
      }

      size_t match_len = 0;
      auto done = false;
      size_t len = 1;
      while(!done && len <= text_len) {
        std::string_view sv(text, len);
        auto it = dic_.find(sv);
        if(it == dic_.end()) {
          done = true;
        } else {
          if(it->second.match) {
            match_len = len;
            id = it->second.id;
          }
          if(it->second.done) {
            done = true;
          }
        }
        len += 1;
      }
      return match_len;
    }

    size_t size() const { return dic_.size(); }

  private:
    std::string to_lower(std::string s) const {
      for(char &c: s) {
        c = std::tolower(c);
      }
      return s;
    }

    struct Info {
      bool done;
      bool match;
      size_t id;
    };

    // TODO: Use unordered_map when heterogeneous lookup is supported in C++20
    // std::unordered_map<std::string, Info> dic_;
    std::map<std::string, Info, std::less<>> dic_;

    bool ignore_case_;
  };

  /*-----------------------------------------------------------------------------
   *  PEG
   *---------------------------------------------------------------------------*/

  /*
   * Line information utility function
   */
  inline std::pair<size_t, size_t> line_info(const char *start, const char *cur) {
    auto p = start;
    auto col_ptr = p;
    auto no = 1;

    while(p < cur) {
      if(*p == '\n') {
        no++;
        col_ptr = p + 1;
      }
      p++;
    }

    auto col = codepoint_count(col_ptr, p - col_ptr) + 1;

    return std::pair(no, col);
  }

  /*
   * String tag
   */
  inline constexpr unsigned int str2tag_core(const char *s, size_t l, unsigned int h) {
    return (l == 0) ? h : str2tag_core(s + 1, l - 1, (h * 33) ^ static_cast<unsigned char>(*s));
  }

  inline constexpr unsigned int str2tag(std::string_view sv) { return str2tag_core(sv.data(), sv.size(), 0); }

  namespace udl {
    inline constexpr unsigned int operator"" _(const char *s, size_t l) { return str2tag_core(s, l, 0); }
  } // namespace udl

  /*
   * Semantic values
   */
  class Context;

  struct SemanticValues : protected std::vector<std::any> {
    SemanticValues() = default;

    SemanticValues(Context *c) : c_(c) {}

    // Input text
    const char *path = nullptr;
    const char *ss = nullptr;

    // Matched string
    std::string_view sv() const { return sv_; }

    // Definition name
    const std::string &name() const { return name_; }

    std::vector<unsigned int> tags;

    // Line number and column at which the matched string is
    std::pair<size_t, size_t> line_info() const;

    // Choice count
    size_t choice_count() const { return choice_count_; }

    // Choice number (0 based index)
    size_t choice() const { return choice_; }

    // Tokens
    std::vector<std::string_view> tokens;

    std::string_view token(size_t id = 0) const {
      if(tokens.empty()) {
        return sv_;
      }
      assert(id < tokens.size());
      return tokens[id];
    }

    // Token conversion
    std::string token_to_string(size_t id = 0) const { return std::string(token(id)); }

    template<typename T>
    T token_to_number() const {
      return token_to_number_<T>(token());
    }

    // Transform the semantic value vector to another vector
    template<typename T>
    std::vector<T> transform(size_t beg = 0, size_t end = static_cast<size_t>(-1)) const {
      std::vector<T> r;
      end = (std::min) (end, size());
      for(size_t i = beg; i < end; i++) {
        r.emplace_back(std::any_cast<T>((*this)[i]));
      }
      return r;
    }

    void append(SemanticValues &chvs) {
      sv_ = chvs.sv_;
      for(auto &v: chvs) {
        emplace_back(std::move(v));
      }
      for(auto &tag: chvs.tags) {
        tags.emplace_back(std::move(tag));
      }
      for(auto &tok: chvs.tokens) {
        tokens.emplace_back(std::move(tok));
      }
    }

    using std::vector<std::any>::iterator;
    using std::vector<std::any>::const_iterator;
    using std::vector<std::any>::size;
    using std::vector<std::any>::empty;
    using std::vector<std::any>::assign;
    using std::vector<std::any>::begin;
    using std::vector<std::any>::end;
    using std::vector<std::any>::rbegin;
    using std::vector<std::any>::rend;
    using std::vector<std::any>::operator[];
    using std::vector<std::any>::at;
    using std::vector<std::any>::resize;
    using std::vector<std::any>::front;
    using std::vector<std::any>::back;
    using std::vector<std::any>::push_back;
    using std::vector<std::any>::pop_back;
    using std::vector<std::any>::insert;
    using std::vector<std::any>::erase;
    using std::vector<std::any>::clear;
    using std::vector<std::any>::swap;
    using std::vector<std::any>::emplace;
    using std::vector<std::any>::emplace_back;

  private:
    friend class Context;
    friend class Dictionary;
    friend class Sequence;
    friend class PrioritizedChoice;
    friend class Repetition;
    friend class Holder;
    friend class PrecedenceClimbing;

    Context *c_ = nullptr;
    std::string_view sv_;
    size_t choice_count_ = 0;
    size_t choice_ = 0;
    std::string name_;
  };

  /*
   * Semantic action
   */
  template<typename F, typename... Args>
  std::any call(F fn, Args &&...args) {
    using R = decltype(fn(std::forward<Args>(args)...));
    if constexpr(std::is_void_v<R>) {
      fn(std::forward<Args>(args)...);
      return std::any();
    } else if constexpr(std::is_same_v<std::remove_cv_t<R>, std::any>) {
      return fn(std::forward<Args>(args)...);
    } else {
      return std::any(fn(std::forward<Args>(args)...));
    }
  }

  template<typename T>
  struct argument_count : argument_count<decltype(&T::operator())> {};

  template<typename R, typename... Args>
  struct argument_count<R (*)(Args...)> : std::integral_constant<unsigned, sizeof...(Args)> {};

  template<typename R, typename C, typename... Args>
  struct argument_count<R (C::*)(Args...)> : std::integral_constant<unsigned, sizeof...(Args)> {};

  template<typename R, typename C, typename... Args>
  struct argument_count<R (C::*)(Args...) const> : std::integral_constant<unsigned, sizeof...(Args)> {};

  class Action {
  public:
    Action() = default;

    Action(Action &&rhs) = default;

    template<typename F>
    Action(F fn) : fn_(make_adaptor(fn)) {}

    template<typename F>
    void operator=(F fn) {
      fn_ = make_adaptor(fn);
    }

    Action &operator=(const Action &rhs) = default;

    operator bool() const { return bool(fn_); }

    std::any operator()(SemanticValues &vs, std::any &dt) const { return fn_(vs, dt); }

  private:
    using Fty = std::function<std::any(SemanticValues &vs, std::any &dt)>;

    template<typename F>
    Fty make_adaptor(F fn) {
      if constexpr(argument_count<F>::value == 1) {
        return [fn](auto &vs, auto & /*dt*/) { return call(fn, vs); };
      } else {
        return [fn](auto &vs, auto &dt) { return call(fn, vs, dt); };
      }
    }

    Fty fn_;
  };

  /*
   * Parse result helper
   */
  inline bool success(const size_t len) { return len != static_cast<size_t>(-1); }

  inline bool fail(const size_t len) { return len == static_cast<size_t>(-1); }

  /*
   * Log
   */
  using Log = std::function<void(size_t line, size_t col, const std::string &msg, const std::string &rule)>;

  /*
   * ErrorInfo
   */
  class Definition;

  struct ErrorInfo {
    const char *error_pos = nullptr;
    std::vector<std::pair<const char *, const Definition *>> expected_tokens;
    const char *message_pos = nullptr;
    std::string message;
    std::string label;
    const char *last_output_pos = nullptr;
    bool keep_previous_token = false;

    void clear() {
      error_pos = nullptr;
      expected_tokens.clear();
      message_pos = nullptr;
      message.clear();
    }

    void add(const char *error_literal, const Definition *error_rule) {
      for(const auto &[t, r]: expected_tokens) {
        if(t == error_literal && r == error_rule) {
          return;
        }
      }
      expected_tokens.emplace_back(error_literal, error_rule);
    }

    void output_log(const Log &log, const char *s, size_t n);

  private:
    int cast_char(const char c) const { return static_cast<unsigned char>(c); }

    std::string heuristic_error_token(const char *s, size_t n, const char *pos) const {
      auto len = n - std::distance(s, pos);
      if(len) {
        size_t i = 0;
        auto c = cast_char(pos[i++]);
        if(!std::ispunct(c) && !std::isspace(c)) {
          while(i < len && !std::ispunct(cast_char(pos[i])) && !std::isspace(cast_char(pos[i]))) {
            i++;
          }
        }

        size_t count = CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT;
        size_t j = 0;
        while(count > 0 && j < i) {
          j += codepoint_length(&pos[j], i - j);
          count--;
        }

        return escape_characters(pos, j);
      }
      return std::string();
    }

    std::string replace_all(std::string str, const std::string &from, const std::string &to) const {
      size_t pos = 0;
      while((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
      }
      return str;
    }
  };

  /*
   * Context
   */
  class Ope;

  using TracerEnter = std::function<void(const Ope &name, const char *s, size_t n, const SemanticValues &vs,
                                         const Context &c, const std::any &dt, std::any &trace_data)>;

  using TracerLeave = std::function<void(const Ope &ope, const char *s, size_t n, const SemanticValues &vs,
                                         const Context &c, const std::any &dt, size_t, std::any &trace_data)>;

  using TracerStartOrEnd = std::function<void(std::any &trace_data)>;

  class Context {
  public:
    const char *path;
    const char *s;
    const size_t l;

    ErrorInfo error_info;
    bool recovered = false;

    std::vector<std::shared_ptr<SemanticValues>> value_stack;
    size_t value_stack_size = 0;

    std::vector<Definition *> rule_stack;
    std::vector<std::vector<std::shared_ptr<Ope>>> args_stack;

    size_t in_token_boundary_count = 0;

    std::shared_ptr<Ope> whitespaceOpe;
    bool in_whitespace = false;

    std::shared_ptr<Ope> wordOpe;

    std::vector<std::map<std::string_view, std::string>> capture_scope_stack;
    size_t capture_scope_stack_size = 0;

    std::vector<bool> cut_stack;

    const size_t def_count;
    const bool enablePackratParsing;
    std::vector<bool> cache_registered;
    std::vector<bool> cache_success;

    std::map<std::pair<size_t, size_t>, std::tuple<size_t, std::any>> cache_values;

    TracerEnter tracer_enter;
    TracerLeave tracer_leave;
    std::any trace_data;
    const bool verbose_trace;

    Log log;

    Context(const char *path, const char *s, size_t l, size_t def_count, std::shared_ptr<Ope> whitespaceOpe,
            std::shared_ptr<Ope> wordOpe, bool enablePackratParsing, TracerEnter tracer_enter, TracerLeave tracer_leave,
            std::any trace_data, bool verbose_trace, Log log) :
        path(path), s(s), l(l), whitespaceOpe(whitespaceOpe), wordOpe(wordOpe), def_count(def_count),
        enablePackratParsing(enablePackratParsing), cache_registered(enablePackratParsing ? def_count * (l + 1) : 0),
        cache_success(enablePackratParsing ? def_count * (l + 1) : 0), tracer_enter(tracer_enter),
        tracer_leave(tracer_leave), trace_data(trace_data), verbose_trace(verbose_trace), log(log) {
      push_args({});
      push_capture_scope();
    }

    ~Context() {
      pop_capture_scope();

      assert(!value_stack_size);
      assert(!capture_scope_stack_size);
      assert(cut_stack.empty());
    }

    Context(const Context &) = delete;

    Context(Context &&) = delete;

    Context operator=(const Context &) = delete;

    template<typename T>
    void packrat(const char *a_s, size_t def_id, size_t &len, std::any &val, T fn) {
      if(!enablePackratParsing) {
        fn(val);
        return;
      }

      auto col = a_s - s;
      auto idx = def_count * static_cast<size_t>(col) + def_id;

      if(cache_registered[idx]) {
        if(cache_success[idx]) {
          const auto key = std::pair(col, def_id);
          std::tie(len, val) = cache_values[key];
          return;
        } else {
          len = static_cast<size_t>(-1);
          return;
        }
      } else {
        fn(val);
        cache_registered[idx] = true;
        cache_success[idx] = success(len);
        if(success(len)) {
          const auto key = std::pair(col, def_id);
          cache_values[key] = std::pair(len, val);
        }
        return;
      }
    }

    SemanticValues &push() {
      push_capture_scope();
      return push_semantic_values_scope();
    }

    void pop() {
      pop_capture_scope();
      pop_semantic_values_scope();
    }

    // Semantic values
    SemanticValues &push_semantic_values_scope() {
      assert(value_stack_size <= value_stack.size());
      if(value_stack_size == value_stack.size()) {
        value_stack.emplace_back(std::make_shared<SemanticValues>(this));
      } else {
        auto &vs = *value_stack[value_stack_size];
        if(!vs.empty()) {
          vs.clear();
          if(!vs.tags.empty()) {
            vs.tags.clear();
          }
        }
        vs.sv_ = std::string_view();
        vs.choice_count_ = 0;
        vs.choice_ = 0;
        if(!vs.tokens.empty()) {
          vs.tokens.clear();
        }
      }

      auto &vs = *value_stack[value_stack_size++];
      vs.path = path;
      vs.ss = s;
      return vs;
    }

    void pop_semantic_values_scope() { value_stack_size--; }

    // Arguments
    void push_args(std::vector<std::shared_ptr<Ope>> &&args) { args_stack.emplace_back(args); }

    void pop_args() { args_stack.pop_back(); }

    const std::vector<std::shared_ptr<Ope>> &top_args() const { return args_stack[args_stack.size() - 1]; }

    // Capture scope
    void push_capture_scope() {
      assert(capture_scope_stack_size <= capture_scope_stack.size());
      if(capture_scope_stack_size == capture_scope_stack.size()) {
        capture_scope_stack.emplace_back(std::map<std::string_view, std::string>());
      } else {
        auto &cs = capture_scope_stack[capture_scope_stack_size];
        if(!cs.empty()) {
          cs.clear();
        }
      }
      capture_scope_stack_size++;
    }

    void pop_capture_scope() { capture_scope_stack_size--; }

    void shift_capture_values() {
      assert(capture_scope_stack_size >= 2);
      auto curr = &capture_scope_stack[capture_scope_stack_size - 1];
      auto prev = curr - 1;
      for(const auto &[k, v]: *curr) {
        (*prev)[k] = v;
      }
    }

    // Error
    void set_error_pos(const char *a_s, const char *literal = nullptr);

    // Trace
    void trace_enter(const Ope &ope, const char *a_s, size_t n, const SemanticValues &vs, std::any &dt);

    void trace_leave(const Ope &ope, const char *a_s, size_t n, const SemanticValues &vs, std::any &dt, size_t len);

    bool is_traceable(const Ope &ope) const;

    // Line info
    std::pair<size_t, size_t> line_info(const char *cur) const {
      std::call_once(source_line_index_init_, [this]() {
        for(size_t pos = 0; pos < l; pos++) {
          if(s[pos] == '\n') {
            source_line_index.push_back(pos);
          }
        }
        source_line_index.push_back(l);
      });

      const auto pos = static_cast<size_t>(std::distance(s, cur));

      const auto it = std::lower_bound(source_line_index.begin(), source_line_index.end(), pos,
                                       [](size_t element, size_t value) { return element < value; });

      const auto id = static_cast<size_t>(std::distance(source_line_index.begin(), it));
      const auto off = pos - (id == 0 ? 0 : source_line_index[id - 1] + 1);
      return std::pair(id + 1, off + 1);
    }

    size_t next_trace_id = 0;
    std::vector<size_t> trace_ids;
    bool ignore_trace_state = false;
    mutable std::once_flag source_line_index_init_;
    mutable std::vector<size_t> source_line_index;
  };

  /*
   * Parser operators
   */
  class Ope {
  public:
    struct Visitor;

    virtual ~Ope() = default;

    size_t parse(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const;

    virtual size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const = 0;

    virtual void accept(Visitor &v) = 0;
  };

  class Sequence : public Ope {
  public:
    template<typename... Args>
    Sequence(const Args &...args) : opes_{static_cast<std::shared_ptr<Ope>>(args)...} {}

    Sequence(const std::vector<std::shared_ptr<Ope>> &opes) : opes_(opes) {}

    Sequence(std::vector<std::shared_ptr<Ope>> &&opes) : opes_(opes) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      auto &chvs = c.push_semantic_values_scope();
      auto se = scope_exit([&]() { c.pop_semantic_values_scope(); });
      size_t i = 0;
      for(const auto &ope: opes_) {
        auto len = ope->parse(s + i, n - i, chvs, c, dt);
        if(fail(len)) {
          return len;
        }
        i += len;
      }
      vs.append(chvs);
      return i;
    }

    void accept(Visitor &v) override;

    std::vector<std::shared_ptr<Ope>> opes_;
  };

  class PrioritizedChoice : public Ope {
  public:
    template<typename... Args>
    PrioritizedChoice(bool for_label, const Args &...args) :
        opes_{static_cast<std::shared_ptr<Ope>>(args)...}, for_label_(for_label) {}

    PrioritizedChoice(const std::vector<std::shared_ptr<Ope>> &opes) : opes_(opes) {}

    PrioritizedChoice(std::vector<std::shared_ptr<Ope>> &&opes) : opes_(opes) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      size_t len = static_cast<size_t>(-1);

      if(!for_label_) {
        c.cut_stack.push_back(false);
      }
      auto se = scope_exit([&]() {
        if(!for_label_) {
          c.cut_stack.pop_back();
        }
      });

      size_t id = 0;
      for(const auto &ope: opes_) {
        if(!c.cut_stack.empty()) {
          c.cut_stack.back() = false;
        }

        auto &chvs = c.push();
        c.error_info.keep_previous_token = id > 0;
        auto se = scope_exit([&]() {
          c.pop();
          c.error_info.keep_previous_token = false;
        });

        len = ope->parse(s, n, chvs, c, dt);

        if(success(len)) {
          vs.append(chvs);
          vs.choice_count_ = opes_.size();
          vs.choice_ = id;
          c.shift_capture_values();
          break;
        } else if(!c.cut_stack.empty() && c.cut_stack.back()) {
          break;
        }

        id++;
      }

      return len;
    }

    void accept(Visitor &v) override;

    size_t size() const { return opes_.size(); }

    std::vector<std::shared_ptr<Ope>> opes_;
    bool for_label_ = false;
  };

  class Repetition : public Ope {
  public:
    Repetition(const std::shared_ptr<Ope> &ope, size_t min, size_t max) : ope_(ope), min_(min), max_(max) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      size_t count = 0;
      size_t i = 0;
      while(count < min_) {
        auto &chvs = c.push();
        auto se = scope_exit([&]() { c.pop(); });

        auto len = ope_->parse(s + i, n - i, chvs, c, dt);

        if(success(len)) {
          vs.append(chvs);
          c.shift_capture_values();
        } else {
          return len;
        }
        i += len;
        count++;
      }

      while(count < max_) {
        auto &chvs = c.push();
        auto se = scope_exit([&]() { c.pop(); });

        auto len = ope_->parse(s + i, n - i, chvs, c, dt);

        if(success(len)) {
          vs.append(chvs);
          c.shift_capture_values();
        } else {
          break;
        }
        i += len;
        count++;
      }
      return i;
    }

    void accept(Visitor &v) override;

    bool is_zom() const { return min_ == 0 && max_ == std::numeric_limits<size_t>::max(); }

    static std::shared_ptr<Repetition> zom(const std::shared_ptr<Ope> &ope) {
      return std::make_shared<Repetition>(ope, 0, std::numeric_limits<size_t>::max());
    }

    static std::shared_ptr<Repetition> oom(const std::shared_ptr<Ope> &ope) {
      return std::make_shared<Repetition>(ope, 1, std::numeric_limits<size_t>::max());
    }

    static std::shared_ptr<Repetition> opt(const std::shared_ptr<Ope> &ope) {
      return std::make_shared<Repetition>(ope, 0, 1);
    }

    std::shared_ptr<Ope> ope_;
    size_t min_;
    size_t max_;
  };

  class AndPredicate : public Ope {
  public:
    AndPredicate(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/, Context &c, std::any &dt) const override {
      auto &chvs = c.push();
      auto se = scope_exit([&]() { c.pop(); });

      auto len = ope_->parse(s, n, chvs, c, dt);

      if(success(len)) {
        return 0;
      } else {
        return len;
      }
    }

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
  };

  class NotPredicate : public Ope {
  public:
    NotPredicate(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/, Context &c, std::any &dt) const override {
      auto &chvs = c.push();
      auto se = scope_exit([&]() { c.pop(); });
      auto len = ope_->parse(s, n, chvs, c, dt);
      if(success(len)) {
        c.set_error_pos(s);
        return static_cast<size_t>(-1);
      } else {
        return 0;
      }
    }

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
  };

  class Dictionary : public Ope, public std::enable_shared_from_this<Dictionary> {
  public:
    Dictionary(const std::vector<std::string> &v, bool ignore_case) : trie_(v, ignore_case) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override;

    void accept(Visitor &v) override;

    Trie trie_;
  };

  class LiteralString : public Ope, public std::enable_shared_from_this<LiteralString> {
  public:
    LiteralString(std::string &&s, bool ignore_case) : lit_(s), ignore_case_(ignore_case), is_word_(false) {}

    LiteralString(const std::string &s, bool ignore_case) : lit_(s), ignore_case_(ignore_case), is_word_(false) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override;

    void accept(Visitor &v) override;

    std::string lit_;
    bool ignore_case_;
    mutable std::once_flag init_is_word_;
    mutable bool is_word_;
  };

  class CharacterClass : public Ope, public std::enable_shared_from_this<CharacterClass> {
  public:
    CharacterClass(const std::string &s, bool negated, bool ignore_case) :
        negated_(negated), ignore_case_(ignore_case) {
      auto chars = decode(s.data(), s.length());
      auto i = 0u;
      while(i < chars.size()) {
        if(i + 2 < chars.size() && chars[i + 1] == '-') {
          auto cp1 = chars[i];
          auto cp2 = chars[i + 2];
          ranges_.emplace_back(std::pair(cp1, cp2));
          i += 3;
        } else {
          auto cp = chars[i];
          ranges_.emplace_back(std::pair(cp, cp));
          i += 1;
        }
      }
      assert(!ranges_.empty());
    }

    CharacterClass(const std::vector<std::pair<char32_t, char32_t>> &ranges, bool negated, bool ignore_case) :
        ranges_(ranges), negated_(negated), ignore_case_(ignore_case) {
      assert(!ranges_.empty());
    }

    size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/, Context &c, std::any & /*dt*/) const override {
      if(n < 1) {
        c.set_error_pos(s);
        return static_cast<size_t>(-1);
      }

      char32_t cp = 0;
      auto len = decode_codepoint(s, n, cp);

      for(const auto &range: ranges_) {
        if(in_range(range, cp)) {
          if(negated_) {
            c.set_error_pos(s);
            return static_cast<size_t>(-1);
          } else {
            return len;
          }
        }
      }

      if(negated_) {
        return len;
      } else {
        c.set_error_pos(s);
        return static_cast<size_t>(-1);
      }
    }

    void accept(Visitor &v) override;

  private:
    bool in_range(const std::pair<char32_t, char32_t> &range, char32_t cp) const {
      if(ignore_case_) {
        auto cpl = std::tolower(cp);
        return std::tolower(range.first) <= cpl && cpl <= std::tolower(range.second);
      } else {
        return range.first <= cp && cp <= range.second;
      }
    }

    std::vector<std::pair<char32_t, char32_t>> ranges_;
    bool negated_;
    bool ignore_case_;
  };

  class Character : public Ope, public std::enable_shared_from_this<Character> {
  public:
    Character(char ch) : ch_(ch) {}

    size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/, Context &c, std::any & /*dt*/) const override {
      if(n < 1 || s[0] != ch_) {
        c.set_error_pos(s);
        return static_cast<size_t>(-1);
      }
      return 1;
    }

    void accept(Visitor &v) override;

    char ch_;
  };

  class AnyCharacter : public Ope, public std::enable_shared_from_this<AnyCharacter> {
  public:
    size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/, Context &c, std::any & /*dt*/) const override {
      auto len = codepoint_length(s, n);
      if(len < 1) {
        c.set_error_pos(s);
        return static_cast<size_t>(-1);
      }
      return len;
    }

    void accept(Visitor &v) override;
  };

  class CaptureScope : public Ope {
  public:
    CaptureScope(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      c.push_capture_scope();
      auto se = scope_exit([&]() { c.pop_capture_scope(); });
      return ope_->parse(s, n, vs, c, dt);
    }

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
  };

  class Capture : public Ope {
  public:
    using MatchAction = std::function<void(const char *s, size_t n, Context &c)>;

    Capture(const std::shared_ptr<Ope> &ope, MatchAction ma) : ope_(ope), match_action_(ma) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      auto len = ope_->parse(s, n, vs, c, dt);
      if(success(len) && match_action_) {
        match_action_(s, len, c);
      }
      return len;
    }

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
    MatchAction match_action_;
  };

  class TokenBoundary : public Ope {
  public:
    TokenBoundary(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override;

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
  };

  class Ignore : public Ope {
  public:
    Ignore(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/, Context &c, std::any &dt) const override {
      auto &chvs = c.push_semantic_values_scope();
      auto se = scope_exit([&]() { c.pop_semantic_values_scope(); });
      return ope_->parse(s, n, chvs, c, dt);
    }

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
  };

  using Parser = std::function<size_t(const char *s, size_t n, SemanticValues &vs, std::any &dt)>;

  class User : public Ope {
  public:
    User(Parser fn) : fn_(fn) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context & /*c*/, std::any &dt) const override {
      assert(fn_);
      return fn_(s, n, vs, dt);
    }

    void accept(Visitor &v) override;

    std::function<size_t(const char *s, size_t n, SemanticValues &vs, std::any &dt)> fn_;
  };

  class WeakHolder : public Ope {
  public:
    WeakHolder(const std::shared_ptr<Ope> &ope) : weak_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      auto ope = weak_.lock();
      assert(ope);
      return ope->parse(s, n, vs, c, dt);
    }

    void accept(Visitor &v) override;

    std::weak_ptr<Ope> weak_;
  };

  class Holder : public Ope {
  public:
    Holder(Definition *outer) : outer_(outer) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override;

    void accept(Visitor &v) override;

    std::any reduce(SemanticValues &vs, std::any &dt) const;

    const std::string &name() const;

    const std::string &trace_name() const;

    std::shared_ptr<Ope> ope_;
    Definition *outer_;
    mutable std::once_flag trace_name_init_;
    mutable std::string trace_name_;

    friend class Definition;
  };

  using Grammar = std::unordered_map<std::string, Definition>;

  class Reference : public Ope, public std::enable_shared_from_this<Reference> {
  public:
    Reference(const Grammar &grammar, const std::string &name, const char *s, bool is_macro,
              const std::vector<std::shared_ptr<Ope>> &args) :
        grammar_(grammar), name_(name), s_(s), is_macro_(is_macro), args_(args), rule_(nullptr), iarg_(0) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override;

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> get_core_operator() const;

    const Grammar &grammar_;
    const std::string name_;
    const char *s_;

    const bool is_macro_;
    const std::vector<std::shared_ptr<Ope>> args_;

    Definition *rule_;
    size_t iarg_;
  };

  class Whitespace : public Ope {
  public:
    Whitespace(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      if(c.in_whitespace) {
        return 0;
      }
      c.in_whitespace = true;
      auto se = scope_exit([&]() { c.in_whitespace = false; });
      return ope_->parse(s, n, vs, c, dt);
    }

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
  };

  class BackReference : public Ope {
  public:
    BackReference(std::string &&name) : name_(name) {}

    BackReference(const std::string &name) : name_(name) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override;

    void accept(Visitor &v) override;

    std::string name_;
  };

  class PrecedenceClimbing : public Ope {
  public:
    using BinOpeInfo = std::map<std::string_view, std::pair<size_t, char>>;

    PrecedenceClimbing(const std::shared_ptr<Ope> &atom, const std::shared_ptr<Ope> &binop, const BinOpeInfo &info,
                       const Definition &rule) : atom_(atom), binop_(binop), info_(info), rule_(rule) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override {
      return parse_expression(s, n, vs, c, dt, 0);
    }

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> atom_;
    std::shared_ptr<Ope> binop_;
    BinOpeInfo info_;
    const Definition &rule_;

  private:
    size_t parse_expression(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt,
                            size_t min_prec) const;

    Definition &get_reference_for_binop(Context &c) const;
  };

  class Recovery : public Ope {
  public:
    Recovery(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

    size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const override;

    void accept(Visitor &v) override;

    std::shared_ptr<Ope> ope_;
  };

  class Cut : public Ope, public std::enable_shared_from_this<Cut> {
  public:
    size_t parse_core(const char * /*s*/, size_t /*n*/, SemanticValues & /*vs*/, Context &c,
                      std::any & /*dt*/) const override {
      if(!c.cut_stack.empty()) {
        c.cut_stack.back() = true;
      }
      return 0;
    }

    void accept(Visitor &v) override;
  };

  /*
   * Factories
   */
  template<typename... Args>
  std::shared_ptr<Ope> seq(Args &&...args) {
    return std::make_shared<Sequence>(static_cast<std::shared_ptr<Ope>>(args)...);
  }

  template<typename... Args>
  std::shared_ptr<Ope> cho(Args &&...args) {
    return std::make_shared<PrioritizedChoice>(false, static_cast<std::shared_ptr<Ope>>(args)...);
  }

  template<typename... Args>
  std::shared_ptr<Ope> cho4label_(Args &&...args) {
    return std::make_shared<PrioritizedChoice>(true, static_cast<std::shared_ptr<Ope>>(args)...);
  }

  inline std::shared_ptr<Ope> zom(const std::shared_ptr<Ope> &ope) { return Repetition::zom(ope); }

  inline std::shared_ptr<Ope> oom(const std::shared_ptr<Ope> &ope) { return Repetition::oom(ope); }

  inline std::shared_ptr<Ope> opt(const std::shared_ptr<Ope> &ope) { return Repetition::opt(ope); }

  inline std::shared_ptr<Ope> rep(const std::shared_ptr<Ope> &ope, size_t min, size_t max) {
    return std::make_shared<Repetition>(ope, min, max);
  }

  inline std::shared_ptr<Ope> apd(const std::shared_ptr<Ope> &ope) { return std::make_shared<AndPredicate>(ope); }

  inline std::shared_ptr<Ope> npd(const std::shared_ptr<Ope> &ope) { return std::make_shared<NotPredicate>(ope); }

  inline std::shared_ptr<Ope> dic(const std::vector<std::string> &v, bool ignore_case) {
    return std::make_shared<Dictionary>(v, ignore_case);
  }

  inline std::shared_ptr<Ope> lit(std::string &&s) { return std::make_shared<LiteralString>(s, false); }

  inline std::shared_ptr<Ope> liti(std::string &&s) { return std::make_shared<LiteralString>(s, true); }

  inline std::shared_ptr<Ope> cls(const std::string &s) { return std::make_shared<CharacterClass>(s, false, false); }

  inline std::shared_ptr<Ope> cls(const std::vector<std::pair<char32_t, char32_t>> &ranges, bool ignore_case = false) {
    return std::make_shared<CharacterClass>(ranges, false, ignore_case);
  }

  inline std::shared_ptr<Ope> ncls(const std::string &s) { return std::make_shared<CharacterClass>(s, true, false); }

  inline std::shared_ptr<Ope> ncls(const std::vector<std::pair<char32_t, char32_t>> &ranges, bool ignore_case = false) {
    return std::make_shared<CharacterClass>(ranges, true, ignore_case);
  }

  inline std::shared_ptr<Ope> chr(char dt) { return std::make_shared<Character>(dt); }

  inline std::shared_ptr<Ope> dot() { return std::make_shared<AnyCharacter>(); }

  inline std::shared_ptr<Ope> csc(const std::shared_ptr<Ope> &ope) { return std::make_shared<CaptureScope>(ope); }

  inline std::shared_ptr<Ope> cap(const std::shared_ptr<Ope> &ope, Capture::MatchAction ma) {
    return std::make_shared<Capture>(ope, ma);
  }

  inline std::shared_ptr<Ope> tok(const std::shared_ptr<Ope> &ope) { return std::make_shared<TokenBoundary>(ope); }

  inline std::shared_ptr<Ope> ign(const std::shared_ptr<Ope> &ope) { return std::make_shared<Ignore>(ope); }

  inline std::shared_ptr<Ope> usr(std::function<size_t(const char *s, size_t n, SemanticValues &vs, std::any &dt)> fn) {
    return std::make_shared<User>(fn);
  }

  inline std::shared_ptr<Ope> ref(const Grammar &grammar, const std::string &name, const char *s, bool is_macro,
                                  const std::vector<std::shared_ptr<Ope>> &args) {
    return std::make_shared<Reference>(grammar, name, s, is_macro, args);
  }

  inline std::shared_ptr<Ope> wsp(const std::shared_ptr<Ope> &ope) {
    return std::make_shared<Whitespace>(std::make_shared<Ignore>(ope));
  }

  inline std::shared_ptr<Ope> bkr(std::string &&name) { return std::make_shared<BackReference>(name); }

  inline std::shared_ptr<Ope> pre(const std::shared_ptr<Ope> &atom, const std::shared_ptr<Ope> &binop,
                                  const PrecedenceClimbing::BinOpeInfo &info, const Definition &rule) {
    return std::make_shared<PrecedenceClimbing>(atom, binop, info, rule);
  }

  inline std::shared_ptr<Ope> rec(const std::shared_ptr<Ope> &ope) { return std::make_shared<Recovery>(ope); }

  inline std::shared_ptr<Ope> cut() { return std::make_shared<Cut>(); }

  /*
   * Visitor
   */
  struct Ope::Visitor {
    virtual ~Visitor() {}

    virtual void visit(Sequence &) {}

    virtual void visit(PrioritizedChoice &) {}

    virtual void visit(Repetition &) {}

    virtual void visit(AndPredicate &) {}

    virtual void visit(NotPredicate &) {}

    virtual void visit(Dictionary &) {}

    virtual void visit(LiteralString &) {}

    virtual void visit(CharacterClass &) {}

    virtual void visit(Character &) {}

    virtual void visit(AnyCharacter &) {}

    virtual void visit(CaptureScope &) {}

    virtual void visit(Capture &) {}

    virtual void visit(TokenBoundary &) {}

    virtual void visit(Ignore &) {}

    virtual void visit(User &) {}

    virtual void visit(WeakHolder &) {}

    virtual void visit(Holder &) {}

    virtual void visit(Reference &) {}

    virtual void visit(Whitespace &) {}

    virtual void visit(BackReference &) {}

    virtual void visit(PrecedenceClimbing &) {}

    virtual void visit(Recovery &) {}

    virtual void visit(Cut &) {}
  };

  struct TraceOpeName : public Ope::Visitor {
    using Ope::Visitor::visit;

    void visit(Sequence &) override { name_ = "Sequence"; }
    void visit(PrioritizedChoice &) override { name_ = "PrioritizedChoice"; }
    void visit(Repetition &) override { name_ = "Repetition"; }
    void visit(AndPredicate &) override { name_ = "AndPredicate"; }
    void visit(NotPredicate &) override { name_ = "NotPredicate"; }
    void visit(Dictionary &) override { name_ = "Dictionary"; }
    void visit(LiteralString &) override { name_ = "LiteralString"; }
    void visit(CharacterClass &) override { name_ = "CharacterClass"; }
    void visit(Character &) override { name_ = "Character"; }
    void visit(AnyCharacter &) override { name_ = "AnyCharacter"; }
    void visit(CaptureScope &) override { name_ = "CaptureScope"; }
    void visit(Capture &) override { name_ = "Capture"; }
    void visit(TokenBoundary &) override { name_ = "TokenBoundary"; }
    void visit(Ignore &) override { name_ = "Ignore"; }
    void visit(User &) override { name_ = "User"; }
    void visit(WeakHolder &) override { name_ = "WeakHolder"; }
    void visit(Holder &ope) override { name_ = ope.trace_name().data(); }
    void visit(Reference &) override { name_ = "Reference"; }
    void visit(Whitespace &) override { name_ = "Whitespace"; }
    void visit(BackReference &) override { name_ = "BackReference"; }
    void visit(PrecedenceClimbing &) override { name_ = "PrecedenceClimbing"; }
    void visit(Recovery &) override { name_ = "Recovery"; }
    void visit(Cut &) override { name_ = "Cut"; }

    static std::string get(Ope &ope) {
      TraceOpeName vis;
      ope.accept(vis);
      return vis.name_;
    }

  private:
    const char *name_ = nullptr;
  };

  struct AssignIDToDefinition : public Ope::Visitor {
    using Ope::Visitor::visit;

    void visit(Sequence &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(PrioritizedChoice &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(Repetition &ope) override { ope.ope_->accept(*this); }
    void visit(AndPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(NotPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
    void visit(Capture &ope) override { ope.ope_->accept(*this); }
    void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }
    void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }

    void visit(Holder &ope) override;

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override { ope.ope_->accept(*this); }

    void visit(PrecedenceClimbing &ope) override;

    void visit(Recovery &ope) override { ope.ope_->accept(*this); }

    std::unordered_map<void *, size_t> ids;
  };

  struct IsLiteralToken : public Ope::Visitor {
    using Ope::Visitor::visit;

    void visit(PrioritizedChoice &ope) override {
      for(auto op: ope.opes_) {
        if(!IsLiteralToken::check(*op)) {
          return;
        }
      }
      result_ = true;
    }

    void visit(Dictionary &) override { result_ = true; }
    void visit(LiteralString &) override { result_ = true; }

    static bool check(Ope &ope) {
      IsLiteralToken vis;
      ope.accept(vis);
      return vis.result_;
    }

  private:
    bool result_ = false;
  };

  struct TokenChecker : public Ope::Visitor {
    using Ope::Visitor::visit;

    void visit(Sequence &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(PrioritizedChoice &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(Repetition &ope) override { ope.ope_->accept(*this); }
    void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
    void visit(Capture &ope) override { ope.ope_->accept(*this); }
    void visit(TokenBoundary &) override { has_token_boundary_ = true; }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }
    void visit(WeakHolder &) override { has_rule_ = true; }
    void visit(Holder &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
    void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
    void visit(Recovery &ope) override { ope.ope_->accept(*this); }

    static bool is_token(Ope &ope) {
      if(IsLiteralToken::check(ope)) {
        return true;
      }

      TokenChecker vis;
      ope.accept(vis);
      return vis.has_token_boundary_ || !vis.has_rule_;
    }

  private:
    bool has_token_boundary_ = false;
    bool has_rule_ = false;
  };

  struct FindLiteralToken : public Ope::Visitor {
    using Ope::Visitor::visit;

    void visit(LiteralString &ope) override { token_ = ope.lit_.data(); }
    void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Recovery &ope) override { ope.ope_->accept(*this); }

    static const char *token(Ope &ope) {
      FindLiteralToken vis;
      ope.accept(vis);
      return vis.token_;
    }

  private:
    const char *token_ = nullptr;
  };

  struct DetectLeftRecursion : public Ope::Visitor {
    using Ope::Visitor::visit;

    DetectLeftRecursion(const std::string &name) : name_(name) {}

    void visit(Sequence &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
        if(done_) {
          break;
        } else if(error_s) {
          done_ = true;
          break;
        }
      }
    }

    void visit(PrioritizedChoice &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
        if(error_s) {
          done_ = true;
          break;
        }
      }
    }

    void visit(Repetition &ope) override {
      ope.ope_->accept(*this);
      done_ = ope.min_ > 0;
    }

    void visit(AndPredicate &ope) override {
      ope.ope_->accept(*this);
      done_ = false;
    }

    void visit(NotPredicate &ope) override {
      ope.ope_->accept(*this);
      done_ = false;
    }

    void visit(Dictionary &) override { done_ = true; }
    void visit(LiteralString &ope) override { done_ = !ope.lit_.empty(); }
    void visit(CharacterClass &) override { done_ = true; }
    void visit(Character &) override { done_ = true; }
    void visit(AnyCharacter &) override { done_ = true; }
    void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
    void visit(Capture &ope) override { ope.ope_->accept(*this); }
    void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }
    void visit(User &) override { done_ = true; }
    void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
    void visit(Holder &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
    void visit(BackReference &) override { done_ = true; }
    void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
    void visit(Recovery &ope) override { ope.ope_->accept(*this); }
    void visit(Cut &) override { done_ = true; }

    const char *error_s = nullptr;

  private:
    std::string name_;
    std::unordered_set<std::string> refs_;
    bool done_ = false;
  };

  struct HasEmptyElement : public Ope::Visitor {
    using Ope::Visitor::visit;

    HasEmptyElement(std::vector<std::pair<const char *, std::string>> &refs,
                    std::unordered_map<std::string, bool> &has_error_cache) :
        refs_(refs), has_error_cache_(has_error_cache) {}

    void visit(Sequence &ope) override;

    void visit(PrioritizedChoice &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
        if(is_empty) {
          return;
        }
      }
    }

    void visit(Repetition &ope) override {
      if(ope.min_ == 0) {
        set_error();
      } else {
        ope.ope_->accept(*this);
      }
    }

    void visit(AndPredicate &) override { set_error(); }
    void visit(NotPredicate &) override { set_error(); }

    void visit(LiteralString &ope) override {
      if(ope.lit_.empty()) {
        set_error();
      }
    }

    void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
    void visit(Capture &ope) override { ope.ope_->accept(*this); }
    void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }
    void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
    void visit(Holder &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
    void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
    void visit(Recovery &ope) override { ope.ope_->accept(*this); }

    bool is_empty = false;
    const char *error_s = nullptr;
    std::string error_name;

  private:
    void set_error() {
      is_empty = true;
      tie(error_s, error_name) = refs_.back();
    }

    std::vector<std::pair<const char *, std::string>> &refs_;
    std::unordered_map<std::string, bool> &has_error_cache_;
  };

  struct DetectInfiniteLoop : public Ope::Visitor {
    using Ope::Visitor::visit;

    DetectInfiniteLoop(const char *s, const std::string &name, std::vector<std::pair<const char *, std::string>> &refs,
                       std::unordered_map<std::string, bool> &has_error_cache) :
        refs_(refs), has_error_cache_(has_error_cache) {
      refs_.emplace_back(s, name);
    }

    DetectInfiniteLoop(std::vector<std::pair<const char *, std::string>> &refs,
                       std::unordered_map<std::string, bool> &has_error_cache) :
        refs_(refs), has_error_cache_(has_error_cache) {}

    void visit(Sequence &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
        if(has_error) {
          return;
        }
      }
    }

    void visit(PrioritizedChoice &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
        if(has_error) {
          return;
        }
      }
    }

    void visit(Repetition &ope) override {
      if(ope.max_ == std::numeric_limits<size_t>::max()) {
        HasEmptyElement vis(refs_, has_error_cache_);
        ope.ope_->accept(vis);
        if(vis.is_empty) {
          has_error = true;
          error_s = vis.error_s;
          error_name = vis.error_name;
        }
      } else {
        ope.ope_->accept(*this);
      }
    }

    void visit(AndPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(NotPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
    void visit(Capture &ope) override { ope.ope_->accept(*this); }
    void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }
    void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
    void visit(Holder &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
    void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
    void visit(Recovery &ope) override { ope.ope_->accept(*this); }

    bool has_error = false;
    const char *error_s = nullptr;
    std::string error_name;

  private:
    std::vector<std::pair<const char *, std::string>> &refs_;
    std::unordered_map<std::string, bool> &has_error_cache_;
  };

  struct ReferenceChecker : public Ope::Visitor {
    using Ope::Visitor::visit;

    ReferenceChecker(const Grammar &grammar, const std::vector<std::string> &params) :
        grammar_(grammar), params_(params) {}

    void visit(Sequence &ope) override {
      for(const auto &op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(PrioritizedChoice &ope) override {
      for(auto op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(Repetition &ope) override { ope.ope_->accept(*this); }
    void visit(AndPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(NotPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
    void visit(Capture &ope) override { ope.ope_->accept(*this); }
    void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }
    void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
    void visit(Holder &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
    void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
    void visit(Recovery &ope) override { ope.ope_->accept(*this); }

    std::unordered_map<std::string, const char *> error_s;
    std::unordered_map<std::string, std::string> error_message;
    std::unordered_set<std::string> referenced;

  private:
    const Grammar &grammar_;
    const std::vector<std::string> &params_;
  };

  struct LinkReferences : public Ope::Visitor {
    using Ope::Visitor::visit;

    LinkReferences(Grammar &grammar, const std::vector<std::string> &params) : grammar_(grammar), params_(params) {}

    void visit(Sequence &ope) override {
      for(auto op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(PrioritizedChoice &ope) override {
      for(auto op: ope.opes_) {
        op->accept(*this);
      }
    }

    void visit(Repetition &ope) override { ope.ope_->accept(*this); }
    void visit(AndPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(NotPredicate &ope) override { ope.ope_->accept(*this); }
    void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
    void visit(Capture &ope) override { ope.ope_->accept(*this); }
    void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
    void visit(Ignore &ope) override { ope.ope_->accept(*this); }
    void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
    void visit(Holder &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
    void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
    void visit(Recovery &ope) override { ope.ope_->accept(*this); }

  private:
    Grammar &grammar_;
    const std::vector<std::string> &params_;
  };

  struct FindReference : public Ope::Visitor {
    using Ope::Visitor::visit;

    FindReference(const std::vector<std::shared_ptr<Ope>> &args, const std::vector<std::string> &params) :
        args_(args), params_(params) {}

    void visit(Sequence &ope) override {
      std::vector<std::shared_ptr<Ope>> opes;
      for(auto o: ope.opes_) {
        o->accept(*this);
        opes.push_back(found_ope);
      }
      found_ope = std::make_shared<Sequence>(opes);
    }

    void visit(PrioritizedChoice &ope) override {
      std::vector<std::shared_ptr<Ope>> opes;
      for(auto o: ope.opes_) {
        o->accept(*this);
        opes.push_back(found_ope);
      }
      found_ope = std::make_shared<PrioritizedChoice>(opes);
    }

    void visit(Repetition &ope) override {
      ope.ope_->accept(*this);
      found_ope = rep(found_ope, ope.min_, ope.max_);
    }

    void visit(AndPredicate &ope) override {
      ope.ope_->accept(*this);
      found_ope = apd(found_ope);
    }

    void visit(NotPredicate &ope) override {
      ope.ope_->accept(*this);
      found_ope = npd(found_ope);
    }

    void visit(Dictionary &ope) override { found_ope = ope.shared_from_this(); }

    void visit(LiteralString &ope) override { found_ope = ope.shared_from_this(); }

    void visit(CharacterClass &ope) override { found_ope = ope.shared_from_this(); }

    void visit(Character &ope) override { found_ope = ope.shared_from_this(); }
    void visit(AnyCharacter &ope) override { found_ope = ope.shared_from_this(); }

    void visit(CaptureScope &ope) override {
      ope.ope_->accept(*this);
      found_ope = csc(found_ope);
    }

    void visit(Capture &ope) override {
      ope.ope_->accept(*this);
      found_ope = cap(found_ope, ope.match_action_);
    }

    void visit(TokenBoundary &ope) override {
      ope.ope_->accept(*this);
      found_ope = tok(found_ope);
    }

    void visit(Ignore &ope) override {
      ope.ope_->accept(*this);
      found_ope = ign(found_ope);
    }

    void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
    void visit(Holder &ope) override { ope.ope_->accept(*this); }

    void visit(Reference &ope) override;

    void visit(Whitespace &ope) override {
      ope.ope_->accept(*this);
      found_ope = wsp(found_ope);
    }

    void visit(PrecedenceClimbing &ope) override {
      ope.atom_->accept(*this);
      found_ope = csc(found_ope);
    }

    void visit(Recovery &ope) override {
      ope.ope_->accept(*this);
      found_ope = rec(found_ope);
    }

    void visit(Cut &ope) override { found_ope = ope.shared_from_this(); }

    std::shared_ptr<Ope> found_ope;

  private:
    const std::vector<std::shared_ptr<Ope>> &args_;
    const std::vector<std::string> &params_;
  };

  /*
   * Definition
   */
  class Definition {
  public:
    struct Result {
      bool ret;
      bool recovered;
      size_t len;
      ErrorInfo error_info;
    };

    Definition() : holder_(std::make_shared<Holder>(this)) {}

    Definition(const Definition &rhs) : name(rhs.name), holder_(rhs.holder_) { holder_->outer_ = this; }

    Definition(const std::shared_ptr<Ope> &ope) : holder_(std::make_shared<Holder>(this)) { *this <= ope; }

    operator std::shared_ptr<Ope>() { return std::make_shared<WeakHolder>(holder_); }

    Definition &operator<=(const std::shared_ptr<Ope> &ope) {
      holder_->ope_ = ope;
      return *this;
    }

    Result parse(const char *s, size_t n, const char *path = nullptr, Log log = nullptr) const {
      SemanticValues vs;
      std::any dt;
      return parse_core(s, n, vs, dt, path, log);
    }

    Result parse(const char *s, const char *path = nullptr, Log log = nullptr) const {
      auto n = strlen(s);
      return parse(s, n, path, log);
    }

    Result parse(const char *s, size_t n, std::any &dt, const char *path = nullptr, Log log = nullptr) const {
      SemanticValues vs;
      return parse_core(s, n, vs, dt, path, log);
    }

    Result parse(const char *s, std::any &dt, const char *path = nullptr, Log log = nullptr) const {
      auto n = strlen(s);
      return parse(s, n, dt, path, log);
    }

    template<typename T>
    Result parse_and_get_value(const char *s, size_t n, T &val, const char *path = nullptr, Log log = nullptr) const {
      SemanticValues vs;
      std::any dt;
      auto r = parse_core(s, n, vs, dt, path, log);
      if(r.ret && !vs.empty() && vs.front().has_value()) {
        val = std::any_cast<T>(vs[0]);
      }
      return r;
    }

    template<typename T>
    Result parse_and_get_value(const char *s, T &val, const char *path = nullptr, Log log = nullptr) const {
      auto n = strlen(s);
      return parse_and_get_value(s, n, val, path, log);
    }

    template<typename T>
    Result parse_and_get_value(const char *s, size_t n, std::any &dt, T &val, const char *path = nullptr,
                               Log log = nullptr) const {
      SemanticValues vs;
      auto r = parse_core(s, n, vs, dt, path, log);
      if(r.ret && !vs.empty() && vs.front().has_value()) {
        val = std::any_cast<T>(vs[0]);
      }
      return r;
    }

    template<typename T>
    Result parse_and_get_value(const char *s, std::any &dt, T &val, const char *path = nullptr,
                               Log log = nullptr) const {
      auto n = strlen(s);
      return parse_and_get_value(s, n, dt, val, path, log);
    }

#if defined(__cpp_lib_char8_t)
    Result parse(const char8_t *s, size_t n, const char *path = nullptr, Log log = nullptr) const {
      return parse(reinterpret_cast<const char *>(s), n, path, log);
    }

    Result parse(const char8_t *s, const char *path = nullptr, Log log = nullptr) const {
      return parse(reinterpret_cast<const char *>(s), path, log);
    }

    Result parse(const char8_t *s, size_t n, std::any &dt, const char *path = nullptr, Log log = nullptr) const {
      return parse(reinterpret_cast<const char *>(s), n, dt, path, log);
    }

    Result parse(const char8_t *s, std::any &dt, const char *path = nullptr, Log log = nullptr) const {
      return parse(reinterpret_cast<const char *>(s), dt, path, log);
    }

    template<typename T>
    Result parse_and_get_value(const char8_t *s, size_t n, T &val, const char *path = nullptr,
                               Log log = nullptr) const {
      return parse_and_get_value(reinterpret_cast<const char *>(s), n, val, *path, log);
    }

    template<typename T>
    Result parse_and_get_value(const char8_t *s, T &val, const char *path = nullptr, Log log = nullptr) const {
      return parse_and_get_value(reinterpret_cast<const char *>(s), val, *path, log);
    }

    template<typename T>
    Result parse_and_get_value(const char8_t *s, size_t n, std::any &dt, T &val, const char *path = nullptr,
                               Log log = nullptr) const {
      return parse_and_get_value(reinterpret_cast<const char *>(s), n, dt, val, *path, log);
    }

    template<typename T>
    Result parse_and_get_value(const char8_t *s, std::any &dt, T &val, const char *path = nullptr,
                               Log log = nullptr) const {
      return parse_and_get_value(reinterpret_cast<const char *>(s), dt, val, *path, log);
    }
#endif

    void operator=(Action a) { action = a; }

    template<typename T>
    Definition &operator,(T fn) {
      operator=(fn);
      return *this;
    }

    Definition &operator~() {
      ignoreSemanticValue = true;
      return *this;
    }

    void accept(Ope::Visitor &v) { holder_->accept(v); }

    std::shared_ptr<Ope> get_core_operator() const { return holder_->ope_; }

    bool is_token() const {
      std::call_once(is_token_init_, [this]() { is_token_ = TokenChecker::is_token(*get_core_operator()); });
      return is_token_;
    }

    std::string name;
    const char *s_ = nullptr;
    std::pair<size_t, size_t> line_ = {1, 1};

    std::function<bool(const SemanticValues &vs, const std::any &dt, std::string &msg)> predicate;

    size_t id = 0;
    Action action;
    std::function<void(const Context &c, const char *s, size_t n, std::any &dt)> enter;
    std::function<void(const Context &c, const char *s, size_t n, size_t matchlen, std::any &value, std::any &dt)>
        leave;
    bool ignoreSemanticValue = false;
    std::shared_ptr<Ope> whitespaceOpe;
    std::shared_ptr<Ope> wordOpe;
    bool enablePackratParsing = false;
    bool is_macro = false;
    std::vector<std::string> params;
    bool disable_action = false;

    TracerEnter tracer_enter;
    TracerLeave tracer_leave;
    bool verbose_trace = false;
    TracerStartOrEnd tracer_start;
    TracerStartOrEnd tracer_end;

    std::string error_message;
    bool no_ast_opt = false;

    bool eoi_check = true;

  private:
    friend class Reference;
    friend class ParserGenerator;

    Definition &operator=(const Definition &rhs);

    Definition &operator=(Definition &&rhs);

    void initialize_definition_ids() const {
      std::call_once(definition_ids_init_, [&]() {
        AssignIDToDefinition vis;
        holder_->accept(vis);
        if(whitespaceOpe) {
          whitespaceOpe->accept(vis);
        }
        if(wordOpe) {
          wordOpe->accept(vis);
        }
        definition_ids_.swap(vis.ids);
      });
    }

    Result parse_core(const char *s, size_t n, SemanticValues &vs, std::any &dt, const char *path, Log log) const {
      initialize_definition_ids();

      std::shared_ptr<Ope> ope = holder_;

      std::any trace_data;
      if(tracer_start) {
        tracer_start(trace_data);
      }
      auto se = scope_exit([&]() {
        if(tracer_end) {
          tracer_end(trace_data);
        }
      });

      Context c(path, s, n, definition_ids_.size(), whitespaceOpe, wordOpe, enablePackratParsing, tracer_enter,
                tracer_leave, trace_data, verbose_trace, log);

      size_t i = 0;

      if(whitespaceOpe) {
        auto save_ignore_trace_state = c.ignore_trace_state;
        c.ignore_trace_state = !c.verbose_trace;
        auto se = scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

        auto len = whitespaceOpe->parse(s, n, vs, c, dt);
        if(fail(len)) {
          return Result{false, c.recovered, i, c.error_info};
        }

        i = len;
      }

      auto len = ope->parse(s + i, n - i, vs, c, dt);
      auto ret = success(len);
      if(ret) {
        i += len;
        if(eoi_check) {
          if(i < n) {
            if(c.error_info.error_pos - c.s < s + i - c.s) {
              c.error_info.message_pos = s + i;
              c.error_info.message = "expected end of input";
            }
            ret = false;
          }
        }
      }
      return Result{ret, c.recovered, i, c.error_info};
    }

    std::shared_ptr<Holder> holder_;
    mutable std::once_flag is_token_init_;
    mutable bool is_token_ = false;
    mutable std::once_flag assign_id_to_definition_init_;
    mutable std::once_flag definition_ids_init_;
    mutable std::unordered_map<void *, size_t> definition_ids_;
  };

  /*
   * Implementations
   */

  inline size_t parse_literal(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt,
                              const std::string &lit, std::once_flag &init_is_word, bool &is_word, bool ignore_case) {
    size_t i = 0;
    for(; i < lit.size(); i++) {
      if(i >= n || (ignore_case ? (std::tolower(s[i]) != std::tolower(lit[i])) : (s[i] != lit[i]))) {
        c.set_error_pos(s, lit.data());
        return static_cast<size_t>(-1);
      }
    }

    // Word check
    if(c.wordOpe) {
      auto save_ignore_trace_state = c.ignore_trace_state;
      c.ignore_trace_state = !c.verbose_trace;
      auto se = scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

      std::call_once(init_is_word, [&]() {
        SemanticValues dummy_vs;
        Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr, nullptr, nullptr, false, nullptr);
        std::any dummy_dt;

        auto len = c.wordOpe->parse(lit.data(), lit.size(), dummy_vs, dummy_c, dummy_dt);
        is_word = success(len);
      });

      if(is_word) {
        SemanticValues dummy_vs;
        Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr, nullptr, nullptr, false, nullptr);
        std::any dummy_dt;

        NotPredicate ope(c.wordOpe);
        auto len = ope.parse(s + i, n - i, dummy_vs, dummy_c, dummy_dt);
        if(fail(len)) {
          c.set_error_pos(s, lit.data());
          return len;
        }
        i += len;
      }
    }

    // Skip whitespace
    if(!c.in_token_boundary_count && c.whitespaceOpe) {
      const auto save_ignore_trace_state = c.ignore_trace_state;
      c.ignore_trace_state = !c.verbose_trace;
      auto se = scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

      auto len = c.whitespaceOpe->parse(s + i, n - i, vs, c, dt);
      if(fail(len)) {
        return len;
      }
      i += len;
    }

    return i;
  }

  inline std::pair<size_t, size_t> SemanticValues::line_info() const {
    assert(c_);
    return c_->line_info(sv_.data());
  }

  inline void ErrorInfo::output_log(const Log &log, const char *s, size_t n) {
    if(message_pos) {
      if(message_pos > last_output_pos) {
        last_output_pos = message_pos;
        const auto line = line_info(s, message_pos);
        std::string msg;
        if(const auto unexpected_token = heuristic_error_token(s, n, message_pos); !unexpected_token.empty()) {
          msg = replace_all(message, "%t", unexpected_token);

          const auto unexpected_char =
              unexpected_token.substr(0, codepoint_length(unexpected_token.data(), unexpected_token.size()));

          msg = replace_all(msg, "%c", unexpected_char);
        } else {
          msg = message;
        }
        log(line.first, line.second, msg, label);
      }
    } else if(error_pos) {
      if(error_pos > last_output_pos) {
        last_output_pos = error_pos;
        auto line = line_info(s, error_pos);

        std::string msg;
        if(expected_tokens.empty()) {
          msg = "syntax error.";
        } else {
          msg = "syntax error";

          // unexpected token
          if(const auto unexpected_token = heuristic_error_token(s, n, error_pos); !unexpected_token.empty()) {
            msg += ", unexpected '";
            msg += unexpected_token;
            msg += "'";
          }

          auto first_item = true;
          size_t i = 0;
          while(i < expected_tokens.size()) {

            // Skip rules start with '_'
            if(auto [error_literal, error_rule] = expected_tokens[i]; !(error_rule && error_rule->name[0] == '_')) {
              msg += (first_item ? ", expecting " : ", ");
              if(error_literal) {
                msg += "'";
                msg += error_literal;
                msg += "'";
              } else {
                msg += "<" + error_rule->name + ">";
                if(label.empty()) {
                  label = error_rule->name;
                }
              }
              first_item = false;
            }

            i++;
          }
          msg += ".";
        }
        log(line.first, line.second, msg, label);
      }
    }
  }

  inline void Context::set_error_pos(const char *a_s, const char *literal) {
    if(log) {
      if(error_info.error_pos <= a_s) {
        if(error_info.error_pos < a_s || !error_info.keep_previous_token) {
          error_info.error_pos = a_s;
          error_info.expected_tokens.clear();
        }

        const char *error_literal = nullptr;
        const Definition *error_rule = nullptr;

        if(literal) {
          error_literal = literal;
        } else if(!rule_stack.empty()) {
          const auto rule = rule_stack.back();
          const auto ope = rule->get_core_operator();
          if(const auto token = FindLiteralToken::token(*ope); token && token[0] != '\0') {
            error_literal = token;
          }
        }

        for(auto r: rule_stack) {
          error_rule = r;
          if(r->is_token()) {
            break;
          }
        }

        if(error_literal || error_rule) {
          error_info.add(error_literal, error_rule);
        }
      }
    }
  }

  inline void Context::trace_enter(const Ope &ope, const char *a_s, size_t n, const SemanticValues &vs, std::any &dt) {
    trace_ids.push_back(next_trace_id++);
    tracer_enter(ope, a_s, n, vs, *this, dt, trace_data);
  }

  inline void Context::trace_leave(const Ope &ope, const char *a_s, size_t n, const SemanticValues &vs, std::any &dt,
                                   size_t len) {
    tracer_leave(ope, a_s, n, vs, *this, dt, len, trace_data);
    trace_ids.pop_back();
  }

  inline bool Context::is_traceable(const Ope &ope) const {
    if(tracer_enter && tracer_leave) {
      if(ignore_trace_state) {
        return false;
      }
      return !FOS_CAST<const peg::Reference *>(&ope);
    }
    return false;
  }

  inline size_t Ope::parse(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const {
    if(c.is_traceable(*this)) {
      c.trace_enter(*this, s, n, vs, dt);
      const auto len = parse_core(s, n, vs, c, dt);
      c.trace_leave(*this, s, n, vs, dt, len);
      return len;
    }
    return parse_core(s, n, vs, c, dt);
  }

  inline size_t Dictionary::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const {
    size_t id;
    auto i = trie_.match(s, n, id);

    if(i == 0) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }

    vs.choice_count_ = trie_.size();
    vs.choice_ = id;

    // Word check
    if(c.wordOpe) {
      const auto save_ignore_trace_state = c.ignore_trace_state;
      c.ignore_trace_state = !c.verbose_trace;
      auto se = scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });
      {
        SemanticValues dummy_vs;
        Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr, nullptr, nullptr, false, nullptr);
        std::any dummy_dt;

        NotPredicate ope(c.wordOpe);
        const auto len = ope.parse(s + i, n - i, dummy_vs, dummy_c, dummy_dt);
        if(fail(len)) {
          c.set_error_pos(s);
          return len;
        }
        i += len;
      }
    }

    // Skip whitespace
    if(!c.in_token_boundary_count && c.whitespaceOpe) {
      auto save_ignore_trace_state = c.ignore_trace_state;
      c.ignore_trace_state = !c.verbose_trace;
      auto se = scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

      const auto len = c.whitespaceOpe->parse(s + i, n - i, vs, c, dt);
      if(fail(len)) {
        return len;
      }
      i += len;
    }

    return i;
  }

  inline size_t LiteralString::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const {
    return parse_literal(s, n, vs, c, dt, lit_, init_is_word_, is_word_, ignore_case_);
  }

  inline size_t TokenBoundary::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const {
    auto save_ignore_trace_state = c.ignore_trace_state;
    c.ignore_trace_state = !c.verbose_trace;
    auto se = scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

    size_t len;
    {
      c.in_token_boundary_count++;
      auto se = scope_exit([&]() { c.in_token_boundary_count--; });
      len = ope_->parse(s, n, vs, c, dt);
    }

    if(success(len)) {
      vs.tokens.emplace_back(std::string_view(s, len));

      if(!c.in_token_boundary_count) {
        if(c.whitespaceOpe) {
          const auto l = c.whitespaceOpe->parse(s + len, n - len, vs, c, dt);
          if(fail(l)) {
            return l;
          }
          len += l;
        }
      }
    }
    return len;
  }

  inline size_t Holder::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const {
    if(!ope_) {
      throw std::logic_error("Uninitialized definition ope was used...");
    }

    // Macro reference
    if(outer_->is_macro) {
      c.rule_stack.push_back(outer_);
      auto len = ope_->parse(s, n, vs, c, dt);
      c.rule_stack.pop_back();
      return len;
    }

    size_t len;
    std::any val;

    c.packrat(s, outer_->id, len, val, [&](std::any &a_val) {
      if(outer_->enter) {
        outer_->enter(c, s, n, dt);
      }
      auto &chvs = c.push_semantic_values_scope();
      auto se = scope_exit([&]() {
        c.pop_semantic_values_scope();
        if(outer_->leave) {
          outer_->leave(c, s, n, len, a_val, dt);
        }
      });

      c.rule_stack.push_back(outer_);
      len = ope_->parse(s, n, chvs, c, dt);
      c.rule_stack.pop_back();

      // Invoke action
      if(success(len)) {
        chvs.sv_ = std::string_view(s, len);
        chvs.name_ = outer_->name;

        auto ope_ptr = ope_.get();
        {
          if(auto tok_ptr = FOS_CAST<const peg::TokenBoundary *>(ope_ptr)) {
            ope_ptr = tok_ptr->ope_.get();
          }
        }
        if(!FOS_CAST<const peg::PrioritizedChoice *>(ope_ptr) && !FOS_CAST<const peg::Dictionary *>(ope_ptr)) {
          chvs.choice_count_ = 0;
          chvs.choice_ = 0;
        }

        std::string msg;
        if(outer_->predicate && !outer_->predicate(chvs, dt, msg)) {
          if(c.log && !msg.empty() && c.error_info.message_pos < s) {
            c.error_info.message_pos = s;
            c.error_info.message = msg;
            c.error_info.label = outer_->name;
          }
          len = static_cast<size_t>(-1);
        }

        if(success(len)) {
          if(!c.recovered) {
            a_val = reduce(chvs, dt);
          }
        } else {
          if(c.log && !msg.empty() && c.error_info.message_pos < s) {
            c.error_info.message_pos = s;
            c.error_info.message = msg;
            c.error_info.label = outer_->name;
          }
        }
      } else {
        if(c.log && !outer_->error_message.empty() && c.error_info.message_pos < s) {
          c.error_info.message_pos = s;
          c.error_info.message = outer_->error_message;
          c.error_info.label = outer_->name;
        }
      }
    });

    if(success(len)) {
      if(!outer_->ignoreSemanticValue) {
        vs.emplace_back(std::move(val));
        vs.tags.emplace_back(str2tag(outer_->name));
      }
    }

    return len;
  }

  inline std::any Holder::reduce(SemanticValues &vs, std::any &dt) const {
    if(outer_->action && !outer_->disable_action) {
      return outer_->action(vs, dt);
    } else if(vs.empty()) {
      return std::any();
    } else {
      return std::move(vs.front());
    }
  }

  inline const std::string &Holder::name() const { return outer_->name; }

  inline const std::string &Holder::trace_name() const {
    std::call_once(trace_name_init_, [this]() { trace_name_ = "[" + outer_->name + "]"; });
    return trace_name_;
  }

  inline size_t Reference::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const {
    auto save_ignore_trace_state = c.ignore_trace_state;
    if(rule_ && rule_->ignoreSemanticValue) {
      c.ignore_trace_state = !c.verbose_trace;
    }
    auto se = scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

    if(rule_) {
      // Reference rule
      if(rule_->is_macro) {
        // Macro
        FindReference vis(c.top_args(), c.rule_stack.back()->params);

        // Collect arguments
        std::vector<std::shared_ptr<Ope>> args;
        for(auto arg: args_) {
          arg->accept(vis);
          args.emplace_back(std::move(vis.found_ope));
        }

        c.push_args(std::move(args));
        auto se = scope_exit([&]() { c.pop_args(); });
        auto ope = get_core_operator();
        return ope->parse(s, n, vs, c, dt);
      } else {
        // Definition
        c.push_args(std::vector<std::shared_ptr<Ope>>());
        auto se = scope_exit([&]() { c.pop_args(); });
        auto ope = get_core_operator();
        return ope->parse(s, n, vs, c, dt);
      }
    } else {
      // Reference parameter in macro
      const auto &args = c.top_args();
      return args[iarg_]->parse(s, n, vs, c, dt);
    }
  }

  inline std::shared_ptr<Ope> Reference::get_core_operator() const { return rule_->holder_; }

  inline size_t BackReference::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c, std::any &dt) const {
    auto size = static_cast<int>(c.capture_scope_stack_size);
    for(auto i = size - 1; i >= 0; i--) {
      auto index = static_cast<size_t>(i);
      const auto &cs = c.capture_scope_stack[index];
      if(cs.find(name_) != cs.end()) {
        const auto &lit = cs.at(name_);
        std::once_flag init_is_word;
        auto is_word = false;
        return parse_literal(s, n, vs, c, dt, lit, init_is_word, is_word, false);
      }
    }

    c.error_info.message_pos = s;
    c.error_info.message = "undefined back reference '$" + name_ + "'...";
    return static_cast<size_t>(-1);
  }

  inline Definition &PrecedenceClimbing::get_reference_for_binop(Context &c) const {
    if(rule_.is_macro) {
      // Reference parameter in macro
      const auto &args = c.top_args();
      const auto iarg = FOS_CAST<Reference &>(*binop_).iarg_;
      const auto arg = args[iarg];
      return *FOS_CAST<Reference &>(*arg).rule_;
    }

    return *FOS_CAST<Reference &>(*binop_).rule_;
  }

  inline size_t PrecedenceClimbing::parse_expression(const char *s, size_t n, SemanticValues &vs, Context &c,
                                                     std::any &dt, size_t min_prec) const {
    const auto len = atom_->parse(s, n, vs, c, dt);
    if(fail(len)) {
      return len;
    }

    std::string tok;
    auto &rule = get_reference_for_binop(c);
    auto action = std::move(rule.action);

    rule.action = [&](SemanticValues &vs2, std::any &dt2) {
      tok = vs2.token();
      if(action) {
        return action(vs2, dt2);
      } else if(!vs2.empty()) {
        return vs2[0];
      }
      return std::any();
    };
    auto action_se = scope_exit([&]() { rule.action = std::move(action); });

    auto i = len;
    while(i < n) {
      std::vector<std::any> save_values(vs.begin(), vs.end());
      const auto save_tokens = vs.tokens;

      auto chvs = c.push_semantic_values_scope();
      auto chlen = binop_->parse(s + i, n - i, chvs, c, dt);
      c.pop_semantic_values_scope();

      if(fail(chlen)) {
        break;
      }

      auto it = info_.find(tok);
      if(it == info_.end()) {
        break;
      }

      const auto level = std::get<0>(it->second);
      const auto assoc = std::get<1>(it->second);

      if(level < min_prec) {
        break;
      }

      vs.emplace_back(std::move(chvs[0]));
      i += chlen;

      auto next_min_prec = level;
      if(assoc == 'L') {
        next_min_prec = level + 1;
      }

      chvs = c.push_semantic_values_scope();
      chlen = parse_expression(s + i, n - i, chvs, c, dt, next_min_prec);
      c.pop_semantic_values_scope();

      if(fail(chlen)) {
        vs.assign(save_values.begin(), save_values.end());
        vs.tokens = save_tokens;
        i = chlen;
        break;
      }

      vs.emplace_back(std::move(chvs[0]));
      i += chlen;

      std::any val;
      if(rule_.action) {
        vs.sv_ = std::string_view(s, i);
        val = rule_.action(vs, dt);
      } else if(!vs.empty()) {
        val = vs[0];
      }
      vs.clear();
      vs.emplace_back(std::move(val));
    }

    return i;
  }

  inline size_t Recovery::parse_core(const char *s, size_t n, SemanticValues & /*vs*/, Context &c,
                                     std::any & /*dt*/) const {
    const auto &rule = FOS_CAST<Reference &>(*ope_);

    // Custom error message
    if(c.log) {
      if(const auto label = FOS_CAST<Reference *>(rule.args_[0].get()); label && !label->rule_->error_message.empty()) {
        c.error_info.message_pos = s;
        c.error_info.message = label->rule_->error_message;
        c.error_info.label = label->rule_->name;
      }
    }

    // Recovery
    auto len = static_cast<size_t>(-1);
    {
      const auto save_log = c.log;
      c.log = nullptr;
      auto se = scope_exit([&]() { c.log = save_log; });

      SemanticValues dummy_vs;
      std::any dummy_dt;

      len = rule.parse(s, n, dummy_vs, c, dummy_dt);
    }

    if(success(len)) {
      c.recovered = true;

      if(c.log) {
        c.error_info.output_log(c.log, c.s, c.l);
        c.error_info.clear();
      }
    }

    // Cut
    if(!c.cut_stack.empty()) {
      c.cut_stack.back() = true;

      if(c.cut_stack.size() == 1) {
        // TODO: Remove unneeded entries in packrat memoise table
      }
    }

    return len;
  }

  inline void Sequence::accept(Visitor &v) { v.visit(*this); }
  inline void PrioritizedChoice::accept(Visitor &v) { v.visit(*this); }
  inline void Repetition::accept(Visitor &v) { v.visit(*this); }
  inline void AndPredicate::accept(Visitor &v) { v.visit(*this); }
  inline void NotPredicate::accept(Visitor &v) { v.visit(*this); }
  inline void Dictionary::accept(Visitor &v) { v.visit(*this); }
  inline void LiteralString::accept(Visitor &v) { v.visit(*this); }
  inline void CharacterClass::accept(Visitor &v) { v.visit(*this); }
  inline void Character::accept(Visitor &v) { v.visit(*this); }
  inline void AnyCharacter::accept(Visitor &v) { v.visit(*this); }
  inline void CaptureScope::accept(Visitor &v) { v.visit(*this); }
  inline void Capture::accept(Visitor &v) { v.visit(*this); }
  inline void TokenBoundary::accept(Visitor &v) { v.visit(*this); }
  inline void Ignore::accept(Visitor &v) { v.visit(*this); }
  inline void User::accept(Visitor &v) { v.visit(*this); }
  inline void WeakHolder::accept(Visitor &v) { v.visit(*this); }
  inline void Holder::accept(Visitor &v) { v.visit(*this); }
  inline void Reference::accept(Visitor &v) { v.visit(*this); }
  inline void Whitespace::accept(Visitor &v) { v.visit(*this); }
  inline void BackReference::accept(Visitor &v) { v.visit(*this); }
  inline void PrecedenceClimbing::accept(Visitor &v) { v.visit(*this); }
  inline void Recovery::accept(Visitor &v) { v.visit(*this); }
  inline void Cut::accept(Visitor &v) { v.visit(*this); }

  inline void AssignIDToDefinition::visit(Holder &ope) {
    const auto p = static_cast<void *>(ope.outer_);
    if(ids.count(p)) {
      return;
    }
    const auto id = ids.size();
    ids[p] = id;
    ope.outer_->id = id;
    ope.ope_->accept(*this);
  }

  inline void AssignIDToDefinition::visit(Reference &ope) {
    if(ope.rule_) {
      for(const auto &arg: ope.args_) {
        arg->accept(*this);
      }
      ope.rule_->accept(*this);
    }
  }

  inline void AssignIDToDefinition::visit(PrecedenceClimbing &ope) {
    ope.atom_->accept(*this);
    ope.binop_->accept(*this);
  }

  inline void TokenChecker::visit(Reference &ope) {
    if(ope.is_macro_) {
      for(const auto &arg: ope.args_) {
        arg->accept(*this);
      }
    } else {
      has_rule_ = true;
    }
  }

  inline void FindLiteralToken::visit(Reference &ope) {
    if(ope.is_macro_) {
      ope.rule_->accept(*this);
      for(const auto &arg: ope.args_) {
        arg->accept(*this);
      }
    }
  }

  inline void DetectLeftRecursion::visit(Reference &ope) {
    if(ope.name_ == name_) {
      error_s = ope.s_;
    } else if(!refs_.count(ope.name_)) {
      refs_.insert(ope.name_);
      if(ope.rule_) {
        ope.rule_->accept(*this);
        if(done_ == false) {
          return;
        }
      }
    }
    done_ = true;
  }

  inline void HasEmptyElement::visit(Sequence &ope) {
    auto save_is_empty = false;
    const char *save_error_s = nullptr;
    std::string save_error_name;

    auto it = ope.opes_.begin();
    while(it != ope.opes_.end()) {
      (*it)->accept(*this);
      if(!is_empty) {
        ++it;
        while(it != ope.opes_.end()) {
          DetectInfiniteLoop vis(refs_, has_error_cache_);
          (*it)->accept(vis);
          if(vis.has_error) {
            is_empty = true;
            error_s = vis.error_s;
            error_name = vis.error_name;
          }
          ++it;
        }
        return;
      }

      save_is_empty = is_empty;
      save_error_s = error_s;
      save_error_name = error_name;

      is_empty = false;
      error_name.clear();
      ++it;
    }

    is_empty = save_is_empty;
    error_s = save_error_s;
    error_name = save_error_name;
  }

  inline void HasEmptyElement::visit(Reference &ope) {
    const auto it = std::find_if(refs_.begin(), refs_.end(), [&](const std::pair<const char *, std::string> &ref) {
      return ope.name_ == ref.second;
    });
    if(it != refs_.end()) {
      return;
    }

    if(ope.rule_) {
      refs_.emplace_back(ope.s_, ope.name_);
      ope.rule_->accept(*this);
      refs_.pop_back();
    }
  }

  inline void DetectInfiniteLoop::visit(Reference &ope) {
    const auto it = std::find_if(refs_.begin(), refs_.end(), [&](const std::pair<const char *, std::string> &ref) {
      return ope.name_ == ref.second;
    });
    if(it != refs_.end()) {
      return;
    }

    if(ope.rule_) {
      auto it = has_error_cache_.find(ope.name_);
      if(it != has_error_cache_.end()) {
        has_error = it->second;
      } else {
        refs_.emplace_back(ope.s_, ope.name_);
        ope.rule_->accept(*this);
        refs_.pop_back();
        has_error_cache_[ope.name_] = has_error;
      }
    }

    if(ope.is_macro_) {
      for(const auto &arg: ope.args_) {
        arg->accept(*this);
      }
    }
  }

  inline void ReferenceChecker::visit(Reference &ope) {
    if(const auto it = std::find(params_.begin(), params_.end(), ope.name_); it != params_.end()) {
      return;
    }

    if(!grammar_.count(ope.name_)) {
      error_s[ope.name_] = ope.s_;
      error_message[ope.name_] = "'" + ope.name_ + "' is not defined.";
    } else {
      if(!referenced.count(ope.name_)) {
        referenced.insert(ope.name_);
      }
      if(const auto &rule = grammar_.at(ope.name_); rule.is_macro) {
        if(!ope.is_macro_ || ope.args_.size() != rule.params.size()) {
          error_s[ope.name_] = ope.s_;
          error_message[ope.name_] = "incorrect number of arguments.";
        }
      } else if(ope.is_macro_) {
        error_s[ope.name_] = ope.s_;
        error_message[ope.name_] = "'" + ope.name_ + "' is not macro.";
      }
      for(const auto &arg: ope.args_) {
        arg->accept(*this);
      }
    }
  }

  inline void LinkReferences::visit(Reference &ope) {
    // Check if the reference is a macro parameter
    auto found_param = false;
    for(size_t i = 0; i < params_.size(); i++) {
      if(const auto &param = params_[i]; param == ope.name_) {
        ope.iarg_ = i;
        found_param = true;
        break;
      }
    }

    // Check if the reference is a definition rule
    if(!found_param && grammar_.count(ope.name_)) {
      auto &rule = grammar_.at(ope.name_);
      ope.rule_ = &rule;
    }

    for(const auto &arg: ope.args_) {
      arg->accept(*this);
    }
  }

  inline void FindReference::visit(Reference &ope) {
    for(size_t i = 0; i < args_.size(); i++) {
      if(const auto &name = params_[i]; name == ope.name_) {
        found_ope = args_[i];
        return;
      }
    }
    found_ope = ope.shared_from_this();
  }

  /*-----------------------------------------------------------------------------
   *  PEG parser generator
   *---------------------------------------------------------------------------*/
  /// REMOVED (NOT USED BY FHATOS)
  /*-----------------------------------------------------------------------------
   *  AST
   *---------------------------------------------------------------------------*/
  /// REMOVED (NOT USED BY FHATOS)
} // namespace peg
