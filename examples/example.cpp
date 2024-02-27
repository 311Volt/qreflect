#include <qreflect.hpp>

#include <iostream>
#include <sstream>
#include <string>

struct Consents {
	bool emailContact = false;
	bool phoneContact = false;

	static constexpr std::tuple qrefl_fields = {
		&Consents::emailContact,
		&Consents::phoneContact
	};
};
static_assert(qreflect::has_field_info<Consents>);

struct User {
	std::string firstName;
	std::string lastName;
	int age;
	Consents consents;

	static constexpr std::tuple qrefl_fields = {
		&User::firstName,
		&User::lastName,
		&User::age,
		&User::consents
	};
};
static_assert(qreflect::has_field_info<User>);


template<typename T>
	requires (not qreflect::has_field_info<T>)
std::string to_str(const T& v, int indent = 0) {
	std::stringstream ss;
	ss << v;
	return ss.str();
}

template<qreflect::has_field_info T>
std::string to_str(const T& v, int indent = 0) {
	std::stringstream ss;
	ss << "\n";
	qreflect::for_each_member_of(v, [&](const std::string_view name, auto&& value) {
		ss << std::string(indent, ' ') << name << " = " << to_str(value, indent+2) << "\n";
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
