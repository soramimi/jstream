
#include "test.h"

const char *state_to_string(jstream::StateType state)
{
	switch (state) {
	case jstream::None:          return "None";
	case jstream::Null:          return "Null";
	case jstream::False:         return "False";
	case jstream::True:          return "True";
	case jstream::Key:           return "Key";
	case jstream::Comma:         return "Comma";
	case jstream::StartObject:   return "StartObject";
	case jstream::EndObject:     return "EndObject";
	case jstream::StartArray:    return "StartArray";
	case jstream::EndArray:      return "EndArray";
	case jstream::String:        return "String";
	case jstream::Number:        return "Number";
	}
	assert(false);
	return "Invalid";
}

bool Event::operator == (const Event &other) const
{
	if (state != other.state) return false;
	if (key != other.key) return false;
	if (value != other.value) return false;
	if (path != other.path) return false;
	return true;
}

void Event::puts() const
{
	char const *statetype = state_to_string(state);
	printf(R"---({%s, "%s", "%s", "%s"},)---""\n"
		   , statetype
		   , key.c_str()
		   , value.c_str()
		   , path.c_str()
		   );
}

std::vector<Event> parse_to_events(char const *json)
{
	std::vector<Event> events;

	jstream::Reader r(json);
	while (r.next()) {
		Event event;
		event.state = r.state();
		event.path = r.path();
		switch (r.state()) {
		case jstream::Key:
		case jstream::StartObject:
		case jstream::EndObject:
		case jstream::StartArray:
		case jstream::EndArray:
			event.key = r.key();
			break;
		default:
			if (r.isvalue()) {
				event.value = r.string();
			}
			break;
		}
		events.push_back(event);
	}

	return events;
}

void print_events(std::vector<Event> const &events)
{
	for (auto const &event : events) {
		event.puts();
	}
}

void print_events(char const *json)
{
	std::vector<Event> events = parse_to_events(json);
	print_events(events);
}
