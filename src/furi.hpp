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

#ifndef FOS_MAX_PATH_SEGMENTS
#define FOS_MAX_PATH_SEGMENTS 12
#endif
#define COMPONENT_SEPARATOR STR(::)
#define COEFFICIENT_SEPARATOR STR($)

#include <ostream>
#include "fhatos.hpp"

namespace fhatos {
  enum class URI_PART { SCHEME, USER, PASSWORD, HOST, PORT, PATH, COEFFICIENT, /*FRAGMENT,*/ QUERY };

  // scheme://user:password@host:port/path...
  const static auto URI_PARTS = Enums<URI_PART>{
      {URI_PART::SCHEME, "scheme"},
      {URI_PART::USER, "user"},
      {URI_PART::PASSWORD, "password"},
      {URI_PART::HOST, "host"},
      {URI_PART::PORT, "port"},
      {URI_PART::PATH, "path"},
      /* {URI_PART::FRAGMENT, "fragment"},*/ {URI_PART::QUERY, "query"},
  };

  class fURI {
  protected:
    string scheme_;
    string user_;
    string password_;
    string host_;
    uint16_t port_ = 0;
    std::vector<string> path_;
    bool sprefix_ = false;
    bool spostfix_ = false;
    string query_;
    string coefficient_;
    ////////////////////////////////////////
    void delete_path();

  public:
    [[nodiscard]] string scheme() const;

    [[nodiscard]] fURI scheme(const char *scheme);

    [[nodiscard]] bool has_scheme() const;

    /// USER
    [[nodiscard]] string user() const;

    fURI user(const char *user) const;

    [[nodiscard]] bool has_user() const;

    /// PASSWORD
    [[nodiscard]] const char *password() const;

    fURI password(const char *password) const;

    [[nodiscard]] bool has_password() const;

    /// HOST
    [[nodiscard]] const char *host() const;

    fURI host(const char *host) const;

    [[nodiscard]] bool has_host() const;

    /// PORT
    [[nodiscard]] uint16_t port() const;

    [[nodiscard]] fURI port(uint16_t port) const;

    [[nodiscard]] bool has_port() const;

    /// AUTHORITY
    [[nodiscard]] string authority() const;

    [[nodiscard]] fURI authority(const char *authority) const;

    /// PATH
    [[nodiscard]] string subpath(uint8_t start, uint8_t end = UINT8_MAX) const;

    [[nodiscard]] bool has_path(const char *segment, uint8_t start_index = 0) const;

    [[nodiscard]] bool has_path() const;

    [[nodiscard]] string path() const;

    [[nodiscard]] const char *segment(uint8_t segment) const;

    [[nodiscard]] fURI segment(uint8_t segment, const fURI &replacement) const;

    [[nodiscard]] bool starts_with(const fURI &prefix_path) const;

    [[nodiscard]] bool ends_with(const fURI &postfix_path) const;


    [[nodiscard]] fURI path(const string &path) const;

    [[nodiscard]] string name() const;

    [[nodiscard]] bool empty() const;

    [[nodiscard]] uint8_t path_length() const;

    /// QUERY
    using IntCoefficient = std::pair<int, int>;
    using DomainRange = Quad<fURI, IntCoefficient, fURI, IntCoefficient>;

    [[nodiscard]] DomainRange dom_rng() const;


    [[nodiscard]] fURI dom_rng(const fURI &domain, const std::pair<FOS_INT_TYPE, FOS_INT_TYPE> &domain_coeff,
                               const fURI &range, const std::pair<FOS_INT_TYPE, FOS_INT_TYPE> &range_coeff) const;

    [[nodiscard]] bool has_coefficient() const;

    [[nodiscard]] const char *coefficient() const;

    [[nodiscard]] std::pair<int, int> coefficients() const;

    [[nodiscard]] fURI coefficient(const char *coefficient) const;

    [[nodiscard]] fURI coefficient(int low, int high) const;

    [[nodiscard]] const char *query() const;

    [[nodiscard]] bool has_query(const char *key = nullptr) const;

    [[nodiscard]] fURI query(const char *query) const;

    [[nodiscard]] fURI no_query() const;

    [[nodiscard]] fURI query(const List<Pair<string, string>> &key_values) const;

    [[nodiscard]] std::vector<Pair<string, string>> query_values() const;

    template<typename T = std::string>
    std::vector<T>
    query_values(const char *key, const Function<string, T> &transformer = [](const string &s) { return s; }) const {
      const Option<string> v = this->query_value(key);
      if(!v.has_value())
        return {};
      auto ss = std::stringstream(v.value());
      string token;
      std::vector<T> list;
      while(!(token = StringHelper::next_token(',', &ss)).empty()) {
        StringHelper::trim(token);
        // decode_query(token);
        list.push_back(transformer(token));
      }
      return list;
    }

