#include <iostream>

#include "compiler.hpp"
#include "lexer.hpp"
#include "token.hpp"

namespace orchid::compiler
{
  int compile(std::string_view src)
  {
    Lexer lexer{src};

    while (true)
    {
      auto t = lexer.next_token();

      if (t.type == TokenType::Eof || t.type == TokenType::Unknown)
      {
        std::cout << std::endl;
        break;
      }

      auto s = src.substr(t.pos, t.len);
      std::cout << "(" << s << ", " << t.line << ", " << t.column << ")\n";
    }

    return 0;
  }
} // namespace orchid::compiler
