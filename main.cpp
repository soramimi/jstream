
#include <jstream.h>
#include <stdio.h>

void reader()
{
	char const *json = R"---(
{
	"name":"Alice",
	"city":"Wonderland"
}
)---";

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{name")) {
			printf("%s\n", r.string().c_str());
		} else if (r.match("{city")) {
			printf("%s\n", r.string().c_str());
		}
	}
}

void writer()
{
	jstream::Writer w;
	w.object({}, [&]() {
		w.string("name", "John Doe");
		w.string("age", "30");
		w.string("city", "New York");
	});
	std::string s = w;
	puts(s.c_str());
}

int main()
{
	reader();
	puts("---");
	writer();
	return 0;
}
