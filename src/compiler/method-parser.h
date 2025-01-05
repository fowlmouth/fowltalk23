#pragma once

#include "method-builder.h"

class MethodParser : public ParserActions
{
  MethodBuilder* builder_;

public:
  MethodParser(MethodBuilder& builder);

protected:
  bool accept_integer(intmax_t value) override;
  bool accept_identifier(std::string_view name) override;
  bool accept_send(std::string_view selector, int arity) override;
  bool accept_assignment(std::string_view name) override;
};
