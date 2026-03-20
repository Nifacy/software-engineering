#include <greeting.hpp>

#include <userver/utest/utest.hpp>

using api_gateway::UserType;

UTEST(SayHelloTo, Basic) {
  EXPECT_EQ(api_gateway::SayHelloTo("Developer", UserType::kFirstTime),
            "Hello, Developer!\n");
  EXPECT_EQ(api_gateway::SayHelloTo({}, UserType::kFirstTime),
            "Hello, unknown user!\n");

  EXPECT_EQ(api_gateway::SayHelloTo("Developer", UserType::kKnown),
            "Hi again, Developer!\n");
  EXPECT_EQ(api_gateway::SayHelloTo({}, UserType::kKnown),
            "Hi again, unknown user!\n");
}