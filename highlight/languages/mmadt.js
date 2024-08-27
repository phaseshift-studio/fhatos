/*
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
 */

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

/*! `mmadt` grammar compiled for Highlight.js 11.10.0 */
(function () {
  var hljsGrammar = (function () {
    'use strict';

    /*
        Language: mmadt
        Author: Dogturd Stynx
    */

    /** @type LanguageFn */
    function mmadt(hljs) {
      const regex = hljs.regex;
      const VAR = {};
      const BRACED_VAR = {
        begin: /\$\{/,
        end: /\}/,
        contains: [
          "self",
          {
            begin: /:-/,
            contains: [VAR]
          } // default values
        ]
      };
      Object.assign(VAR, {
        className: 'variable',
        variants: [
          {
            begin: regex.concat(/\$[\w\d#@][\w\d_]*/,
                // negative look-ahead tries to avoid matching patterns that are not
                // Perl at all like $ident$, @ident@, etc.
                `(?![\\w\\d])(?![$])`)
          },
          BRACED_VAR
        ]
      });

      const SUBST = {
        className: 'subst',
        begin: /\$\(/,
        end: /\)/,
        contains: [hljs.BACKSLASH_ESCAPE]
      };
      const COMMENT = hljs.inherit(
          hljs.COMMENT(),
          {
            match: [
              /(^|\s)/,
              /#.*$/
            ],
            scope: {
              2: 'comment'
            }
          }
      );
      const HERE_DOC = {
        begin: /<<-?\s*(?=\w+)/,
        starts: {
          contains: [
            hljs.END_SAME_AS_BEGIN({
              begin: /(\w+)/,
              end: /(\w+)/,
              className: 'string'
            })
          ]
        }
      };
      const QUOTE_STRING = {
        className: 'string',
        begin: /'/,
        end: /'/,
        contains: [
          hljs.BACKSLASH_ESCAPE,
          VAR,
          SUBST
        ]
      };
      SUBST.contains.push(QUOTE_STRING);
      const ESCAPED_QUOTE = {
        match: /\\"/
      };
      const APOS_STRING = {
        className: 'string',
        begin: /'/,
        end: /'/
      };
      const ESCAPED_APOS = {
        match: /\\'/
      };
      const ARITHMETIC = {
        begin: /\$?\(\(/,
        end: /\)\)/,
        contains: [
          {
            begin: /\d+#[0-9a-f]+/,
            className: "number"
          },
          hljs.NUMBER_MODE,
          VAR
        ]
      };
      const SH_LIKE_SHELLS = [
        "fish",
        "bash",
        "zsh",
        "sh",
        "csh",
        "ksh",
        "tcsh",
        "dash",
        "scsh",
      ];
      const FUNCTION = {
        className: 'function',
        begin: /\w[\w\d_]*\s*\(\s*\)\s*\{/,
        returnBegin: true,
        contains: [hljs.inherit(hljs.TITLE_MODE, {begin: /\w[\w\d_]*/})],
        relevance: 10
      };

      const KEYWORDS = [
        "plus",
        "mult",
        "get",
        "switch",
        "to",
        "from",
        "print",
        "define",
        ///
        "thread",
        "setup",
        "loop",
        "stop",
        "nat",

        ///
        "abc"

      ];

      const SHELL_BUILT_INS = [
        "fhatos>",
        "==>",
        "=>"
      ]

      const LITERALS = [
        "true",
        "false"
      ];

      // to consume paths to prevent keyword matches inside them
      const PATH_MODE = {match: /(\/[a-z._-]+)+/};

      return {
        name: 'mmadt',
        aliases: [],
        keywords: {
          $pattern: /\b[a-z][a-z0-9._-]+\b/,
          keyword: KEYWORDS,
          literal: LITERALS,
          built_in: [
            ...SHELL_BUILT_INS,
          ]
        },
        contains: [
          FUNCTION,
          ARITHMETIC,
          COMMENT,
          HERE_DOC,
          PATH_MODE,
          QUOTE_STRING,
          ESCAPED_QUOTE,
          APOS_STRING,
          ESCAPED_APOS,
          VAR
        ]
      };
    }

    return mmadt;

  })();

  hljs.registerLanguage('mmadt', hljsGrammar);
})();