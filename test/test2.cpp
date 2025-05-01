
#include "test.h"
#include <gtest/gtest.h>

using namespace jstream;

TEST(Json, GoogleForms1)
{
	char const *json = R"---(
{
  "responses": [
	{
	  "responseId": "ACYDBNiWZb5bf6PCC964lJBNSaQfEjfc4rCTBMaAjb62EhOwJMqXCxS9NreBtx7DxOWlpRA",
	  "createTime": "2024-06-19T16:05:51.394Z",
	  "lastSubmittedTime": "2024-06-19T16:05:51.394652Z",
	  "answers": {
		"704f9a53": {
		  "questionId": "704f9a53",
		  "textAnswers": {
			"answers": [
			  {
				"value": "soramimi"
			  }
			]
		  }
		},
		"4842f816": {
		  "questionId": "4842f816",
		  "textAnswers": {
			"answers": [
			  {
				"value": "L"
			  }
			]
		  }
		},
		"1e9dfb54": {
		  "questionId": "1e9dfb54",
		  "textAnswers": {
			"answers": [
			  {
				"value": "test"
			  }
			]
		  }
		}
	  }
	}
  ]
}
)---";

	struct ParsedData {
		struct Response {
			std::string resourceId;
			std::string createTime;
			std::string lastSubmittedTime;
			struct Answers {
				std::string key;
				std::string questionId;
				struct TextAnswers {
					struct Answer {
						std::string value;
					};
					std::vector<Answer> answers;
				} textAnswers;
			};
			std::vector<Answers> answers;
		};
		std::vector<Response> responses;
	} parsed;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match_start_object("{responses[*")) {
			ParsedData::Response response;
			r.nest();
			do {
				if (r.match("{responses[{responseId")) {
					response.resourceId = r.string();
				} else if (r.match("{responses[{createTime")) {
					response.createTime = r.string();
				} else if (r.match("{responses[{lastSubmittedTime")) {
					response.lastSubmittedTime = r.string();
				} else if (r.match_start_object("{responses[{answers{*")) {
					r.nest();
					ParsedData::Response::Answers answers;
					answers.key = r.key();
					do {
						if (r.match("{responses[{answers{*{questionId")) {
							answers.questionId = r.string();
						} else if (r.match("{responses[{answers{*{textAnswers{answers[{value")) {
							ParsedData::Response::Answers::TextAnswers::Answer a;
							a.value = r.string();
							answers.textAnswers.answers.push_back(a);
						}
					} while (r.next());
					response.answers.push_back(answers);
				}
			} while (r.next());
			parsed.responses.push_back(response);
		}
	}

	ASSERT_EQ(parsed.responses.size(), 1);
	EXPECT_EQ(parsed.responses[0].resourceId, "ACYDBNiWZb5bf6PCC964lJBNSaQfEjfc4rCTBMaAjb62EhOwJMqXCxS9NreBtx7DxOWlpRA");
	ASSERT_EQ(parsed.responses[0].answers.size(), 3);
	EXPECT_EQ(parsed.responses[0].answers[0].key, "704f9a53");
	EXPECT_EQ(parsed.responses[0].answers[0].questionId, "704f9a53");
	EXPECT_EQ(parsed.responses[0].answers[0].textAnswers.answers[0].value, "soramimi");
	EXPECT_EQ(parsed.responses[0].answers[1].key, "4842f816");
	EXPECT_EQ(parsed.responses[0].answers[1].questionId, "4842f816");
	EXPECT_EQ(parsed.responses[0].answers[1].textAnswers.answers[0].value, "L");
	EXPECT_EQ(parsed.responses[0].answers[2].key, "1e9dfb54");
	EXPECT_EQ(parsed.responses[0].answers[2].questionId, "1e9dfb54");
	EXPECT_EQ(parsed.responses[0].answers[2].textAnswers.answers[0].value, "test");


}

