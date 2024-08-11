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
#ifndef fhatos_rewrite_hpp
#define fhatos_rewrite_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <util/string_printer.hpp>

namespace fhatos {
  using PriorPost = Pair<List<ID>, List<ID>>;
  using Rewrite = Trip<ID, Function<BCode_p, BCode_p>, PriorPost>;
  struct Rewriter {
    List<Rewrite> _rewrites;
    explicit Rewriter(const List<Rewrite> &rewrites) : _rewrites(rewrites) {}
    BCode_p apply(const BCode_p &bcode) const {
      BCode_p running = bcode;
      for (const Rewrite &rw: this->_rewrites) {
        LOG(DEBUG, "Applying rewrite %s\n", std::get<0>(rw).toString().c_str());
        running = std::get<1>(rw)(running);
      }
      return running;
    }
    static void LOG_REWRITE(const ID &rewriteID, const BCode_p &original, const BCode_p &rewrite) {
      LOG(DEBUG, "!g[!b%s!g]!! !yrewrote!! %s !r=to=>!! %s\n", rewriteID.toString().c_str(),
          original->toString().c_str(), rewrite->toString().c_str());
    }
    static Rewrite explain() {
      return Rewrite({ID("/lang/rewrite/explain"),
                      [](const BCode_p &bcode) {
                        if (bcode->bcode_value()->back()->id()->equals(ID(FOS_TYPE_PREFIX "inst/explain"))) {
                          auto ex = string();
                          auto p = Ansi<StringPrinter>(StringPrinter(&ex));
                          // bcode->bcode_value()->back()->inst_seed()->add_obj(bcode);
                          p.printf("\n!r!_%s\t  %s\t\t\t%s!!\n", "op", "inst", "domain/range");
                          for (const Inst_p &inst: *bcode->bcode_value()) {
                            p.printf("!b%s!!\t  %s\t\t\t%s\n", inst->inst_op().c_str(), inst->toString().c_str(),
                                     ITypeSignatures.toChars(inst->itype()));
                          }
                          // bcode->bcode_value()->back()->inst_seed()->add_obj(Obj::to_str(ex));
                          BCode_p rewrite = Obj::to_bcode({Insts::start(Obj::to_objs({Obj::to_str(ex)}))});
                          LOG_REWRITE(ID("/lang/rewrite/by"), bcode, rewrite);
                          return rewrite;
                        }
                        return bcode;
                      },
                      {{}, {}}});
    }
    static Rewrite by() {
      return Rewrite({ID("/lang/rewrite/by"),
                      [](const BCode_p &bcode) {
                        Inst_p prev = Obj::to_noobj();
                        bool found = false;
                        List<Inst_p> newInsts;
                        for (const Inst_p &inst: *bcode->bcode_value()) {
                          if (inst->id()->equals(ID(FOS_TYPE_PREFIX "inst/by")) && !prev->isNoObj()) {
                            found = true;
                            // rewrite args
                            bool done = false;
                            List<Obj_p> newArgs;
                            for (const Obj_p &arg: prev->inst_args()) {
                              if (!done && arg->isNoObj()) {
                                newArgs.push_back(inst->inst_arg(0));
                                done = true;
                              } else {
                                newArgs.push_back(arg);
                              }
                            }
                            if (!done)
                              throw fError("Previous inst could not be by()-modulated: %s !r<=/=!! %s\n",
                                           prev->toString().c_str(), inst->toString().c_str());
                            // rewrite inst
                            newInsts.pop_back();
                            newInsts.push_back(Insts::to_inst(*prev->id(), newArgs));
                          } else {
                            newInsts.push_back(inst);
                          }
                          prev = newInsts.back();
                        }
                        if (found) {
                          const BCode_p rewrite = Obj::to_bcode(newInsts);
                          LOG_REWRITE(ID("/lang/rewrite/by"), bcode, rewrite);
                          return rewrite;
                        }
                        return bcode;
                      },
                      {{}, {}}});
    }
  };
} // namespace fhatos
#endif