    template<typename T = std::string>
    [[nodiscard]] std::optional<T>
    query_value(const char *key, const Function<string, T> &transformer = [](const string &s) { return s; }) const {
      if(this->query_.empty())
        return {};
      const char *index = strstr(this->query_.c_str(), key);
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
      // decode_query(value);
      return {transformer(value)};
    }

    ////////////////////////////////////////////////////////////////

    [[nodiscard]] fURI extend(const fURI &furi_path) const;

    [[nodiscard]] fURI extend(const char *extension) const;


    [[nodiscard]] fURI retract(int steps = 1) const;

    [[nodiscard]] fURI head() const;

    [[nodiscard]] fURI pretract(const fURI &prefix) const;

    [[nodiscard]] fURI retract(const fURI &prefix) const;

    [[nodiscard]] fURI pretract(int steps = 1) const;

    [[nodiscard]] fURI prepend(const fURI &furi_path) const;

    [[nodiscard]] fURI prepend(const char *extension) const;

    [[nodiscard]] bool has_wildcard(char wildcard = '#') const;

    [[nodiscard]] fURI retract_pattern() const;

    [[nodiscard]] fURI as_node() const;

    [[nodiscard]] fURI as_branch() const;

    [[nodiscard]] bool is_subfuri_of(const fURI &other) const;

    [[nodiscard]] bool is_relative() const;

    [[nodiscard]] fURI as_relative() const;

    [[nodiscard]] bool is_branch() const;

    [[nodiscard]] bool is_node() const;

    [[nodiscard]] bool is_scheme_path() const;

    [[nodiscard]] bool has_components() const;

    [[nodiscard]] fURI add_component(const fURI &component) const;

    [[nodiscard]] List<fURI> components() const;

    [[nodiscard]] fURI remove_subpath(const string &subpath, bool forward = true) const;

    [[nodiscard]] fURI append(const fURI &other) const;

    [[nodiscard]] virtual fURI resolve(const fURI &other) const;

    [[nodiscard]] virtual bool is_pattern() const;

    [[nodiscard]] virtual bool is_subpattern(const fURI &pattern) const;

    [[nodiscard]] virtual bool bimatches(const fURI &other) const;

    [[nodiscard]] virtual bool matches(const fURI &pattern) const;

    fURI &operator=(const fURI &other) noexcept;

    fURI &operator=(fURI &&other) noexcept;

    [[nodiscard]] bool headless() const;

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    virtual ~fURI();

    fURI(const fURI &other);

    fURI(const string &uri_string);

    fURI(const char *uri_chars);

    bool operator<(const fURI &other) const;

    bool operator!=(const fURI &other) const;

    bool operator==(const fURI &other) const;

    [[nodiscard]] bool equals(const fURI &other) const;

    [[nodiscard]] string toString() const;
  };

  class ID final : public fURI {
  public:
    ID(const ID &id) = default;

    ID(const fURI &id) : fURI(id.toString()) {}

    ID(const string &furi_string) : ID(furi_string.c_str()) {}

    ID(const char *furi_characters) : fURI(furi_characters) {
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
    Pattern(const Pattern &uri) = default;

    Pattern(const fURI &uri) : fURI(uri) {}

    Pattern(const string &uri_string) : fURI(uri_string) {};

    Pattern(const char *uri_chars) : fURI(uri_chars) {};
  };

  using fURI_p = ptr<fURI>;
  using ID_p = ptr<ID>;
  using Pattern_p = ptr<Pattern>;

  ///////////////////////////////////////////////////
  ///////////////// TYPED FURI OBJ /////////////////
  //////////////////////////////////////////////////

  class Typed {
  public:
    virtual ~Typed() = default;

    ID_p tid;

    explicit Typed(const ID_p &type) : tid(type) {}

    explicit Typed(const ID &id) : Typed(make_shared<ID>(id)) {}
  };

  ////////////////////////////////////////////////////
  ///////////////// VALUED FURI OBJ /////////////////
  ///////////////////////////////////////////////////

  class Valued {
  public:
    ID_p vid;

    virtual ~Valued() = default;

    explicit Valued(const ID_p &id) : vid(id) {
      if(id && id->empty())
        vid = nullptr;
    }

    explicit Valued(const ID &id) : Valued(make_shared<ID>(id)) {}
  };

  struct furi_less : std::less<fURI> {
    auto operator()(const fURI &a, const fURI &b) const { return std::less<string>()(a.toString(), b.toString()); }
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

  inline std::ostream &operator<<(std::ostream &os, const fURI &value) {
    os << value.toString();
    return os;
  }
} // namespace fhatos

#endif
