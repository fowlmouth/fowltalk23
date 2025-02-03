#include "utest.h"
#include "parser.h"

struct CountingResults
{
  int count_integer = 0,
      count_identifier = 0,
      count_send = 0,
      count_assignment = 0;
};

class CountingParser : public ParserActions
{
public:
  CountingResults& results;

  CountingParser(CountingResults& results) : results(results) {}

  bool accept_integer(intmax_t value) override
  {
    ++results.count_integer;
    return true;
  }
  bool accept_identifier(std::string_view name) override
  {
    ++results.count_identifier;
    return true;
  }
  bool accept_send(std::string_view selector, int arity) override
  {
    ++results.count_send;
    return true;
  }
  bool accept_assignment(std::string_view name) override
  {
    ++results.count_assignment;
    return true;
  }
};

UTEST(parser, integer) {
  const char *code = "42";
  CountingResults results;
  Parser parser(code, std::make_unique<CountingParser>(results));
  parser.parse_expression();

  EXPECT_EQ(results.count_integer, 1);
  EXPECT_EQ(results.count_identifier, 0);
  EXPECT_EQ(results.count_send, 0);
  EXPECT_EQ(results.count_assignment, 0);
}

UTEST(parser, identifier_foo) {
  const char *code = "foo";
  CountingResults results;
  Parser parser(code, std::make_unique<CountingParser>(results));
  parser.parse_expression();

  EXPECT_EQ(results.count_integer, 0);
  EXPECT_EQ(results.count_identifier, 1);
  EXPECT_EQ(results.count_send, 0);
  EXPECT_EQ(results.count_assignment, 0);
}

UTEST(parser, send_foo_bar) {
  const char *code = "foo bar";
  CountingResults results;
  Parser parser(code, std::make_unique<CountingParser>(results));
  parser.parse_expression();

  EXPECT_EQ(results.count_integer, 0);
  EXPECT_EQ(results.count_identifier, 1);
  EXPECT_EQ(results.count_send, 1);
  EXPECT_EQ(results.count_assignment, 0);
}

UTEST(parser, assignment_foo_bar) {
  const char *code = "foo := bar";
  CountingResults results;
  Parser parser(code, std::make_unique<CountingParser>(results));
  parser.parse_statement();

  EXPECT_EQ(results.count_integer, 0);
  EXPECT_EQ(results.count_identifier, 1);
  EXPECT_EQ(results.count_send, 0);
  EXPECT_EQ(results.count_assignment, 1);
}
