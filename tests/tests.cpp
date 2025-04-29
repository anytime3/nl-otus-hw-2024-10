#include <gtest/gtest.h>
#include "HttpServer.h"
//------------------------------------------------------------------------------
TEST(SessionIdTest, ExtractsCookieCorrectly) {
  HttpRequest req;
  req.set(http::field::cookie, "abc123");

  std::string sessionId;
  getSessionIdFromReq(req, sessionId);

  EXPECT_EQ(sessionId, "abc123");
}
//------------------------------------------------------------------------------
TEST(SessionIdTest, NoCookieHeader) {
  HttpRequest req;
  std::string sessionId;
  getSessionIdFromReq(req, sessionId);

  EXPECT_EQ(sessionId.empty(), true);
}
//------------------------------------------------------------------------------
TEST(SessionIdTest, GenerateResponseCorrectly) {
  HttpRequest req;
  HttpResponse res;
  std::string bodyMsg = "Not found";
  RequestMng reqMng;

  reqMng.generateResponse(req, bodyMsg, res, http::status::not_found);

  EXPECT_EQ(res.body(), bodyMsg);
  EXPECT_EQ(res.find(http::field::content_type)->value(), "text/html");
}
//------------------------------------------------------------------------------
TEST(SessionIdTest, GenerateResponseWithContentTypeCorrectly) {
  HttpRequest req;
  HttpResponse res;
  std::string bodyMsg = "Not found";
  std::string content = "application/json";
  RequestMng reqMng;

  reqMng.generateResponseWithContentType(req, bodyMsg, content, res, http::status::not_found);

  EXPECT_EQ(res.find(http::field::content_type)->value(), content);
}
//------------------------------------------------------------------------------
