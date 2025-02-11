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

  protected:
    static ptr<Text> get_or_create(const Obj_p &text) {
      const ID_p text_state_id = id_p(text->uri_value());
      if(!GLOBAL::singleton()->exists(text_state_id))
        GLOBAL::singleton()->store(text_state_id, make_shared<Text>());
      return GLOBAL::singleton()->load<ptr<Text>>(text_state_id);
    }

    static bool postfix_match(const string &text, const string &postfix) {
      return text.length() >= postfix.length() && text.substr(text.length() - postfix.length()) == postfix;
    }

    static Obj_p edit_inst(const Obj_p &text_obj, const InstArgs &) {
      const ID_p text_state_id = id_p(text_obj->uri_value());
      const ptr<Text> text_state = Text::get_or_create(text_obj);
      Ansi<>::singleton()->printf("!m[!y:s!g(!bave!g) !y:p!g(!barse!g) !y:q!g(!buit!g)!m]!b %s!!\n",
                                  text_state_id->toString().c_str());
      if(!text_state->body.empty())
        Ansi<>::singleton()->printf("%s", text_state->body.c_str());
      auto temp = string(text_state->body);
      while(true) {
        FEED_WATCDOG();
        temp += static_cast<char>(Terminal::STD_IN_DIRECT()->int_value());
        ///////////////// SAVE /////////////////
        if(Text::postfix_match(temp, ":s")) {
          temp = temp.substr(0, temp.length() - 2);
          text_state->body = temp;
          GLOBAL::singleton()->store(text_state_id, text_state);
          LOG_OBJ(INFO, text_obj, "!b%s !ysource code!! saved\n", text_obj->vid->toString().c_str());
          Ansi<>::singleton()->print(text_state->body.c_str());
          Ansi<>::singleton()->flush();
        }
        ///////////////// PARSE /////////////////
        else if(Text::postfix_match(temp, ":p")) {
          temp = temp.substr(0, temp.length() - 2);
          StringHelper::trim(temp);
          const Obj_p obj = mmadt::Parser::singleton()->parse(temp.c_str());
          Router::singleton()->write(text_state_id, obj);
          LOG_OBJ(INFO, text_obj, "!b%s !ysource %s!! parsed\n",
                  text_state_id->toString().c_str(),
                  obj->toString().c_str());
          Ansi<>::singleton()->print(temp.c_str());
          Ansi<>::singleton()->flush();
        }
        ///////////////// QUIT /////////////////
        else if(Text::postfix_match(temp, ":q")) {
          break;
        }
      }
      return text_obj;
    }

  public:
    static void *import() {
      //////////////////////
      Typer::singleton()->save_type(TEXT_FURI, Obj::to_type(URI_FURI));
      ///////////////////////////////////////////////////////
      InstBuilder::build(TEXT_FURI->add_component("edit"))
          ->domain_range(TEXT_FURI, {1, 1}, TEXT_FURI, {1, 1})
          ->inst_f([](const Obj_p &text, const InstArgs &args) {
            return Text::edit_inst(text, args);
          })->save();
      return nullptr;
    }
  };
}
#endif
