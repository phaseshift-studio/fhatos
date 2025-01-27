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

#include  "../fhatos.hpp"
#include "../lang/obj.hpp"

namespace fhatos {
  class PrintHelper {
  public:
    PrintHelper() = delete;


    static string pretty_print_obj(const Obj_p &obj, const FOS_INT_TYPE max_depth, bool char_indent = false) {
      auto sb = std::stringbuf();
      pretty_print_obj(obj, 0, max_depth, false, &sb, char_indent);
      const string result = sb.str();
      return result.substr(0, result.length() - 1);
    }

    static void pretty_print_obj(const Obj_p &obj, const FOS_INT_TYPE depth,
                                 const FOS_INT_TYPE max_depth, const bool parent_rec, std::streambuf *sb,
                                 bool char_indent = true,
                                 ObjPrinter *obj_printer = nullptr) {
      auto ss = std::ostream(sb);
      if(obj->is_objs()) {
        for(Obj_p &o: *obj->objs_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          pretty_print_obj(o, depth, max_depth, parent_rec, sb, char_indent);
        }
      } else if(obj->is_lst() && max_depth > depth) {
        if(!parent_rec) {
          ss << "!g" << StringHelper::repeat(depth, char_indent ? "=" : " ") << (char_indent ? ">" : " ") << "!b"
              << (obj->is_base_type() ? "" : obj->tid_->name().c_str());
        }
        ss << "!m[!!\n";
        for(const auto &e: *obj->lst_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          if(!e->is_poly()) {
            ss << StringHelper::format("%s%s!!\n",
                                       (string("!g") + StringHelper::repeat(depth, char_indent ? "=" : " ") + (
                                          char_indent ? "==>!!" : "   !!")).c_str(),
                                       e->toString().c_str());
          } else {
            //to_out->append("!m,!!");
            pretty_print_obj(e, depth + 1, max_depth, false, sb, char_indent);
          }
        }
        ss << string("!g") + StringHelper::repeat(depth, char_indent ? "=" : " ") << (char_indent ? ">" : " ") << "!b";
        ss << (obj->is_base_type() ? "" : StringHelper::repeat(obj->tid_->name().length(), " ").c_str()) << "!m]!!\n";
      } else if(obj->is_rec() && max_depth > depth) {
        if(!parent_rec) {
          ss << "!g" << StringHelper::repeat(depth, char_indent ? "=" : " ") << (char_indent ? ">" : " ") << "!b" <<
              (obj->is_base_type() ? "" : obj->tid_->name().c_str());
        }
        ss << "!m[!!\n";
        for(const auto &[key, value]: *obj->rec_value()) {
          Process::current_process()->feed_watchdog_via_counter();
          if(!value->is_poly()) {
            ss << StringHelper::format(
              "%s!c%s!m=>!!%s!!\n",
              (string("!g") + StringHelper::repeat(depth, char_indent ? "=" : " ") + (char_indent ? "==>!!" : "   !!")).
              c_str(),
              key->toString().c_str(),
              value->toString().c_str());
          } else {
            ss << StringHelper::format(
              "%s!c%s!m=>!!", (string("!g") +
                               StringHelper::repeat(depth, char_indent ? "=" : " ") +
                               (char_indent ? "==>!!" : "   !!")).c_str(),
              key->toString().c_str());
            pretty_print_obj(value, depth + 1, max_depth, true, sb, char_indent);
          }
        }
        ss << "!g" << StringHelper::repeat(depth, char_indent ? "=" : " ")
            << (char_indent ? ">" : " ") << "!b" << "!m]";
        if(obj->vid_) {
          ss << "!m@!b";
          ss << obj->vid_->toString();
        }
        ss << "!!\n";
      } else {
        if(!parent_rec)
          ss << (char_indent ? "!g==>!!" : "   !!");
        ss << print_obj(obj);
        ss << '\n';
      }
    }

    static string print_obj(const Obj_p &obj, const ObjPrinter *obj_printer = nullptr) {
      std::stringbuf sb;
      print_obj(obj, &sb, obj_printer);
      return sb.str();
    }

