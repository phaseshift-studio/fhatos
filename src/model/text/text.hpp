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

  class Text final {
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
      if(text.length() < postfix.length())
        return false;
      //sAnsi<>::singleton()->printf("%s==%s\n", text.c_str(), postfix.c_str());
      return text.substr(text.length() - postfix.length()) == postfix;
    }

    static ptr<Text> get_or_create(const ID_p &save_id) {
      if(!GLOBAL::singleton()->exists(save_id))
        GLOBAL::singleton()->store(save_id, make_shared<Text>(save_id));
      return GLOBAL::singleton()->load<ptr<Text>>(save_id);
    }

    void save() {
      GLOBAL::singleton()->store(this->save_id, ptr<Text>(this));
    }

    static void *import() {
      //////////////////////
      Typer::singleton()->save_type(TEXT_FURI, Obj::to_type(URI_FURI));
      ///////////////////////////////////////////////////////
      InstBuilder::build(TEXT_FURI->add_component("edit"))
          ->domain_range(TEXT_FURI, {1, 1}, TEXT_FURI, {1, 1})
          ->inst_f([](const Obj_p &text, const InstArgs &) {
            const ptr<Text> state = Text::get_or_create(id_p(text->uri_value()));
            Ansi<>::singleton()->printf("!m[!y:s!g(!bave!g) !y:p!g(!barsing!g) !y:q!g(!buit!g)!m]!b %s!!\n",
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
              } else if(Text::postfix_match(temp, ":p")) {
                temp = temp.substr(0, temp.length() - 2);
                StringHelper::trim(temp);
                LOG_OBJ(INFO, text, "!yparsing and storing obj!! %s => !b%s!!\n",
                        temp.c_str(),
                        state->save_id->toString().c_str());
                const Obj_p obj = mmadt::Parser::singleton()->parse(temp.c_str());
                LOG_OBJ(INFO, text, FOS_TAB_2 "%s\n", obj->toString().c_str());
                state->body = temp;
                Router::singleton()->write(state->save_id, obj);
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
