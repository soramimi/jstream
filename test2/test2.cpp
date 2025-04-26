
#include "../test.h"
#include <gtest/gtest.h>
#include <variant>

//

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
		if (r.match("{responses[*") && r.is_start_object()) {
			ParsedData::Response response;
			r.nest();
			do {
				if (r.match("{responses[{responseId")) {
					response.resourceId = r.string();
				} else if (r.match("{responses[{createTime")) {
					response.createTime = r.string();
				} else if (r.match("{responses[{lastSubmittedTime")) {
					response.lastSubmittedTime = r.string();
				} else if (r.match("{responses[{answers{*")) {
					if (r.is_start_object()) {
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
				}
			} while (r.next());
			parsed.responses.push_back(response);
		}
	}

	assert(parsed.responses.size() == 1);
	EXPECT_EQ(parsed.responses[0].resourceId, "ACYDBNiWZb5bf6PCC964lJBNSaQfEjfc4rCTBMaAjb62EhOwJMqXCxS9NreBtx7DxOWlpRA");
	assert(parsed.responses[0].answers.size() == 3);
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

