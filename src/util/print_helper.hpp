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
#ifndef fhatos_print_helper_hpp
#define fhatos_print_helper_hpp

#include <fmt/core.h>
#include "../fhatos.hpp"
#include "../lang/obj.hpp"
#include "../model/fos/sys/router/router.hpp"

namespace fhatos {
  class PrintHelper {
  public:
    PrintHelper() = delete;
    static void import() {
      PRINT_OBJ = [](const Obj_p &obj, const ObjPrinter *obj_printer) -> string {
        return std::move(PrintHelper::print_obj(obj, obj_printer));
      };
    }
    static string print_fail_reason(std::stack<std::string> fail_reason) {
      string fail;
      int count = 1;
      while(!fail_reason.empty()) {
        fail.append("\n\t\t")
            .append(StringHelper::repeat(count, " "))
            .append("!m\\")
            .append(StringHelper::repeat(count, "_"))
            .append("!!")
            .append(fail_reason.top());
        fail_reason.pop();
        count++;
      }
      return fail;
    }

    static string pretty_print_obj(const Obj_p &obj, const FOS_INT_TYPE max_depth, bool char_indent = false) {
      auto sb = std::stringbuf();
      pretty_print_obj(obj, 0, max_depth, false, &sb, char_indent);
      const string result = sb.str();
      return result.substr(0, result.length() - 1);
    }

    static void pretty_print_obj(const Obj_p &obj, const FOS_INT_TYPE depth, const FOS_INT_TYPE max_depth,
                                 const bool parent_rec, std::streambuf *sb, const bool char_indent = true,
                                 ObjPrinter *obj_printer = nullptr) {
      auto ss = std::ostream(sb);
      if(obj->is_objs()) {
        for(Obj_p &o: *obj->objs_value()) {
          pretty_print_obj(o, depth, max_depth, parent_rec, sb, char_indent);
        }
      } else if(obj->is_lst() && max_depth > depth) {
        if(!parent_rec) {
          const string indentation = StringHelper::repeat(depth, char_indent ? "=" : " ");
          // if(char_indent && 0 == depth)
          //   indentation += '=';
          ss << "!m" << indentation << (char_indent ? ">" : " ") << "!b"
             << (obj->is_base_type() ? "" : obj->tid->name().c_str());
        }
        ss << "!m[!!\n";
        for(int i = 0; i < obj->lst_value()->size(); i++) {
          if(const auto &e = obj->lst_value()->at(i); !e->is_poly()) {
            string indentation = StringHelper::repeat(depth, char_indent ? "=" : " ");
            // if(char_indent && 0 == depth)
            //   indentation += '=';
            ss << StringHelper::format(
                /*i == obj->lst_value()->size() - 1*/ true ? "%s%s!!\n" : "%s%s!m,!!\n",
                (string("!m") + indentation + (char_indent ? "==>!!" : "   !!")).c_str(), e->toString().c_str());
          } else {
            // to_out->append("!m,!!");
            pretty_print_obj(e, depth + 1, max_depth, false, sb, char_indent);
          }
        }
        const string indentation = StringHelper::repeat(depth, char_indent ? "=" : " ");
        // if(char_indent && 0 == depth)
        //  indentation += '=';
        ss << "!m" << indentation << (char_indent ? ">" : " ") << "!b";
        ss << (obj->is_base_type() ? "" : StringHelper::repeat(obj->tid->name().length(), " ").c_str()) << "!m]!!\n";
      } else if(obj->is_rec() && max_depth > depth) {
        if(!parent_rec) {
          const string indentation = StringHelper::repeat(depth, char_indent ? "=" : " ");
          //  if(char_indent && 0 == depth)
          //    indentation += '=';
          ss << "!m" << indentation << (char_indent ? ">" : " ") << "!b"
             << (obj->is_base_type() ? "" : obj->tid->name().c_str());
        }
        ss << "!m[!!\n";
        for(const auto &[key, value]: *obj->rec_value()) { // TODO: comma after every key/value pair save the last
          const string indentation = StringHelper::repeat(depth, char_indent ? "=" : " ");
          string format_style = "{}"; // {}!c{}!y=>!!{}!!
          format_style.append("!c"); // depth == 0 ? "!c" : "!m");
          format_style.append("{}!y=>!!");
          format_style.append(value->is_poly() ? "" : "{}!!\n");
          ss << fmt::format(format_style, string("!m") + indentation + (char_indent ? "==>!!" : "   !!"),
                            key->toString(), value->is_poly() ? "" : value->toString());
          if(value->is_poly())
            pretty_print_obj(value, depth + 1, max_depth, true, sb, char_indent);
        }
        const string indentation = StringHelper::repeat(depth, char_indent ? "=" : " ");
        //  if(char_indent && 0 == depth)
        //   indentation += '=';
        ss << "!m" << indentation << (char_indent ? ">" : " ") << "!b" << "!m]";
        if(obj->vid) {
          ss << "!m@!b";
          ss << obj->vid->toString();
        }
        ss << "!!\n";
      } else {
        if(!parent_rec)
          ss << (char_indent ? "!m==>!!" : "   !!");
        ss << print_obj(obj);
        ss << '\n';
      }
    }

