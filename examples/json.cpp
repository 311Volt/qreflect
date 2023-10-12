#include <qreflect.hpp>
#include <nlohmann/json.hpp>
#include <concepts>
#include <iostream>

namespace nl = nlohmann;

template<typename T>
	requires std::constructible_from<nl::json, T>
nl::json to_json(const T& value)
{
	return value;
}

template<qreflect::inspectable T>
nl::json to_json(const T& object)
{
	nl::json result = nl::json::object();
	qreflect::for_each_member_of(object, [&](const std::string_view key, auto&& value){
		result[key] = to_json(value);
	});
	return result;
}


struct Inner {
	std::string a;
	int b;
	int c;
};
QREFL_DESCRIBE(Inner, a, b, c);

struct Outer {
	float d;
	char e;
	Inner f;
};
QREFL_DESCRIBE(Outer, d, e, f);

int main() {
	Outer test = {
		.d = 5.56,
		.e = 'k',
		.f = {
			.a = "This is a test",
			.b = 6,
			.c = 7
		}
	};
	std::cout << to_json(test).dump(4);
}