#pragma once

#include "method-builder.h"

class MethodParser : public Parser
{
  MethodBuilder* builder_;

public:
  MethodParser(std::string_view input, MethodBuilder& builder);

protected:
  bool accept_integer(intmax_t value) override;
  bool accept_send(std::string_view selector, int arity) override;
  bool accept_assignment(std::string_view name) override;
};
