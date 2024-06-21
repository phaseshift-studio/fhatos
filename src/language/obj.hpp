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

/**
 *  @file   obj.hpp
 *  @brief  Foundation objects of mm-ADT
 *  @author Marko A. Rodriguez
 *  @date   2024-05-01
 ***********************************************/

#ifndef fhatos_obj_hpp
#define fhatos_obj_hpp


#ifndef FL_REAL_TYPE
#define FL_REAL_TYPE float
#include <any>
#endif
#ifndef FL_INT_TYPE
#define FL_INT_TYPE int
#endif

#include <fhatos.hpp>
#include <structure/furi.hpp>
#include <util/uuid.hpp>
#include <utility>
#include <variant>
#ifdef NATIVE
#include <assert.h>
#else
#include <esp_assert.h>
#endif

namespace fhatos {


  /// @brief The base types of mm-ADT
  enum class OType : uint8_t {
    /// The base type of all types is the obj
    OBJ,
    /// A "null" object type used to kill a processing monad
    NOOBJ,
    OBJS,
    /// A boolean mono-type
    BOOL,
    /// An integral number mono-type in Z
    INT,
    /// A real number mono-type in R
    REAL,
    /// A string mono-type denoting a sequence of characters where a "char" is equivalent to str[0]
    STR,
    /// A Uniform Resource Identifier mono-type
    URI,
    /// A list poly-type
    LST,
    /// A key/value pair record poly-type
    REC,
    /// An instruction type denoting an opcode, arguments, and a function
    INST,
    /// A "null" instruction type used to halt a processing monad
    NOINST,
    /// A sequence of instructions denoting a program
    BYTECODE
  };


  using BObj = Triple<OType, fbyte *, uint32_t>;
  class PtrSerializer;
  class Type;
  class Objs;

  static const Map<OType, const char *> OTYPE_STR = {{{OType::NOOBJ, "noobj"},
                                                      {OType::NOINST, "noinst"},
                                                      {OType::OBJ, "obj"},
                                                      {OType::OBJS, "objs"},
                                                      {OType::URI, "uri"},
                                                      {OType::BOOL, "bool"},
                                                      {OType::INT, "int"},
                                                      {OType::REAL, "real"},
                                                      {OType::STR, "str"},
                                                      {OType::LST, "lst"},
                                                      {OType::REC, "rec"},
                                                      {OType::INST, "inst"},
                                                      {OType::BYTECODE, "bcode"}}};
  static const Map<string, OType> STR_OTYPE = {{{"noobj", OType::NOOBJ},
                                                {"noinst", OType::NOINST},
                                                {"obj", OType::OBJ},
                                                {"objs", OType::OBJS},
                                                {"uri", OType::URI},
                                                {"bool", OType::BOOL},
                                                {"int", OType::INT},
                                                {"real", OType::REAL},
                                                {"str", OType::STR},
                                                {"lst", OType::LST},
                                                {"rec", OType::REC},
                                                {"inst", OType::INST},
                                                {"bcode", OType::BYTECODE}}};

  static const ptr<fURI> OBJ_FURI = ptr<fURI>(new fURI("/obj"));
  static const ptr<fURI> NOOBJ_FURI = ptr<fURI>(new fURI("/noobj"));
  static const ptr<fURI> TYPE_FURI = ptr<fURI>(new fURI("/type"));
  static const ptr<fURI> BOOL_FURI = ptr<fURI>(new fURI("/bool"));
  static const ptr<fURI> INT_FURI = ptr<fURI>(new fURI("/int"));
  static const ptr<fURI> REAL_FURI = ptr<fURI>(new fURI("/real"));
  static const ptr<fURI> URI_FURI = ptr<fURI>(new fURI("/uri"));
  static const ptr<fURI> STR_FURI = ptr<fURI>(new fURI("/str"));
  static const ptr<fURI> REC_FURI = ptr<fURI>(new fURI("/rec"));
  static const ptr<fURI> INST_FURI = ptr<fURI>(new fURI("/inst"));
  static const ptr<fURI> BCODE_FURI = ptr<fURI>(new fURI("/bcode"));
  static const ptr<fURI> OBJS_FURI = ptr<fURI>(new fURI("/objs"));

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Bytecode;
  class Obj {
  protected:
    std::variant<Any, ptr<Bytecode>> _var;