    static void print_obj(const Obj_p &obj, std::streambuf *sb, const ObjPrinter *obj_printer = nullptr) {
      auto ss = std::ostream(sb);
      string close;
      if(!obj_printer)
        obj_printer = GLOBAL_PRINTERS.at(obj->otype_);
      if(obj->is_noobj())
        ss << "!r" STR(FOS_NOOBJ_TOKEN) "!!";
      else {
        if(!obj->is_noobj() &&
           (obj->is_type() ||
            obj->is_inst() ||
            (obj_printer->show_type && !obj->is_base_type()))) {
          string typing;
          if(obj->is_type())
            ss << "!m[!!";
          ss << (obj->is_base_type() && !obj->is_inst() && !obj->is_type() && !obj->is_uri()
                   ? ""
                   : string("!b")
                   .append(obj_printer->strict ? obj->tid_->toString() : obj->tid_->name()).append("!!"));
          // TODO: remove base_type check
          if(obj_printer->show_domain_range) {
            const string dom_str = obj->has_domain(1, 1) && !obj_printer->strict
                                     ? ""
                                     : obj->has_domain(0, 1)
                                         ? "?"
                                         : obj->has_domain(1,INT_MAX)
                                             ? "+"
                                             : obj->is_initial()
                                                 ? "."
                                                 : obj->is_gather()
                                                     ? "*"
                                                     : to_string(obj->domain_coefficient().first)
                                                     .append(",")
                                                     .append(to_string(obj->domain_coefficient().second));
            const string rng_str = obj->has_range(1, 1) && !obj_printer->strict
                                     ? ""
                                     : obj->has_range(0, 1)
                                         ? "?"
                                         : obj->has_range(1,INT_MAX)
                                             ? "+"
                                             : obj->is_terminal()
                                                 ? "."
                                                 : obj->is_scatter()
                                                     ? "*"
                                                     : to_string(obj->range_coefficient().first)
                                                     .append(",")
                                                     .append(to_string(obj->range_coefficient().second));

            if(!dom_str.empty() || !rng_str.empty() ||
               !obj->range()->equals(*OBJ_FURI) || !obj->domain()->equals(*OBJ_FURI)) {
              ss << "!m?!!"
                  << "!c" << (obj_printer->strict ? obj->range()->toString() : obj->range()->name())
                  << (rng_str.empty() ? "" : string("!m{!c").append(rng_str).append("!m}!!"))
                  << "!m<=!!"
                  << "!c"
                  << (obj_printer->strict ? obj->domain()->toString() : obj->domain()->name())
                  << (dom_str.empty() ? "" : string("!m{!c").append(dom_str).append("!m}!!"));
            }
          }
          if(obj->is_type())
            ss << "!m]!!";
        }

        switch(obj->otype_) {
          case OType::BOOL:
            ss << (obj->bool_value() ? "!ytrue!!" : "!yfalse!!");
            break;
          case OType::INT:
            ss << obj->int_value();
            break;
          case OType::REAL:
            ss << obj->real_value();
            break;
          case OType::URI:
            ss << "!_" + (obj_printer->strict
                            ? "<" + obj->uri_value().toString() + ">"
                            : obj->uri_value().toString()) + "!!";
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
              ss << "!m[!g=>!m]!!";
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
                ss << "!g=>!!";
                print_obj(v, sb);
              }
              ss << "!m]!!";
            }
            break;
          }
          case OType::INST: {
            bool first = true;
            ss << "!g(!!";
            for(const auto &[k,v]: *obj->inst_args()->rec_value()) {
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
            ss << "!g)!!";
            if(obj->inst_f()) {
              ss << "!g[!!" << (std::holds_alternative<Obj_p>(*obj->inst_f())
                                  ? std::get<Obj_p>(*obj->inst_f())->toString()
                                  : "!ycpp!!") << "!g]!!";
            }
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
            throw fError("unknown obj type in toString(): %s", OTypes.to_chars(obj->otype_).c_str());
        }
      }

      if(obj_printer->show_id && obj->vid_)
        ss << "!m@!b" << obj->vid_->toString() << "!!";

      //obj_string = obj_printer->ansi ? obj_string : Ansi<>::strip(obj_string);
    }
  };
}

#endif
