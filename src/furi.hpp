/*******************************************************************************
  FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once
#ifndef fhatos_furi_hpp
#define fhatos_furi_hpp

#define FOS_MAX_PATH_SEGMENTS 15
#define COMPONENT_SEPARATOR STR(::)
#define COEFFICIENT_SEPARATOR STR($)

#include "fhatos.hpp"
//
#include <sstream>
#include <ostream>
#include "util/string_helper.hpp"

namespace fhatos {
  enum class URI_PART { SCHEME, USER, PASSWORD, HOST, PORT, PATH, COEFFICIENT,/*FRAGMENT,*/ QUERY };

  // scheme://user:password@host:port/path...
  const static Enums<URI_PART> URI_PARTS = Enums<URI_PART>{
      {URI_PART::SCHEME, "scheme"}, {URI_PART::USER, "user"}, {URI_PART::PASSWORD, "password"},
      {URI_PART::HOST, "host"}, {URI_PART::PORT, "port"}, {URI_PART::PATH, "path"},
      /* {URI_PART::FRAGMENT, "fragment"},*/ {URI_PART::QUERY, "query"},
  };

  class fURI {
  protected:
    const char *scheme_ = nullptr;
    const char *user_ = nullptr;
    const char *password_ = nullptr;
    const char *host_ = nullptr;
    uint16_t port_ = 0;
    char **path_ = nullptr;
    bool sprefix_ = false;
    bool spostfix_ = false;
    uint8_t path_length_ = 0;
    const char *query_ = nullptr;
    const char *coefficient_ = nullptr;
    // const char *fragment_ = nullptr;

  public:
    [[nodiscard]] const char *scheme() const { return this->scheme_ ? this->scheme_ : ""; }

    [[nodiscard]] fURI scheme(const char *scheme) {
      auto new_uri = fURI(*this);
      free((void *) new_uri.scheme_);
      const size_t len = strlen(scheme);
      new_uri.scheme_ = 0 == len ? nullptr : strndup(scheme, len);
      return new_uri;
    }

    [[nodiscard]] bool has_scheme() const { return this->scheme_; }

    /// USER
    [[nodiscard]] const char *user() const { return this->user_ ? this->user_ : ""; }

    fURI user(const char *user) const {
      auto new_uri = fURI(*this);
      free((void *) new_uri.user_);
      new_uri.user_ = 0 == strlen(user) ? nullptr : strdup(user);
      return new_uri;
    }

    [[nodiscard]] bool has_user() const { return this->user_; }

    /// PASSWORD
    [[nodiscard]] const char *password() const { return this->password_ ? this->password_ : ""; }

    fURI password(const char *password) const {
      auto new_uri = fURI(*this);
      free((void *) new_uri.password_);
      new_uri.password_ = 0 == strlen(password) ? nullptr : strdup(password);
      return new_uri;
    }

    [[nodiscard]] bool has_password() const { return this->password_; }

    /// HOST
    [[nodiscard]] const char *host() const { return this->host_ ? this->host_ : ""; }

    fURI host(const char *host) const {
      auto new_uri = fURI(*this);
      free((void *) new_uri.host_);
      new_uri.host_ = 0 == strlen(host) ? nullptr : strdup(host);
      if(new_uri.path_length_ == 0)
        new_uri.spostfix_ = false;
      else
        new_uri.sprefix_ = true;
      return new_uri;
    }

    [[nodiscard]] bool has_host() const { return this->host_; }

    /// PORT
    [[nodiscard]] uint16_t port() const { return this->port_; }

    [[nodiscard]] fURI port(const uint16_t port) const {
      auto new_uri = fURI(*this);
      new_uri.port_ = port;
      return new_uri;
    }

    [[nodiscard]] bool has_port() const { return this->port_ > 0; }

    /// AUTHORITY
    [[nodiscard]] string authority() const {
      string authority;
      if(this->user_) {
        authority += this->user_;
      }
      if(this->password_) {
        authority += ':';
        authority += this->password_;
      }
      if(this->host_) {
        if(this->user_ || this->password_)
          authority += '@';
        authority += this->host_;
      }
      if(this->port_) {
        authority += ':';
        authority += to_string(this->port_);
      }
      return authority;
    }

    [[nodiscard]] fURI authority(const char *authority) const {
      const string authority_string =
          (strlen(authority) > 1 && authority[0] == '/' && authority[1] == '/') ? authority : string("//") + authority;
      const auto furi =
          fURI(nullptr != this->scheme_ ? string(this->scheme_) + ":" + authority_string : authority_string);
      return this->path_length_ > 0 ? furi.path(this->path()) : furi;
    }

    /// PATH
    [[nodiscard]] string subpath(const uint8_t start, const uint8_t end = UINT8_MAX) const {
      if(start > this->path_length_ || start > end)
        return "";
      string path_str;
      const uint8_t clip_end = end > this->path_length_ ? this->path_length_ : end;
      if(this->path_) {
        if(this->sprefix_ && start == 0)
          path_str += '/';
        for(uint8_t i = start; i < clip_end; i++) {
          path_str = path_str.append(this->path_[i]);
          if(i != (clip_end - 1))
            path_str += '/';
        }
        if(this->spostfix_ && clip_end >= this->path_length_)
          path_str += '/';
      }
      return path_str;
    }

    /*    [[nodiscard]] Pair<string, const string> path_prefixed(const uint8_t index = -1) const {
          const string p = -1 == index ? this->name() : this->path(index);
          const size_t split = p.find(':');
          return make_pair<string, const string>(split == string::npos ? EMPTY_CHARS : string(p.substr(0, split)),
                                                 split == string::npos ? p : string(p.substr(split)));
        }*/

    [[nodiscard]] bool has_path(const char *segment, const uint8_t start_index = 0) const {
      for(int i = start_index; i < path_length_; i++) {
        if(strcmp(path_[i], segment) == 0)
          return true;
      }
      return false;
    }

    [[nodiscard]] bool has_path() const { return this->path_length_ > 0; }

    [[nodiscard]] string path() const { return this->subpath(0, this->path_length_); }

    [[nodiscard]] const char *segment(const uint8_t segment) const {
      return (this->path_ && this->path_length_ > segment) ? this->path_[segment] : EMPTY_CHARS;
    }

    [[nodiscard]] fURI segment(const uint8_t segment, const fURI &replacement) const {
      string new_path = this->sprefix_ ? "/" : "";
      if(segment == 0 && this->path_length_ == 0)
        return this->path(replacement.toString());
      for(int i = 0; i < this->path_length_; i++) {
        new_path += ((i == segment) ? replacement.toString() : this->path_[i]);
        if(i < this->path_length_ - 1)
          new_path += "/";
      }
      if(this->spostfix_)
        new_path += "/";
      return this->path(new_path);
    }

    [[nodiscard]] bool starts_with(const fURI &prefix_path) const {
      if(this->sprefix_ != prefix_path.sprefix_ ||
         this->path_length_ < prefix_path.path_length_)
        return false;
      for(int i = 0; i < prefix_path.path_length_; i++) {
        if(strcmp(this->path_[i], prefix_path.path_[i]) != 0)
          return false;
      }
      return true;
    }

    [[nodiscard]] bool ends_with(const fURI &postfix_path) const {
      if(this->spostfix_ != postfix_path.spostfix_ ||
         this->path_length_ < postfix_path.path_length_)
        return false;
      for(int i = 0; i < postfix_path.path_length_; i++) {
        if(strcmp(this->path_[(this->path_length_ - 1) - i],
                  postfix_path.path_[(postfix_path.path_length_ - 1) - i]) != 0)
          return false;
      }
      return true;
    }


    [[nodiscard]] fURI path(const string &path) const {
      auto new_uri = fURI(*this);
      StringHelper::trim(path);
      for(uint8_t i = 0; i < this->path_length_; i++) {
        free(new_uri.path_[i]);
      }
      delete[] new_uri.path_;
      new_uri.path_length_ = 0;
      new_uri.path_ = new char *[FOS_MAX_PATH_SEGMENTS];
      const size_t len = strlen(path.c_str());
      char *dup = strndup(path.c_str(), len);
      auto ss = std::stringstream(dup);
      string segment;
      uint8_t i = 0;
      char c;
      while(ss.get(c)) {
        if(c == '\0' || isspace(c) || !isascii(c))
          break;
        if(c == '/') {
          if(segment.empty() && 0 == i) {
            new_uri.sprefix_ = true;
          } else {
            const size_t len2 = strlen(segment.c_str());
            new_uri.path_[i] = strndup(segment.c_str(), len2);
            i++;
          }
          segment.clear();
        } else {
          segment += c;
        }
      }
      StringHelper::trim(segment);
      if(segment.empty()) {
        new_uri.path_length_ = i;
        new_uri.spostfix_ = true;
      } else {
        const size_t len2 = strlen(segment.c_str());
        new_uri.path_[i] = strndup(segment.c_str(), len2);
        new_uri.path_length_ = i + 1;
        new_uri.spostfix_ = false;
      }
      if(path[path.length() - 1] == '/')
        new_uri.spostfix_ = true;
      free(dup);
      if(new_uri.host_ || new_uri.scheme_)
        new_uri.sprefix_ = true;
      return new_uri;
    }

    [[nodiscard]] string name() const {
      if(0 == this->path_length_)
        return EMPTY_CHARS;
      for(int i = this->path_length_ - 1; i >= 0; i--) {
        if(strlen(this->path_[i]) > 0) {
          //const size_t index = string(this->path_[i]).find_last_of(':'); // make find_last_of (indexing is goofy)
          //const size_t index = string::npos;
          //return index == string::npos
          //         ? string(this->scheme()).append(this->path_[i])
          //          : string(this->path_[i]).substr(index);
          return string(this->path_[i]);
        }
      }
      return "";
    }

    [[nodiscard]] bool empty() const {
      return this->path_length_ == 0 && !this->host_ && !this->scheme_ &&
             !this->user_ && !this->password_ && !this->query_;
    }

    [[nodiscard]] uint8_t path_length() const { return this->path_length_; }

    /// QUERY
    using IntCoefficient = std::pair<int, int>;
    using DomainRange = Quad<fURI, IntCoefficient, fURI, IntCoefficient>;

    [[nodiscard]] DomainRange dom_rng() const {
      const std::vector<string> dom_coeff_str = this->query_values(FOS_DOM_COEF);
      const std::vector<string> rng_coeff_str = this->query_values(FOS_RNG_COEF);
      return {
          this->query_value(FOS_DOMAIN).value(),
          IntCoefficient(stoi(dom_coeff_str.at(0)), stoi(dom_coeff_str.at(1))),
          this->query_value(FOS_RANGE).value(),
          IntCoefficient(stoi(rng_coeff_str.at(0)), stoi(rng_coeff_str.at(1)))};
    }


    [[nodiscard]] fURI dom_rng(const fURI &domain, const std::pair<FOS_INT_TYPE,FOS_INT_TYPE> &domain_coeff,
                               const fURI &range, const std::pair<FOS_INT_TYPE,FOS_INT_TYPE> &range_coeff) const {
      // TODO: string current_query = this->query();
      return this->query({
          {FOS_DOMAIN, domain.no_query().toString()},
          {FOS_DOM_COEF, to_string(domain_coeff.first).append(",").append(to_string(domain_coeff.second))},
          {FOS_RANGE, range.no_query().toString()},
          {FOS_RNG_COEF, to_string(range_coeff.first).append(",").append(to_string(range_coeff.second))}});
    }

    [[nodiscard]] bool has_coefficient() const {
      return nullptr != this->coefficient_;
    }

    [[nodiscard]] const char *coefficient() const {
      return this->coefficient_ ? this->coefficient_ : "";
    }

    [[nodiscard]] std::pair<int, int> coefficients() const {
      if(!this->coefficient_)
        return make_pair<int, int>(1, 1);
      const auto coeff_str = string(this->coefficient_);
      if(this->coefficient_[coeff_str.length() - 1] == ',')
        return make_pair<int, int>(std::stoi(coeff_str.substr(0, coeff_str.length() - 2)),
                                   INT_MAX);
      if(string::npos == coeff_str.find(',')) {
        int d = std::stoi(coeff_str);
        const auto pair = std::pair(d, d);
        return pair;
      }
      const std::vector<int> pairs = StringHelper::tokenize<int>(',', coeff_str, [](const string &d) {
        return std::stoi(d);
      });
      const auto pair = std::pair(pairs.at(0), pairs.at(1));
      return pair;
    }

    [[nodiscard]] fURI coefficient(const char *coefficient) const {
      auto new_uri = fURI(*this);
      FOS_SAFE_FREE(new_uri.coefficient_);
      new_uri.coefficient_ = nullptr == coefficient || 0 == strlen(coefficient) ? nullptr : strdup(coefficient);
      return new_uri;
    }

    [[nodiscard]] fURI coefficient(const int low, const int high) const {
      auto new_uri = fURI(*this);
      FOS_SAFE_FREE(new_uri.coefficient_);
      if(low == INT_MIN)
        new_uri.coefficient_ = high == INT_MAX ? strdup(",") : strdup(to_string(high).insert(0, ",").c_str());
      else if(high == INT_MAX)
        new_uri.coefficient_ = strdup((to_string(low) + ",").c_str());
      else if(low == high)
        new_uri.coefficient_ = strdup(to_string(low).c_str());
      else
        new_uri.coefficient_ = strdup((to_string(low) + "," + to_string(high)).c_str());
      return new_uri;
    }

    [[nodiscard]] const char *query() const { return this->query_ ? this->query_ : ""; }

    [[nodiscard]] bool has_query(const char *key = nullptr) const {
      if(!this->query_ || 0 == strlen(this->query_))
        return false;
      if(nullptr == key)
        return true;
      return this->query_value(key).has_value();
    }

    static void encode_query(string &query) {
      if(query.empty())
        return;
      //StringHelper::replace(&query, "=>", "%20%50");
      //StringHelper::replace(&query, "<=", "%20%60");
    }

    static void decode_query(string &query) {
      if(query.empty())
        return;
      //StringHelper::replace(&query, "%20%50", "=>");
      //StringHelper::replace(&query, "%20%60", "<=");
    }

    [[nodiscard]] fURI query(const char *query) const {
      auto new_uri = fURI(*this);
      FOS_SAFE_FREE(new_uri.query_);
      new_uri.query_ = nullptr == query || 0 == strlen(query) ? nullptr : strdup(query);
      return new_uri;
    }

    [[nodiscard]] fURI no_query() const {
      const fURI furi_no_query = this->query("");
      return furi_no_query;
    }

    [[nodiscard]] fURI query(const List<Pair<string, string>> &key_values) const {
      string query_string;
      for(const auto &[k,v]: key_values) {
        //  encode_query(k);
        //  encode_query(v);
        if(v.empty())
          query_string.append(k).append("&");
        else
          query_string.append(k).append("=").append(v).append("&");
      }
      query_string = query_string.substr(0, query_string.length() - 1);
      return this->query(query_string.c_str());
    }

    [[nodiscard]] List<Pair<string, string>> query_values() const {
      if(!this->query_)
        return {};
      std::vector<std::pair<string, string>> key_values;
      for(const string &pairs: StringHelper::tokenize('&', this->query_)) {
        const std::vector<string> pair = StringHelper::tokenize('=', pairs);
        std::pair<string, string> split_pair = (pair.size() == 1)
                                                 ? std::make_pair(pair.at(0), string(""))
                                                 : std::make_pair(pair.at(0), pair.at(1));
        //  decode_query(split_pair.first);
        // decode_query(split_pair.second);
        key_values.emplace_back(split_pair);
      }
      return key_values;
    }

    template<typename T = std::string>
    [[nodiscard]] List<T> query_values(const char *key,
                                       const Function<string, T> &transformer = [](const string &s) {
                                         return s;
                                       }) const {
      const Option<string> v = this->query_value(key);
      if(!v.has_value())
        return {};
      auto ss = std::stringstream(v.value());
      string token;
      List<T> list;
      while(!(token = StringHelper::next_token(',', &ss)).empty()) {
        StringHelper::trim(token);
        //decode_query(token);
        list.push_back(transformer(token));
      }
      return list;
    }

    template<typename T = std::string>
    [[nodiscard]] Option<T> query_value(const char *key,
                                        const Function<string, T> &transformer = [](const string &s) {
                                          return s;
                                        }) const {
      if(!this->query_)
        return {};
      const char *index = strstr(this->query_, key);
      if(!index)
        return {};
      size_t counter = 0;
      char c = index[strlen(key) + counter];
      if(c != '=')
        return {transformer("")};
      counter++;
      c = index[strlen(key) + counter];
      string value;
      while(c != '\0' && c != '&') {
        value += c;
        counter++;
        c = index[strlen(key) + counter];
      }
      StringHelper::trim(value);
      //decode_query(value);
      return {transformer(value)};
    }

    /// FRAGMENT
    /*[[nodiscard]] const char *fragment() const { return this->fragment_ ? this->fragment_ : ""; }

    [[nodiscard]] fURI fragment(const char *fragment) const {
      auto new_uri = fURI(*this);
      new_uri.fragment_ = 0 == strlen(fragment) ? nullptr : fragment;
      return new_uri;
    }*/

    ////////////////////////////////////////////////////////////////

    [[nodiscard]] fURI extend(const fURI &furi_path) const { return this->extend(furi_path.path().c_str()); }

    [[nodiscard]] fURI extend(const char *extension) const {
      if(strlen(extension) == 0) {
        auto new_uri = fURI(*this);
        new_uri.spostfix_ = true;
        return new_uri;
      }
      if(this->path().empty() || this->path() == "/") {
        fURI new_furi = this->path(extension);
        new_furi.sprefix_ = true;
        return new_furi;
      } else {
        auto new_path = string(this->path());
        if(!this->spostfix_ && extension[0] != '/')
          new_path += '/';
        if(new_path.empty())
          return this->path(extension);
        new_path += extension;
        return this->path(new_path);
      }
    }

    [[nodiscard]] fURI retract(const int steps = 1) const {
      /// pathless clone
      auto new_uri = fURI(*this);
      for(uint8_t i = 0; i < new_uri.path_length_; i++) {
        free(new_uri.path_[i]);
      }
      delete[] new_uri.path_;
      /////////////////////////////////////////
      new_uri.path_length_ = this->path_length_ > steps ? this->path_length_ - steps : 0;
      new_uri.path_ = new char *[new_uri.path_length_];
      for(uint8_t i = 0; i < new_uri.path_length_; i++) {
        new_uri.path_[i] = strdup(this->path_[i]);
      }
      if(new_uri.path_length_ == 0) {
        new_uri.spostfix_ = false;
        new_uri.sprefix_ = false;
      }
      return new_uri.path_length_ > 1 && 0 == strcmp(new_uri.path_[new_uri.path_length_ - 1], COMPONENT_SEPARATOR)
               ? new_uri.retract()
               : new_uri;
    }

    [[nodiscard]] fURI head() const {
      if(this->empty())
        return *this;
      return fURI(this->segment(0));
    }

    [[nodiscard]] fURI pretract(const fURI &prefix) const {
      if(!this->starts_with(prefix))
        return *this;
      else {
        return this->pretract(prefix.path_length_);
      }
    }

    [[nodiscard]] fURI retract(const fURI &prefix) const {
      if(!this->ends_with(prefix))
        return *this;
      else {
        return this->retract(prefix.path_length_);
      }
    }

    [[nodiscard]] fURI pretract(const int steps = 1) const {
      /// pathless clone
      auto new_uri = fURI(*this);
      for(uint8_t i = 0; i < new_uri.path_length_; i++) {
        free(new_uri.path_[i]);
      }
      delete[] new_uri.path_;
      /////////////////////////////////////////
      new_uri.path_length_ = (this->path_length_ > steps) ? ((this->path_length_ - steps)) : 0;
      new_uri.path_ = new char *[new_uri.path_length_];
      for(uint8_t i = steps; i < this->path_length_; i++) {
        new_uri.path_[i - steps] = strdup(this->path_[i]);
      }
      if(new_uri.path_length_ == 0) {
        new_uri.spostfix_ = false;
        new_uri.sprefix_ = false;
      }
      return new_uri.path_length_ > 1 && 0 == strcmp(new_uri.path_[0], COMPONENT_SEPARATOR)
               ? new_uri.pretract()
               : new_uri;
    }

    [[nodiscard]] fURI prepend(const fURI &furi_path) const { return this->prepend(furi_path.path().c_str()); }

    [[nodiscard]] fURI prepend(const char *extension) const {
      if(strlen(extension) == 0) {
        auto new_uri = fURI(*this);
        new_uri.sprefix_ = true;
        return new_uri;
      }
      auto new_path = string(extension);
      const string old_path = this->path();
      if(old_path[0] != '/' && new_path[new_path.length() - 1] != '/')
        new_path += '/';
      new_path = new_path += (old_path[0] == '/' && new_path[new_path.length() - 1] == '/'
                                ? old_path.substr(1)
                                : old_path);

      return this->path(new_path);
    }

    [[nodiscard]] bool has_wildcard(const char wildcard = '#') const {
      for(uint8_t i = 0; i < this->path_length_; i++) {
        if(strstr(this->path_[i], &wildcard)) {
          return true;
        }
      }
      return false;
    }

    [[nodiscard]] fURI retract_pattern() const {
      for(uint8_t i = 0; i < this->path_length_; i++) {
        if(strcmp(this->segment(i), "+") == 0 || 0 == strcmp(this->segment(i), "#")) {
          auto retracted = fURI(*this);
          retracted.path_length_ = i;
          for(uint8_t j = i; j < this->path_length_; j++) {
            free(retracted.path_[j]);
          }
          return retracted;
        }
      }
      return *this;
    }

    [[nodiscard]] fURI as_node() const {
      if(!this->spostfix_)
        return *this;
      auto f = fURI(*this);
      f.spostfix_ = false;
      return f;
    }

    [[nodiscard]] fURI as_branch() const {
      if(this->spostfix_)
        return *this;
      auto f = fURI(*this);
      f.spostfix_ = true;
      return f;
    }

    [[nodiscard]] bool is_subfuri_of(const fURI &other) const {
      const string this_string = this->toString();
      const string other_string = other.toString();
      return other_string.length() >= this_string.length() &&
             other_string.substr(0, this_string.length()) == this_string;
    }

    [[nodiscard]] bool is_relative() const {
      const char first = this->toString()[0];
      return first == '.' || first == ':'; //|| (first != '/' && !this->scheme_ && !this->host_);
    }

    [[nodiscard]] fURI as_relative() const {
      if(this->path_length_ > 0 && this->path_[0][0] == '/') {
        return this->path(this->path().substr(1));
      }
      return *this;
    }

    [[nodiscard]] bool is_branch() const { return this->spostfix_ || (this->path_length_ == 0 && this->sprefix_); }

    [[nodiscard]] bool is_node() const { return !this->spostfix_; }

    [[nodiscard]] bool is_scheme_path() const {
      return this->scheme_ && this->path_length_ > 0 && !this->host_ && !this->user_ && !this->password_;
    }

    [[nodiscard]] bool has_components() const {
      if(0 == this->path_length_)
        return false;
      for(uint8_t i = 0; i < this->path_length_; i++) {
        if(0 == strcmp("::", this->path_[i]))
          return true;
      }
      return false;
    }

    [[nodiscard]] fURI add_component(const fURI &component) const {
      return this->extend("::").extend(component);
    }

    [[nodiscard]] List<fURI> components() const {
      List<fURI> comps = {""};
      for(uint8_t i = 0; i < this->path_length_; i++) {
        if(0 == strcmp("::", this->path_[i])) {
          comps.emplace_back("");
        } else {
          fURI x = comps.back();
          fURI next = x.empty() ? fURI(this->sprefix_ ? "/" : "").append(this->path_[i]) : x.extend(this->path_[i]);
          comps.pop_back();
          comps.push_back(next);
        }
      }
      if(comps.back().empty())
        comps.pop_back();
      return comps;
    }

    [[nodiscard]] fURI remove_subpath(const string &subpath, const bool forward = true) const {
      string new_path = this->toString();
      StringHelper::replace(&new_path, subpath, "", forward);
      return fURI(new_path);
    }

    [[nodiscard]] fURI append(const fURI &other) const {
      return fURI(this->toString().append(other.toString()));
    }

    [[nodiscard]] virtual fURI resolve(const fURI &other) const {
      if(this->is_pattern() && other.matches(*this))
        return other;
      ///////////////////////////////////////////////////////////////
      ////////////  mm-ADT specific resolution pattern //////////////
      ///////////////////////////////////////////////////////////////
      ///         /abc ~> :xyz => /abc:xyz  NOT /:xyz             ///
      ///////////////////////////////////////////////////////////////
      if((!other.toString().empty() && other.toString()[0] == ':') ||
         (!this->toString().empty() && this->toString()[this->toString().length() - 1] == ':'))
        return fURI(this->retract_pattern().toString() + other.toString()).query(other.query());
      if(other.is_scheme_path()) {
        if(!this->scheme_)
          return other.sprefix_ ? other : this->retract_pattern().extend(other.toString().c_str()).query(other.query());
        else if(strcmp(this->scheme_, other.scheme_) != 0) {
          return this->retract_pattern().extend(other.toString().c_str()).query(other.query());
        }
      }
      ///////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////
      if(other.path_length_ == 0)
        return *this;
      const bool path_end_slash = this->path()[this->path().length() - 1] == '/' || this->spostfix_;
      const bool path_start_slash = this->path()[0] == '/' || this->sprefix_;
      if(other.path().find('.') == string::npos) {
        const auto other_path_chars = std::unique_ptr<char, void (*)(void *)>(strdup(other.path().c_str()), free);
        const bool other_start_slash = other_path_chars.get()[0] == '/';
        if(path_end_slash || this->path_length_ == 0)
          return (other_start_slash ? this->path(other_path_chars.get()) : this->extend(other_path_chars.get())).query(
              other.query());
        if(other_start_slash)
          return this->path(other_path_chars.get()).query(other.query());
        if(this->path_length_ == 1)
          return (this->path((path_start_slash) ? (string("/") + other_path_chars.get()) : other_path_chars.get())).
              query(other.query());
        return this->retract().extend(other_path_chars.get()).query(other.query());
      }
      fURI *temp = path_end_slash || this->path_length_ == 0 ? new fURI(*this) : new fURI(this->retract());
      for(uint8_t i = 0; i < other.path_length_; i++) {
        if(strcmp(other.segment(i), "..") == 0) {
          const fURI *temp2 = new fURI(*temp);
          delete temp;
          temp = temp2->path_length() > 0 ? new fURI(temp2->retract()) : new fURI(*temp2);
          delete temp2;
        } else if(strcmp(other.segment(i), ".") != 0) {
          const fURI *temp2 = new fURI(*temp);
          delete temp;
          temp = new fURI(temp2->extend(other.segment(i)));
          delete temp2;
        }
        if(i == other.path_length_ - 1)
          temp->spostfix_ = other.spostfix_;
      }
      const auto ret = fURI(*temp);
      delete temp;
      return ret.query(other.query());
    }

    [[nodiscard]] virtual bool is_pattern() const {
      const string temp = this->toString();
      bool result = temp.find('#') != string::npos || temp.find('+') != string::npos;
      return result;
    }

    [[nodiscard]] virtual bool bimatches(const fURI &other) const {
      return this->matches(other) || other.matches(*this);
    }

    [[nodiscard]] virtual bool matches(const fURI &pattern) const {
      //if (this->has_query() || pattern.has_query()) {
      //  return this->query("").matches(pattern.query(""));
      // }
      if(this->equals(pattern))
        return true;
      const string pattern_str = pattern.toString();
      // if (pattern_str[0] == ':' && this->toString()[0] == ':')
      //   return fURI(this->toString().substr(1)).matches(fURI(pattern_str.substr(1)));
      if(pattern_str[0] == ':')
        return this->name() == pattern_str; // ./blah/:setup ~ :setup
      if(pattern.toString() == "#")
        return true;
      if(pattern_str.find('+') == string::npos && pattern_str.find('#') == string::npos)
        return this->toString() == pattern_str;
      if(strcmp(pattern.scheme(), "#") == 0)
        return true;
      if((strlen(this->scheme()) == 0 && strlen(pattern.scheme()) != 0) ||
         (strcmp(pattern.scheme(), "+") != 0 && strcmp(this->scheme(), pattern.scheme()) != 0))
        return false;
      if(strcmp(pattern.host(), "#") == 0)
        return true;
      if((strlen(this->host()) == 0 && strlen(pattern.host()) != 0) ||
         (strcmp(pattern.host(), "+") != 0 &&
          strcmp(this->host(), pattern.host()) !=
          0)) // TODO: this should be just to authority as user:pass can't be wildcard matched ??
        return false;
      if(strcmp(pattern.user(), "#") == 0)
        return true;
      if(strcmp(pattern.user(), "+") != 0 && strcmp(this->user(), pattern.user()) != 0)
        return false;
      if(strcmp(pattern.password(), "#") == 0)
        return true;
      if(strcmp(pattern.password(), "+") != 0 && strcmp(this->password(), pattern.password()) != 0)
        return false;
      for(uint8_t i = 0; i < pattern.path_length(); i++) {
        if(strcmp(pattern.segment(i), "#") == 0)
          return true;
        if(0 == i && (this->sprefix_ != pattern.sprefix_))
          return false;
        if(strcmp(pattern.segment(i), "+") == 0) {
          if(strcmp(this->segment(i), "#") == 0)
            return false;
          if((i == (pattern.path_length_ - 1)) && this->is_branch() != pattern.is_branch())
            return false;
          if(this->path_length_ <= i && this->spostfix_)
            return true;
        }
        if(this->path_length_ <= i)
          return false;
        if((strlen(this->segment(i)) == 0 && strlen(pattern.segment(i)) != 0) ||
           (strcmp(pattern.segment(i), "+") != 0 && strcmp(this->segment(i), pattern.segment(i)) != 0))
          return false;
      }
      return (0 == strlen(pattern.query()) || this->query() == pattern.query()) && this->path_length_ == pattern.
             path_length();
    }


    fURI &operator=(const fURI &other) {
      if(&other == this)
        return *this;
      free((void *) this->scheme_);
      this->scheme_ = other.scheme_ ? strdup(other.scheme_) : nullptr;
      free((void *) this->host_);
      this->host_ = other.host_ ? strdup(other.host_) : nullptr;
      this->port_ = other.port_;
      free((void *) this->user_);
      this->user_ = other.user_ ? strdup(other.user_) : nullptr;
      free((void *) this->password_);
      this->password_ = other.password_ ? strdup(other.password_) : nullptr;
      free((void *) this->coefficient_);
      this->coefficient_ = other.coefficient_ ? strdup(other.coefficient_) : nullptr;
      free((void *) this->query_);
      this->query_ = other.query_ ? strdup(other.query_) : nullptr;
      this->path_length_ = other.path_length_;
      for(size_t i = 0; i < this->path_length_; i++) {
        free(this->path_[i]);
      }
      delete[] path_;
      this->path_ = new char *[other.path_length_]();
      if(other.path_) {
        for(uint8_t i = 0; i < other.path_length_; i++) {
          this->path_[i] = strdup(other.path_[i]);
        }
      }
      this->sprefix_ = other.sprefix_;
      this->spostfix_ = other.spostfix_;
      return *this;
    }

    [[nodiscard]] bool headless() const {
      const char first = this->toString()[0];
      return first == '.' || first == ':' || (first != '/' && !this->scheme_ && !this->host_);
    }

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    virtual ~fURI() {
      free((void *) this->scheme_);
      free((void *) this->host_);
      free((void *) this->user_);
      free((void *) this->password_);
      free((void *) this->coefficient_);
      free((void *) this->query_);
      // free((void *) this->fragment_);
      for(size_t i = 0; i < this->path_length_; i++) {
        free(path_[i]);
      }
      delete[] path_;
    }

    fURI(const fURI &other) {
      this->scheme_ = other.scheme_ ? strdup(other.scheme_) : nullptr;
      this->user_ = other.user_ ? strdup(other.user_) : nullptr;
      this->password_ = other.password_ ? strdup(other.password_) : nullptr;
      this->host_ = other.host_ ? strdup(other.host_) : nullptr;
      this->port_ = other.port_;
      this->sprefix_ = other.sprefix_;
      this->spostfix_ = other.spostfix_;
      this->path_length_ = other.path_length_;
      this->coefficient_ = other.coefficient_ ? strdup(other.coefficient_) : nullptr;
      this->query_ = other.query_ ? strdup(other.query_) : nullptr;
      // this->fragment_ = other.fragment_ ? strdup(other.fragment_) : nullptr;
      this->path_ = new char *[other.path_length_]();
      if(other.path_) {
        for(uint8_t i = 0; i < other.path_length_; i++) {
          this->path_[i] = strdup(other.path_[i]);
        }
      }
    }

    fURI(const string &uri_string) :
      fURI(uri_string.c_str()) {
    }

    fURI(const char *uri_chars) {
      if(strlen(uri_chars) == 0)
        return;
      const char *dups = strdup(uri_chars);
      /*for (size_t i = 0; i < strlen(dups); i++) {
        if (dups[i] == '#' && i != strlen(dups) - 1) {
          const string temp = string(dups);
          free((void *) dups);
      throw fError("Recurssive !b#!! wildcard must be the last character: {}\n", temp.c_str());
        }
      }*/
      try {
        auto ss = std::stringstream(dups);
        string token;
        auto part = URI_PART::SCHEME;
        bool hasUserInfo = strchr(dups, '@') != nullptr;
        bool foundAuthority = false;
        while(!ss.eof()) {
          char c = static_cast<char>(ss.get());
          if(!isascii(c) || isspace(c) || c < 32 || c > 126)
            continue;
          if(!foundAuthority && c == '/' && ss.peek() == '/') {
            foundAuthority = true;
            if(part == URI_PART::SCHEME || part == URI_PART::USER) {
              part = URI_PART::USER;
              ss.get();
            }
          } else if(c == ':' && ss.peek() == ':') {
            ss.get(); //drop :
            if(!this->path_)
              this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
            if(!token.empty()) {
              this->path_[this->path_length_] = strdup(token.c_str());
              this->path_length_ = this->path_length_ + 1;
              token.clear();
            }
            this->path_[this->path_length_] = strdup("::");
            this->path_length_ = this->path_length_ + 1;
            if(ss.peek() == '/') {
              ss.get(); // drop /
            }
            part = URI_PART::PATH;
          } else if(c == ':') {
            if(part == URI_PART::SCHEME) {
              if(!token.empty()) {
                this->scheme_ = strdup(token.c_str());
                token.clear();
              } else
                token += c;
              part = URI_PART::USER;
            } else if(part == URI_PART::USER) {
              if(hasUserInfo) {
                this->user_ = strdup(token.c_str());
                part = URI_PART::PASSWORD;
              } else {
                this->host_ = strdup(token.c_str());
                part = URI_PART::PORT;
              }
              token.clear();
            } else if(part == URI_PART::HOST) {
              this->host_ = strdup(token.c_str());
              part = URI_PART::PORT;
              token.clear();
            } else {
              token += c;
            }
          } else if(c == '@') {
            if(part == URI_PART::USER || part == URI_PART::PASSWORD) {
              if(this->user_) {
                this->password_ = strdup(token.c_str());
              } else {
                this->user_ = strdup(token.c_str());
              }
              part = URI_PART::HOST;
              token.clear();
            } else {
              token += c;
            }
          } else if(c == '/') {
            if(part == URI_PART::PORT) {
              if(!StringHelper::is_integer(token))
                throw fError("uri port not an int: %s", token.c_str());
              this->port_ = stoi(token);
              part = URI_PART::PATH;
              this->sprefix_ = true;
              token.clear();
            } else if(part == URI_PART::SCHEME || part == URI_PART::HOST || part == URI_PART::USER ||
                      part == URI_PART::PASSWORD) {
              if(foundAuthority) {
                this->host_ = strdup(token.c_str());
                part = URI_PART::PATH;
                this->sprefix_ = true;
              } else {
                if(!token.empty()) {
                  // TODO: what about empty components?
                  if(!this->path_)
                    this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
                  this->path_[this->path_length_] = strdup(token.c_str());
                  this->path_length_ = this->path_length_ + 1;
                  check_path_length(uri_chars);
                } else {
                  this->sprefix_ = true;
                }
                part = URI_PART::PATH;
              }
              token.clear();
            } else if(part == URI_PART::PATH) {
              if(!this->path_)
                this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
              this->path_[this->path_length_] = strdup(token.c_str());
              this->path_length_ = this->path_length_ + 1;
              check_path_length(uri_chars);
              this->spostfix_ = true;
              token.clear();
            } else {
              token += c;
            }
          } else if(part == URI_PART::PATH && c == '.' && ss.peek() == '.') { // TODO: fix
            ss.get(); // drop .
            this->spostfix_ = (ss.peek() == '/');
            if(this->spostfix_) {
              ss.get();
            }
            if(this->path_) {
              if(this->path_[this->path_length_ - 1][0] == '.') {
                this->path_[this->path_length_++] = strdup("..");
              } else {
                free(this->path_[--this->path_length_]);
                this->path_[this->path_length_] = nullptr;
              }
            } else {
              this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
              this->path_[this->path_length_++] = strdup("..");
            }
          } else if(c == '$') {
            if(part == URI_PART::PATH || part == URI_PART::SCHEME) {
              if(!token.empty()) {
                if(!this->path_)
                  this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
                this->path_[this->path_length_] = strdup(token.c_str());
                this->path_length_ = this->path_length_ + 1;
                check_path_length(uri_chars);
              }
              part = URI_PART::COEFFICIENT;
              // this->query_ = strdup("");
              token.clear();
            } else if(part == URI_PART::HOST || part == URI_PART::USER) {
              this->host_ = strdup(token.c_str());
              part = URI_PART::COEFFICIENT;
              token.clear();
            } else {
              token += c;
            }
          } else if(c == '?') {
            if(part == URI_PART::COEFFICIENT) {
              if(!token.empty()) {
                if(this->coefficient_)
                  free((void *) this->coefficient_);
                this->coefficient_ = strdup(token.c_str());
              }
              part = URI_PART::QUERY;
              token.clear();
            } else if(part == URI_PART::PATH || part == URI_PART::SCHEME) {
              if(!token.empty()) {
                if(!this->path_)
                  this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
                this->path_[this->path_length_] = strdup(token.c_str());
                this->path_length_ = this->path_length_ + 1;
                check_path_length(uri_chars);
              }
              part = URI_PART::QUERY;
              // this->query_ = strdup("");
              token.clear();
            } else if(part == URI_PART::HOST || part == URI_PART::USER) {
              this->host_ = strdup(token.c_str());
              part = URI_PART::QUERY;
              token.clear();
            } else {
              token += c;
            }
          } else if(!isspace(c) && isascii(c)) {
            if(part != URI_PART::QUERY && part != URI_PART::COEFFICIENT)
              this->spostfix_ = false;
            token += c;
          }
          if((ss.eof() && c == '/'))
            this->spostfix_ = true;
        }
        StringHelper::trim(token);
        if(!token.empty()) {
          if((!foundAuthority &&
              part != URI_PART::COEFFICIENT &&
              part != URI_PART::QUERY) ||
             part == URI_PART::PATH || part == URI_PART::SCHEME) {
            if(!this->path_)
              this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
            this->path_[this->path_length_] = strdup(token.c_str());
            this->path_length_ = this->path_length_ + 1;
            check_path_length(uri_chars);
          } else if(part == URI_PART::HOST || part == URI_PART::USER) {
            this->host_ = strdup(token.c_str());
          } else if(part == URI_PART::PORT) {
            if(!StringHelper::is_integer(token))
              throw fError("!yuri!! port not an !bint!!: %s", token.c_str());
            this->port_ = stoi(token);
          } else if(part == URI_PART::COEFFICIENT) {
            free((void *) this->coefficient_);
            this->coefficient_ = strdup(token.c_str());
          } else if(part == URI_PART::QUERY) {
            free((void *) this->query_);
            this->query_ = strdup(token.c_str());
          } // else if (part == URI_PART::FRAGMENT) {
          // this->_fragment = strdup(token.c_str());
          // }
        }
        if(this->path_) {
          for(uint8_t i = 0; i < this->path_length_; i++) {
            if(this->path_[i][0] == '#' && i != this->path_length_ - 1) {
              throw fError("only the last path segment can contain the recursive !b#!! wildcard: %s",
                           this->path().c_str());
            }
            if(!this->spostfix_) {
              if(char last = this->path_[this->path_length_ - 1][strlen(this->path_[this->path_length_ - 1]) - 1];
                /* last == '.' || */last == '_' || last == '=') {
                throw fError("furis can not end with chars !g[!y_!c=!g]!!: %s", this->name().c_str());
              }
            }
          }
        }
      } catch(const std::exception &) {
        FOS_SAFE_FREE(dups);
        throw;
      }
      FOS_SAFE_FREE(dups);
    }

    bool operator<(const fURI &other) const { return this->toString() < other.toString(); }

    bool operator!=(const fURI &other) const { return !this->equals(other); }

    bool operator==(const fURI &other) const {
      return this->equals(other);
    }

    [[nodiscard]] bool equals(const fURI &other) const {
      if(this->path_length_ != other.path_length_)
        return false;
      for(int i = 0; i < this->path_length_; i++) {
        if(!StringHelper::char_ptr_equal(this->path_[i], other.path_[i]))
          return false;
      }
      return this->spostfix_ == other.spostfix_ &&
             this->sprefix_ == other.sprefix_ &&
             StringHelper::char_ptr_equal(this->coefficient_, other.coefficient_) &&
             StringHelper::char_ptr_equal(this->query_, other.query_) &&
             StringHelper::char_ptr_equal(this->scheme_, other.scheme_) &&
             StringHelper::char_ptr_equal(this->host_, other.host_) &&
             this->port_ == other.port_ &&
             StringHelper::char_ptr_equal(this->user_, other.user_) &&
             StringHelper::char_ptr_equal(this->password_, other.password_);
    }

    [[nodiscard]] string toString() const {
      string uri;
      if(this->scheme_)
        uri.append(this->scheme_).append(":");
      if(this->host_ || this->user_) {
        uri.append("//");
        if(this->user_) {
          uri.append(this->user_);
          if(this->password_)
            uri.append(":").append(this->password_);
          uri.append("@");
        }
        if(this->host_) {
          uri.append(this->host_);
          if(this->port_ > 0)
            uri.append(":").append(std::to_string(this->port_));
        }
      }
      if(this->sprefix_)
        uri.append("/");
      for(int i = 0; i < this->path_length_; i++) {
        if(strlen(this->path_[i]) > 0)
          uri.append(this->path_[i]);
        if(i < (this->path_length_ - 1) || this->spostfix_)
          uri.append("/");
      }
      if(this->coefficient_)
        uri.append("$").append(this->coefficient_);
      if(this->query_)
        uri.append("?").append(this->query_);
      // if (this->_fragment)
      //   uri.append("#").append(this->_fragment);
      StringHelper::trim(uri);
      return uri;
    }

  private:
    void check_path_length(const char *self) const {
      if(this->path_length_ >= FOS_MAX_PATH_SEGMENTS)
        throw fError("!ymax path length!! of !r%i!! has been reached: !b%i!!", FOS_MAX_PATH_SEGMENTS, self);
    }
  };

  class ID final : public fURI {
  public:
    ID(const fURI &id) :
      fURI(id.toString()) {
    }

    ID(const string &furi_string) :
      ID(furi_string.c_str()) {
    }

    ID(const char *furi_characters) :
      fURI(furi_characters) {
      if(strchr(furi_characters, '#')) {
        throw fError("ids can not contain pattern symbols: !b#!!: %s", furi_characters);
      } else if(strchr(furi_characters, '+')) {
        throw fError("ids can not contain pattern symbols: !b+!!: %s", furi_characters);
      }
    }

    // const bool isPattern() const override { return false; }
  };

  class Pattern final : public fURI {
  public:
    Pattern(const Pattern &uri) :
      fURI(uri) {
    }

    Pattern(const fURI &uri) :
      fURI(uri) {
    }

    Pattern(const string &uri_string) :
      fURI(uri_string) {
    };

    Pattern(const char *uri_chars) :
      fURI(uri_chars) {
    };
  };

  using fURI_p = ptr<fURI>;
  using ID_p = ptr<ID>;
  using ID_wp = weak_ptr<ID>;
  using Pattern_p = ptr<Pattern>;
  using SourceID = ID;
  using TargetID = ID;

  ///////////////////////////////////////////////////
  ///////////////// TYPED FURI OBJ /////////////////
  //////////////////////////////////////////////////

  class Typed {
  public:
    virtual ~Typed() = default;

    ID_p tid;

    explicit Typed(const ID_p &type) :
      tid(type) {
    }

    explicit Typed(const ID &id) :
      Typed(make_shared<ID>(id)) {
    }
  };

  ////////////////////////////////////////////////////
  ///////////////// VALUED FURI OBJ /////////////////
  ///////////////////////////////////////////////////

  class Valued {
  public:
    ID_p vid;

    virtual ~Valued() = default;

    explicit Valued(const ID_p &id) :
      vid(id) {
      if(id && id->empty())
        vid = nullptr;
    }

    explicit Valued(const ID &id) :
      Valued(make_shared<ID>(id)) {
    }
  };

  struct furi_less : std::less<fURI> {
    auto operator()(const fURI &a, const fURI &b) const {
      return std::less<string>()(a.toString(), b.toString());
    }
  };


  struct furi_p_less : std::less<fURI_p> {
    auto operator()(const fURI_p &a, const fURI_p &b) const {
      return std::less<string>()(a->toString(), b->toString());
    }
  };

  struct furi_p_hash {
    size_t operator()(const fURI_p &furi) const { return std::hash<std::string>{}(furi->toString()); }
  };

  struct furi_p_equal_to {
    bool operator()(const fURI_p &a, const fURI_p &b) const { return a->equals(*b); }
  };

  struct furi_hash {
    size_t operator()(const fURI &furi) const { return std::hash<std::string>{}(furi.toString()); }
  };

  struct furi_equal_to {
    bool operator()(const fURI &a, const fURI &b) const { return a.equals(b); }
  };

  [[maybe_unused]] static fURI_p furi_p(const char *id_chars) { return make_shared<fURI>(id_chars); }

  [[maybe_unused]] static fURI_p furi_p(const fURI &furi) { return make_shared<fURI>(furi); }

  [[maybe_unused]] static ID_p id_p(const char *id_chars) { return make_shared<ID>(id_chars); }

  [[maybe_unused]] static ID_p id_p(const ID &id) { return make_shared<ID>(id); }

  [[maybe_unused]] static ID_p id_p(const fURI &id) { return make_shared<ID>(id); }

  [[maybe_unused]] static ID_p id_p(const fURI_p &id) { return make_shared<ID>(*id); }

  [[maybe_unused]] static Pattern_p p_p(const char *pattern_chars) { return make_shared<Pattern>(pattern_chars); }

  [[maybe_unused]] static Pattern_p p_p(const Pattern &pattern) { return make_shared<Pattern>(pattern); }

  [[maybe_unused]] static Pattern_p p_p(const fURI &pattern) { return make_shared<Pattern>(pattern); }


  using ValueO = ID;
  using ValueO_p = ID_p;
  using TypeO_p = ID_p;

  /*class vID : public std::variant<ID, ID_p> {
  public:
    [[nodiscard]] ID_p as_p() const {
      return std::holds_alternative<ID_p>(*this) ? std::get<ID_p>(*this) : id_p(std::get<ID>(*this));
    }

    [[nodiscard]] ID as_() const {
      return std::holds_alternative<ID>(*this) ? std::get<ID>(*this) : *id_p(std::get<ID_p>(*this));
    }
  };*/

  inline std::ostream &operator <<(std::ostream &os, const fURI &value) {
    os << value.toString();
    return os;
  }
} // namespace fhatos

#endif