  public:
    ptr<fURI> _type;
    virtual ~Obj() = default;
    explicit Obj(const std::variant<Any, ptr<Bytecode>> &var, const ptr<fURI> &type) : _var(var), _type(type) {}

    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    template<class T, class = typename std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool) : _var(xbool), _type(BOOL_FURI) {}
    Obj(const FL_INT_TYPE xint) : _var(xint), _type(INT_FURI) {}
    Obj(const FL_REAL_TYPE xreal) : _var(xreal), _type(REAL_FURI) {}
    Obj(const fURI xuri) : _var(xuri), _type(URI_FURI) {}
    Obj(const char *xstr) : _var(xstr), _type(STR_FURI) {}
    Obj(const string xstr) : _var(xstr), _type(STR_FURI) {}
    Obj(const std::initializer_list<Pair<Obj const, Obj>> &xrec) : _var(Map<ptr<Obj>, ptr<Obj>>({})), _type(REC_FURI) {
      Map<ptr<Obj>, ptr<Obj>> map = std::any_cast<Map<ptr<Obj>, ptr<Obj>>>(std::get<0>(this->_var));
      for (const Pair<Obj const, Obj> &pair: xrec) {
        map.emplace(ptr<Obj>((Obj *) &(pair.first)), ptr<Obj>((Obj *) &(pair.second)));
      }
      this->_var = map;
    }
    //////////////////////////////////////////////////////////////
    virtual OType otype() const { return STR_OTYPE.at(this->_type->path(0, 1)); }
    virtual ptr<Type> type() const { throw fError("obj has no structural type"); }
    virtual string toString() const { throw fError("obj has no character representation"); }
    // virtual Any value() const { throw fError("obj has no structural value"); }

    template<typename OBJ = Obj>
    const ptr<OBJ>
    split(const Any &newValue,
          const std::variant<ptr<fURI>, const char *> &newType = std::variant<ptr<fURI>, const char *>(nullptr)) const {
      return ptr<OBJ>(new OBJ(newValue, std::variant<ptr<fURI>, const char *>(nullptr) == newType
                                            ? this->_type
                                            : (std::holds_alternative<const char *>(newType)
                                                   ? ptr<fURI>(new fURI(std::get<const char *>(newType)))
                                                   : std::get<ptr<fURI>>(newType))));
    }
    bool isBytecode() const { return std::holds_alternative<ptr<Bytecode>>(this->_var); }
    virtual ptr<Bytecode> bcode() const { return std::get<ptr<Bytecode>>(this->_var); }
    virtual ptr<Obj> apply(const ptr<Obj> &obj) { throw fError("obj has no applicable form"); }
    virtual bool operator==(const Obj &other) const {
      throw fError("obj has no equality relation");
     /* return this->_type->equals(*other._type) && (this->isBytecode() == other.isBytecode()) &&
             (this->isBytecode() ? (this->bcode() == other.bcode()) : (this->toString() == other.toString()));*/
    }
    virtual bool operator!=(const Obj &other) const { return !(*this == other); }
    virtual bool isNoObj() const { return this->otype() == OType::NOOBJ; }

    template<typename SERIALIZER = PtrSerializer>
    const ptr<BObj> serialize() const {
      return SERIALIZER::singleton()->serialize(this);
    }

  protected:
    template<typename A = Obj>
    ptr<A> as(const ptr<fURI> &type) const {
      if (type->equals(*this->_type))
        return share(*(A *) this);
      A *a = new A(*(A *) this);
      a->_type = type;
      return ptr<A>(a);
    }

    /*template<typename SERIALIZER = PtrSerializer, typename _OBJ>
    static const ptr<_OBJ> deserialize(const BObj *bobj) {
      return SERIALIZER::singleton()->deserialize(this);
    }*/
  };

  ///////////////////////////////////////////////////
  ////////////////////// TYPE //////////////////////
  //////////////////////////////////////////////////
  /// An mm-ADT type encoded in a URI
  class Type : public Obj {
  public:
    explicit Type(const ptr<fURI> &furi) : Obj(furi, TYPE_FURI) {}
    ptr<fURI> v_furi() const { return std::any_cast<ptr<fURI>>(std::get<0>(this->_var)); }
    OType instanceOType() { return STR_OTYPE.at(this->v_furi()->path(0, 1)); }
    string variable() const { return this->v_furi()->user().value_or(""); }
    string name() const { return this->v_furi()->lastSegment(); }
    string location() const { return this->v_furi()->host(); }
    bool mutating() const { return this->variable()[0] == '~'; }
    string objString(const string &objString) const {
      if (this->v_furi()->length() > 2)
        return (this->variable().empty() ? "" : (this->variable() + "@")) +
               ("!b" + this->name() + "!g[!!" + objString + "!g]!!");
      else
        return (this->variable().empty() ? "" : (this->variable() + "@")) + objString;
    }
    string toString() const override { return this->v_furi()->toString(); }
    static ptr<fURI> obj_t(const OType &otype, const char *utype) {
      return ptr<fURI>(new fURI(string("/") + OTYPE_STR.at(otype) + (strlen(utype) == 0 ? "" : ("/" + string(utype)))));
    }
  };

  ///////////////////////////////////////////////////
  ////////////////////// NOOBJ //////////////////////
  ///////////////////////////////////////////////////
  class NoObj : public Obj {
  public:
    template<typename OBJ = Obj>
    static ptr<OBJ> self_ptr() {
      return ptr<OBJ>(new NoObj());
    }
    ptr<Obj> apply(const ptr<Obj> &obj) override { return NoObj::self_ptr(); }
    bool isNoObj() const override { return true; }
    string toString() const override { return "!rØ!!"; }
    bool operator==(const Obj &other) const override { return other.isNoObj(); }

  private:
    NoObj() : Obj(std::variant<Any, ptr<Bytecode>>(false), NOOBJ_FURI) {}
  };
} // namespace fhatos

#endif
