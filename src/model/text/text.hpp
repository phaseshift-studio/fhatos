#pragma once
#ifndef fhatos_text_hpp
#define fhatos_text_hpp

#include "../../fhatos.hpp"
#include "../../lang/type.hpp"
#include "../../lang/obj.hpp"
#include "../../util/obj_helper.hpp"
#include "../../util/global.hpp"
#include "../../lang/mmadt/parser.hpp"

namespace fhatos {
  static ID_p TEXT_FURI = id_p(FOS_URI "/text");

  class Text final : public enable_shared_from_this<Text> {
    std::string body;
    ID_p save_id;

  public:
    explicit Text(const ID_p &save) :
      body(""), save_id(save) {
    }

    static char read_stdin() {
      return static_cast<char>(Terminal::STD_IN_DIRECT()->int_value());
    }

    static bool postfix_match(const string &text, const string &postfix) {
      return text.length() >= postfix.length() && text.substr(text.length() - postfix.length()) == postfix;
    }

    static ptr<Text> get_or_create(const ID_p &save_id) {
      if(!GLOBAL::singleton()->exists(save_id))
        GLOBAL::singleton()->store(save_id, make_shared<Text>(save_id));
      return GLOBAL::singleton()->load<ptr<Text>>(save_id);
    }

    void save() {
      GLOBAL::singleton()->store(this->save_id, this->shared_from_this());
    }

    static void *import() {
      //////////////////////
      Typer::singleton()->save_type(TEXT_FURI, Obj::to_type(URI_FURI));
      ///////////////////////////////////////////////////////
      InstBuilder::build(TEXT_FURI->add_component("edit"))
          ->domain_range(TEXT_FURI, {1, 1}, TEXT_FURI, {1, 1})
          ->inst_f([](const Obj_p &text, const InstArgs &) {
            const ptr<Text> state = Text::get_or_create(id_p(text->uri_value()));
            Ansi<>::singleton()->printf("!m[!y:s!g(!bave!g) !y:p!g(!barse!g) !y:q!g(!buit!g)!m]!b %s!!\n",
                                        state->save_id->toString().c_str());
            if(!state->body.empty())
              Ansi<>::singleton()->printf("%s", state->body.c_str());
            auto temp = string(state->body);
            while(true) {
              FEED_WATCDOG();
              temp += read_stdin();
              if(Text::postfix_match(temp, ":s")) {
                temp = temp.substr(0, temp.length() - 2);
                state->save();
                LOG_OBJ(INFO, text, "!b%s !ysource code!! saved\n", text->vid_->toString().c_str());
                Ansi<>::singleton()->print(state->body.c_str());
                Ansi<>::singleton()->flush();
              } else if(Text::postfix_match(temp, ":p")) {
                temp = temp.substr(0, temp.length() - 2);
                StringHelper::trim(temp);
                const Obj_p obj = mmadt::Parser::singleton()->parse(temp.c_str());
                Router::singleton()->write(state->save_id, obj);
                LOG_OBJ(INFO, text, "!b%s !ysource %s!! parsed\n",
                        state->save_id->toString().c_str(),
                        obj->toString().c_str());
                Ansi<>::singleton()->print(state->body.c_str());
                Ansi<>::singleton()->flush();
              } else if(Text::postfix_match(temp, ":q")) {
                break;
              }
            }
            return text;
          })->save();
      return nullptr;
    }
  };
}
#endif
