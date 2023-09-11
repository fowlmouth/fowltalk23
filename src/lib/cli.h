#pragma once

#include <cstring>
#include <functional>
#include <string_view>
#include <unordered_map>

class CLI
{
  struct Option
  {
    const char* opt;
    bool value_required;
    std::function< void(std::string_view) > callback;
  };
  std::unordered_map< std::string_view, Option > options;
  std::function< void(std::string_view) > argument_callback;

public:
  CLI& on(const char* arg, std::function< void(std::string_view) >);
  CLI& on(const char* arg, std::function< void() >);

  CLI& on_argument(std::function< void(std::string_view) >);

  void parse(int argc, const char** argv) const;
};
