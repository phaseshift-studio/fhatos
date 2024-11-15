#ifndef fhatos_monad_hpp
#define fhatos_monad_hpp

namespace fhatos {
  class Monad {
    //: public Rec {
  protected:
    const Obj_p obj_;
    const Inst_p inst_;
    const long bulk_ = 1;
    uint16_t loops_ = 0;

  public:
    Monad() = delete;

    explicit Monad(const Obj_p &obj, const Inst_p &inst, const ID &processor_id = nullptr) :
      /*  Rec(rmap({{"obj", obj}, {"inst", inst}}), OType::REC, REC_FURI),*/
      obj_(obj), inst_(inst) {

    }

    void split(const BCode_p &bcode, Deque<ptr<Monad>> *running) const {
      Obj_p next_obj;
      try {
        next_obj = this->inst_->apply(this->obj_);
      } catch (const fError &error) {
        throw fError("%s\n\t\t!rthrown when applying!! %s => %s", error.what(),
                     this->obj_->toString().c_str(),
                     this->inst_->toString().c_str());
      }
      const Inst_p next_inst = bcode->next_inst(this->inst_);
      if (!next_obj->is_noobj()) {
        LOG(DEBUG, FOS_TAB_2 "!mProcessing!! monad(s): %s\n", next_obj->toString().c_str());
        const List<Obj_p> objs = next_obj->is_objs() ? *next_obj->objs_value() : List<Obj_p>{next_obj};
        for (const auto &obj: objs) {
          if (!obj->is_noobj()) {
            if (this->inst_->inst_op() == "repeat") {
              const Obj_p until = this->inst_->inst_arg(1);
              const Obj_p emit = this->inst_->inst_arg(2);
              if (!emit->is_noobj() && !emit->apply(obj)->is_noobj()) {
                // repeat.emit
                const auto monad = make_shared<Monad>(obj, next_inst);
                running->push_back(monad);
                LOG(DEBUG, FOS_TAB_4 "!mEmitting!! monad: %s\n", monad->toString().c_str());
              }
              if (!until->is_noobj() && until->apply(obj)->is_noobj()) {
                // repeat.until
                const auto monad = make_shared<Monad>(obj, this->inst_);
                monad->loops_ = this->loops_ + 1;
                running->push_back(monad);
                LOG(DEBUG, FOS_TAB_4 "!mLooping!! monad: %s\n", monad->toString().c_str());
                continue;
              }
            }
            const auto monad = make_shared<Monad>(obj, next_inst);
            running->push_back(monad);
            LOG(DEBUG, FOS_TAB_4 "!mGenerating!! monad: %s\n", monad->toString().c_str());
          }
        }
      }
    }

    [[nodiscard]] Obj_p obj() const { return this->obj_; }

    [[nodiscard]] Inst_p inst() const { return this->inst_; }

    [[nodiscard]] long bulk() const { return this->bulk_; }

    [[nodiscard]] uint16_t loops() const { return this->loops_; }

    [[nodiscard]] bool halted() const { return this->inst_->is_noobj(); }

    [[nodiscard]] bool dead() const { return this->obj_->is_noobj(); }

    [[nodiscard]] string toString() const {
      return string("!MM!y[!!") + this->obj_->toString() + "!g@!!" + this->inst_->toString() + "!y]!!";
    }

    static ptr<Monad> create(const Obj_p &obj, const Inst_p &inst) {
      return make_shared<Monad>(obj, inst);
    }
  };

  using Monad_p = ptr<Monad>;



}
#endif