    static string print_obj(const Obj_p &obj, const ObjPrinter *obj_printer = nullptr) {
      FEED_WATCHDOG();
      std::stringbuf sb;
      const ObjPrinter *temp = obj_printer ? obj_printer : GLOBAL_PRINTERS.at(obj->otype);
      print_obj(obj, &sb, temp);
      return std::move(temp->ansi ? sb.str() : Ansi<>::strip(sb.str()));
    }

    static void print_obj(const Obj_p &obj, std::streambuf *sb, const ObjPrinter *obj_printer = nullptr) {
      auto ss = std::ostream(sb);
      if(!obj_printer)
        obj_printer = GLOBAL_PRINTERS.at(obj->otype);
      if(obj->is_noobj())
        ss << "!r" STR(FOS_NOOBJ_TOKEN) "!!";
      else {
        bool type_printed = false;
        string typing;
        if(obj->is_type() || obj->is_inst() || !obj->domain_range().is_single() ||
           (obj_printer->show_type && !obj->is_base_type())) {
          if(obj->is_type()) {
            typing += "!m[!!";
            type_printed = true;
          }
          if(!obj->is_base_type() || obj->is_inst() || obj->is_type() || obj->is_uri()) {
            type_printed = true;
            if(obj_printer->strict)
              typing += string("!b").append(obj->tid->toString()).append("!!");
            else {
              bool match = false;
              for(const auto &uri: Router::singleton()->auto_prefixes_) {
                if(obj->tid->no_query().starts_with(uri->uri_value().retract_pattern())) {
                  match = true;
                  typing += string("!b")
                                .append(obj->tid->no_query().pretract(uri->uri_value().retract_pattern()).toString())
                                .append("!!");
                  break;
                }
              }
              if(!match) {
                typing += string("!b").append(obj->tid->no_query().toString()).append("!!");
              }
            }
          }
          if(obj_printer->show_domain_range && !obj->is_inst_stub()) {
            const string dom_str = obj->has_domain(1, 1) && !obj_printer->strict ? ""
                                   : obj->has_domain(0, 1)                       ? "?"
                                   : obj->has_domain(1, INT_MAX)                 ? "+"
                                   : obj->is_initial()                           ? "."
                                   : obj->is_gather()                            ? "*"
                                                      : to_string(obj->domain_coefficient().first)
                                                            .append(",")
                                                            .append(to_string(obj->domain_coefficient().second));
            const string rng_str = obj->has_range(1, 1) && !obj_printer->strict ? ""
                                   : obj->has_range(0, 1)                       ? "?"
                                   : obj->has_range(1, INT_MAX)                 ? "+"
                                   : obj->is_terminal()                         ? "."
                                   : obj->is_scatter()                          ? "*"
                                                       : to_string(obj->range_coefficient().first)
                                                             .append(",")
                                                             .append(to_string(obj->range_coefficient().second));

           // if(!dom_str.empty() || !rng_str.empty()) {
              typing = typing.append("!m?!!")
                           .append("!c")
                           .append(obj_printer->strict ? obj->range()->toString() : obj->range()->name())
                           .append(rng_str.empty() ? "" : string("!m{!c").append(rng_str).append("!m}!!"))
                           .append("!m<=!!")
                           .append("!c")
                           .append(obj_printer->strict ? obj->domain()->toString() : obj->domain()->name())
                           .append(dom_str.empty() ? "" : string("!m{!c").append(dom_str).append("!m}!!"));
         //   }
          }
          // TODO: remove base_type check
          /* if(!obj->domain_range().is_single() || obj_printer->show_domain_range) {
             const string dom_str = obj->has_domain(1, 1) && !obj_printer->strict ? ""
                                    : obj->has_domain(0, 1)                       ? "?"
                                    : obj->has_domain(1, INT_MAX)                 ? "+"
                                    : obj->is_initial()                           ? "."
                                    : obj->is_gather()                            ? "*"
                                                       : to_string(obj->domain_coefficient().first)
                                                             .append(",")
                                                             .append(to_string(obj->domain_coefficient().second));
             const string rng_str = obj->has_range(1, 1) && !obj_printer->strict ? ""
                                    : obj->is_filter()                           ? "?"
                                    : obj->has_range(1, INT_MAX)                 ? "+"
                                    : obj->is_terminal()                         ? "."
                                    : obj->is_scatter()                          ? "*"
                                                        : to_string(obj->range_coefficient().first)
                                                              .append(",")
                                                              .append(to_string(obj->range_coefficient().second));

             if(!dom_str.empty() || !rng_str.empty() || !obj->range()->equals(obj->tid->no_query()) ||
                !obj->domain()->equals(*OBJ_FURI)) {
               ss << "!m?!!"
                  << "!c" << (obj_printer->strict ? obj->range()->toString() : obj->range()->name())
                  << (rng_str.empty() ? "" : string("!m{!c").append(rng_str).append("!m}!!")) << "!m<=!!"
                  << "!c" << (obj_printer->strict ? obj->domain()->toString() : obj->domain()->name())
                  << (dom_str.empty() ? "" : string("!m{!c").append(dom_str).append("!m}!!"));
             }
           }*/
        }
        if(obj->is_type())
          typing += "!m]!!";
        if(type_printed)
          ss << typing;
        if(type_printed && !obj->is_inst())
          ss << "!g[!!";
        switch(obj->otype) {
          case OType::BOOL:
            ss << (obj->bool_value() ? "!ytrue!!" : "!yfalse!!");
            break;
          case OType::INT:
            ss << obj->int_value();
            break;
          case OType::REAL:
            ss << fmt::format("{:f}", obj->real_value());
            break;
          case OType::URI:
            ss << "!_" +
                      (obj_printer->strict || obj->uri_value().empty() ? "<" + obj->uri_value().toString() + ">"
                                                                       : obj->uri_value().toString()) +
                      "!!";
            break;
          case OType::STR:
            ss << "!m'!!!~" + obj->str_value() + "!m'!!";
            break;
          case OType::LST: {
            if(obj->lst_value()->empty())
              ss << "!m[]!!";
            else {
              ss << "!m[!!";
              bool first = true;
              for(const auto &obj2: *obj->lst_value()) {
                if(first) {
                  first = false;
                } else {
                  ss << "!m,!!";
                }
                print_obj(obj2, sb, obj_printer->next());
              }
              ss << "!m]!!";
            }
            break;
          }
          case OType::REC: {
            if(obj->rec_value()->empty())
              ss << "!m[!y=>!m]!!";
            else {
              ss << "!m[!!";
              bool first = true;
              for(const auto &[k, v]: *obj->rec_value()) {
                if(first) {
                  first = false;
                } else {
                  ss << "!m,";
                }
                ss << "!c";
                print_obj(k, sb, obj_printer->next()); // {ansi=false});
                ss << "!y=>!!";
                print_obj(v, sb, obj_printer->next());
              }
              ss << "!m]!!";
            }
            break;
          }
          case OType::INST: {
            bool first = true;
            ss << "!g(!!";
            for(const auto &[k, v]: *obj->inst_args()->rec_value()) {
              if(first) {
                first = false;
              } else {
                ss << "!m,!!";
              }
              if(!k->is_indexed_arg()) {
                ss << "!c";
                print_obj(k, sb, obj_printer->next()); // {ansi=false});
                ss << "!g=>!!";
              }
              print_obj(v, sb);
            }
            ss << "!g)";
            if(obj->has_inst_f()) {
              if(std::holds_alternative<Obj_p>(obj->inst_f())) {
                const string inst_f_str = std::get<Obj_p>(obj->inst_f())->toString();
                if(Ansi<>::strip(inst_f_str).length() > 20) // TODO: this is arbitrary
                  ss << "\n\t";
                ss << "!m[!!" << inst_f_str;
              } else {
                ss << "!m[!ycpp!!";
              }
              ss << "!m]!!";
            } else
              ss << "!m[!rnoobj!m]!!";
            break;
          }
          case OType::BCODE: {
            if(obj->bcode_value()->empty())
              ss << "_";
            else {
              // objString += "!b" + this->bcode_range()->name() + "!g<=!b" + this->bcode_domain()->name() + "!g[!!";
              bool first = true;
              for(const auto &inst: *obj->bcode_value()) {
                if(first) {
                  first = false;
                } else {
                  ss << "!g.!!";
                }
                print_obj(inst, sb);
              }
              // objString += "!g]!!";
            }
            break;
          }
          case OType::OBJS: {
            ss << "!m{!!";
            bool first = true;
            for(const auto &obj2: *obj->objs_value()) {
              if(first) {
                first = false;
              } else {
                ss << "!m,!!";
              }
              print_obj(obj2, sb, obj_printer->next());
            };
            ss << "!m}!!";
            break;
          }
          case OType::ERROR: {
            ss << "!r<<!!";
            print_obj(obj->error_value().first, sb, obj_printer->next());
            ss << "!r@!!";
            print_obj(obj->error_value().second, sb, obj_printer->next());
            ss << "!r>>!!";
            break;
          }
          case OType::NOOBJ: {
            ss << "!r" STR(FOS_NOOBJ_TOKEN) "!!"; // TODO: repeated above (is_noobj()) to skip the typing checks
            break;
          }
          case OType::TYPE: {
            ss << "!g[!!";
            print_obj(obj->type_value(), sb);
            ss << "!g]!!";
            break;
          }
          default:
            throw fError("unknown obj type in toString(): %s", OTypes.to_chars(obj->otype).c_str());
        }
        if(type_printed && !obj->is_inst())
          ss << "!g]!!";
      }

      if(obj_printer->show_id && obj->vid)
        ss << "!m@!b" << obj->vid->toString() << "!!";

      // obj_string = obj_printer->ansi ? obj_string : Ansi<>::strip(obj_string);
    }
  };
} // namespace fhatos

