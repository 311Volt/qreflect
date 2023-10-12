#include <qreflect.hpp>

#include <iostream>
#include <stdint.h>
#include <string>
#include <sstream>

struct Consents {
	bool emailContact = false;
	bool phoneContact = false;
};

struct User {
	std::string firstName;
	std::string lastName;
	int age;
	Consents consents;
};

QREFL_DESCRIBE(Consents, emailContact, phoneContact)
QREFL_DESCRIBE(User, firstName, lastName, age, consents)

template<typename T>
	requires requires(T v, std::ostream& os){os << v;}
std::string to_str(T&& value, int indent = 0)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template<qreflect::inspectable T>
std::string to_str(T&& value, int indent = 0)
{
	std::stringstream ss;
    ss << "\n";
	qreflect::for_each_member_of(value, [&](const std::string_view name, auto&& value){
		ss << std::string(indent, ' ') << name << ": " << to_str(value, indent+4) << "\n";
	});
	return ss.str();
}

int main() {
	User exampleUser {
		.firstName = "Gabe",
		.lastName = "Itch",
		.age = 27,
		.consents = {
			.emailContact = true,
			.phoneContact = false
		}
	};
	std::cout << to_str(exampleUser);
}