TEST(Json, OAuth1)
{
	char const *json = R"---(
{
  "access_token": "qwerty123",
  "expires_in": 3599,
  "scope": "https://www.googleapis.com/auth/userinfo.profile",
  "token_type": "Bearer",
  "id_token": "abcdefg"
}
)---";

	struct ParsedData {
		std::string access_token;
		double expires_in;
		std::string scope;
		std::string token_type;
		std::string id_token;
	} parsed;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{access_token")) {
			parsed.access_token = r.string();
		} else if (r.match("{expires_in")) {
			parsed.expires_in = r.number();
		} else if (r.match("{scope")) {
			parsed.scope = r.string();
		} else if (r.match("{token_type")) {
			parsed.token_type = r.string();
		} else if (r.match("{id_token")) {
			parsed.id_token = r.string();
		}
	}

	EXPECT_EQ(parsed.access_token, "qwerty123");
	EXPECT_EQ(parsed.expires_in, 3599);
	EXPECT_EQ(parsed.scope, "https://www.googleapis.com/auth/userinfo.profile");
	EXPECT_EQ(parsed.token_type, "Bearer");
	EXPECT_EQ(parsed.id_token, "abcdefg");
}

TEST(Json, MinioConfig1)
{
	char const *json = R"---(
{
		"version": "10",
		"aliases": {
				"mary": {
						"url": "http://192.168.0.80:9000",
						"accessKey": "aaaaaaaaaaaaaaaaaaaa",
						"secretKey": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
						"api": "s3v4",
						"path": "auto"
				},
				"play": {
						"url": "https://play.min.io",
						"accessKey": "cccccccccccccccccccc",
						"secretKey": "dddddddddddddddddddddddddddddddddddddddd",
						"api": "S3v4",
						"path": "auto"
				}
		}
}
)---";

	struct ParsedData {
		std::string version;
		struct Alias {
			std::string key;
			std::string url;
			std::string accessKey;
			std::string secretKey;
			std::string api;
			std::string path;
		};
		std::vector<Alias> aliases;
	} parsed;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{version")) {
			parsed.version = r.string();
		} else if (r.match_start_object("{aliases{*")) {
			ParsedData::Alias alias;
			alias.key = r.key();
			r.nest();
			do {
				if (r.match("{aliases{*{url")) {
					alias.url = r.string();
				} else if (r.match("{aliases{*{accessKey")) {
					alias.accessKey = r.string();
				} else if (r.match("{aliases{*{secretKey")) {
					alias.secretKey = r.string();
				} else if (r.match("{aliases{*{api")) {
					alias.api = r.string();
				} else if (r.match("{aliases{*{path")) {
					alias.path = r.string();
				}
			} while (r.next());
			parsed.aliases.push_back(alias);
		}
	}

	EXPECT_EQ(parsed.version, "10");
	ASSERT_EQ(parsed.aliases.size(), 2);
	EXPECT_EQ(parsed.aliases[0].key, "mary");
	EXPECT_EQ(parsed.aliases[0].url, "http://192.168.0.80:9000");
	EXPECT_EQ(parsed.aliases[0].accessKey, "aaaaaaaaaaaaaaaaaaaa");
	EXPECT_EQ(parsed.aliases[0].secretKey, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
	EXPECT_EQ(parsed.aliases[0].api, "s3v4");
	EXPECT_EQ(parsed.aliases[0].path, "auto");
	EXPECT_EQ(parsed.aliases[1].key, "play");
	EXPECT_EQ(parsed.aliases[1].url, "https://play.min.io");
	EXPECT_EQ(parsed.aliases[1].accessKey, "cccccccccccccccccccc");
	EXPECT_EQ(parsed.aliases[1].secretKey, "dddddddddddddddddddddddddddddddddddddddd");
	EXPECT_EQ(parsed.aliases[1].api, "S3v4");
	EXPECT_EQ(parsed.aliases[1].path, "auto");
}

