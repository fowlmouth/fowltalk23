#include "cli.h"

CLI& CLI::on(const char* option, std::function< void() > callback)
{
  options[option] = { option, false, [=](std::string_view){ callback(); } };
  return *this;
}

CLI& CLI::on(const char* option, std::function< void(std::string_view) > callback)
{
  options[option] = { option, true, callback };
  return *this;
}

CLI& CLI::on(const char* option, bool& flag_present)
{
  options[option] = { option, false, [&flag_present](std::string_view){ flag_present = true; } };
  return *this;
}

CLI& CLI::on_argument(std::function< void(std::string_view) > callback)
{
  argument_callback = callback;
  return *this;
}

void CLI::parse(int argc, const char** argv) const
{
  for(int i = 1; i < argc; ++i)
  {
    auto arg = argv[i];
    if(!strcmp(arg, "--"))
    {
      // parse the rest as arguments
      ++i;
      for(; i < argc; ++i)
      {
        if(argument_callback)
        {
          argument_callback(argv[i]);
        }
      }
      break;
    }

    if(arg[0] == '-')
    {
      std::string_view option(arg);
      std::string_view value;
      const auto equals_position = option.find('=');
      const auto has_equals = equals_position != std::string_view::npos;

      if(has_equals)
      {
        value = option.substr(equals_position + 1);
        option = option.substr(0, equals_position);
        if(auto it = options.find(option); it != options.end())
        {
          it->second.callback(value);
        }
      }

      if(auto it = options.find(option); it != options.end())
      {
        if(it->second.value_required)
        {
          if(!has_equals)
          {
            value = argv[++i];
          }
          it->second.callback(value);
        }
        else
        {
          it->second.callback("");
        }
      }
    }
    else if(argument_callback)
    {
      argument_callback(arg);
    }
  }
}