#endif

/*
 *if(!obj_printer)
        obj_printer = GLOBAL_PRINTERS.at(this->otype);
      string obj_string;
      if(this->is_noobj())
        obj_string = "!r" STR(FOS_NOOBJ_TOKEN) "!!";
      else {
        switch(this->otype) {
          case OType::BOOL:
            obj_string = this->bool_value() ? "!ytrue!!" : "!yfalse!!";
            break;
          case OType::INT:
            obj_string = std::to_string(this->int_value());
            break;
          case OType::REAL:
            obj_string = fmt::format("{:.5f}", this->real_value());
            break;
          case OType::URI:
            obj_string = "!_" +
                         (obj_printer->strict || this->uri_value().empty() ? "<" + this->uri_value().toString() + ">"
                                                                           : this->uri_value().toString()) +
                         "!!";
            break;
          case OType::STR:
            obj_string = "!m'!!!~" + this->str_value() + "!m'!!";
            break;
          case OType::LST: {
            if(this->lst_value()->empty())
              obj_string = "!m[]!!";
            else {
              obj_string = "!m[!!";
              bool first = true;
              for(const auto &obj: *this->lst_value()) {
                if(first) {
                  first = false;
                } else {
                  obj_string += "!m,!!";
                }
                obj_string += obj->toString(obj_printer->next());
              }
              obj_string += "!m]!!";
            }
            break;
          }
          case OType::REC: {
            if(this->rec_value()->empty())
              obj_string = "!m[!y=>!m]!!";
            else {
              obj_string = "!m[!!";
              bool first = true;
              for(const auto &[k, v]: *this->rec_value()) {
                if(first) {
                  first = false;
                } else {
                  obj_string += "!m,";
                }
                obj_string += "!c";
                obj_string += k->toString(obj_printer->next()); // {ansi=false});
                obj_string += "!y=>!!";
                obj_string += v->toString(obj_printer->next());
              }
              obj_string += "!m]!!";
            }
            break;
          }
          case OType::INST: {
            bool first = true;
            for(const auto &[k, v]: *this->inst_args()->rec_value()) {
              if(first) {
                first = false;
              } else {
                obj_string += "!m,!!";
              }
              if(!k->is_indexed_arg()) {
                obj_string += "!c";
                obj_string += k->toString(obj_printer->next());
                obj_string += "!g=>!!";
              }
              obj_string += v->toString();
            }
            break;
          }
          case OType::BCODE: {
            if(this->bcode_value()->empty())
              obj_string = "_";
            else {
              // objString += "!b" + this->bcode_range()->name() + "!g<=!b" + this->bcode_domain()->name() + "!g[!!";
              bool first = true;
              for(const auto &inst: *this->bcode_value()) {
                if(first) {
                  first = false;
                } else {
                  obj_string += "!g.!!";
                }
                obj_string += inst->toString(GLOBAL_PRINTERS.at(OType::OBJ));
              }
              // objString += "!g]!!";
            }
            break;
          }
          case OType::OBJS: {
            obj_string += "!m{!!";
            bool first = true;
            for(const auto &obj: *this->objs_value()) {
              if(first) {
                first = false;
              } else {
                obj_string += "!m,!!";
              }
              obj_string += obj->toString(obj_printer->next());
            };
            obj_string += "!m}!!";
            break;
          }
          case OType::ERROR: {
            obj_string = "!r<<!!";
            obj_string += this->error_value().first->toString(obj_printer->next());
            obj_string += "!r@!!";
            obj_string += this->error_value().second->toString(obj_printer->next());
            obj_string += "!r>>!!";
            break;
          }
          case OType::NOOBJ: {
            obj_string = "!r" STR(FOS_NOOBJ_TOKEN) "!!"; // TODO: repeated above (is_noobj()) to skip the typing checks
            break;
          }
          case OType::TYPE: {
            obj_string = this->type_value()->toString();
            break;
          }
          case OType::OBJ: {
            obj_string = this->vid_or_tid()->toString();
            break;
          }
          default:
            throw fError("unknown obj type in toString(): %s", OTypes.to_chars(this->otype).c_str());
        }
      }
      if(!this->is_noobj() &&
         (this->is_type() || this->is_nst() || (obj_printer->show_type && !this->is_base_type()))) {
        string typing;
        if(this->is_type())
          typing += "!m[!!";
        typing +=
            this->is_base_type() && !this->is_inst() && !this->is_type() && !this->is_uri()
                ? ""
                : string("!b").append(obj_printer->strict ? this->tid->toString() : this->tid->name()).append("!!");
        // TODO: remove base_type check
        if(obj_printer->show_domain_range) {
          const string dom_str = this->has_domain(1, 1) && !obj_printer->strict ? ""
                                 : this->has_domain(0, 1)                       ? "?"
                                 : this->has_domain(1, INT_MAX)                 ? "+"
                                 : this->is_initial()                           ? "."
                                 : this->is_gather()                            ? "*"
                                                     : to_string(this->domain_coefficient().first)
                                                           .append(",")
                                                           .append(to_string(this->domain_coefficient().second));
          const string rng_str = this->has_range(1, 1) && !obj_printer->strict ? ""
                                 : this->has_range(0, 1)                       ? "?"
                                 : this->has_range(1, INT_MAX)                 ? "+"
                                 : this->is_terminal()                         ? "."
                                 : this->is_scatter()                          ? "*"
                                                      : to_string(this->range_coefficient().first)
                                                            .append(",")
                                                            .append(to_string(this->range_coefficient().second));

          if(!dom_str.empty() || !rng_str.empty()) {
            typing = typing.append("!m?!!")
                         .append("!c")
                         .append(obj_printer->strict ? this->range()->toString() : this->range()->name())
                         .append(rng_str.empty() ? "" : string("!m{!c").append(rng_str).append("!m}!!"))
                         .append("!m<=!!")
                         .append("!c")
                         .append(obj_printer->strict ? this->domain()->toString() : this->domain()->name())
                         .append(dom_str.empty() ? "" : string("!m{!c").append(dom_str).append("!m}!!"));
          }
        }
        if(this->is_type())
          typing += "!m]!!";
        obj_string = this->is_base_type() && !this->is_inst() && !this->is_type()
                         ? typing.append(obj_string)
                         : typing.append(this->is_inst() ? "!g(!!" : "!g[!!")
                               .append(obj_string)
                               .append(this->is_inst() ? "!g)!!" : "!g]!!");

        if(this->is_inst() && this->has_inst_f()) {
          obj_string =
              obj_string.append("!g[!!")
                  .append(std::holds_alternative<Obj_p>(this->inst_f()) ? std::get<Obj_p>(this->inst_f())->toString()
                                                                        : "!ycpp!!")
                  .append("!g]!!");
        }
      }
      if(obj_printer->show_id && this->vid)
        obj_string.append("!m@!b").append(this->vid->toString()).append("!!");

      obj_string = obj_printer->ansi ? obj_string : Ansi<>::strip(obj_string);
      return std::move(obj_string);
 */
