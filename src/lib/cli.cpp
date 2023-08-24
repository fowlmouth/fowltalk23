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
      if(auto it = options.find(option); it != options.end())
      {
        if(it->second.value_required)
        {
          it->second.callback(argv[++i]);
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
