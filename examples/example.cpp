#include <qreflect.hpp>

#include <iostream>
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

int main() {
	for(auto& s: qreflect::field_names_v<User>) {
		std::cout << s << "\n";
	}
}


int old_main() {
	// TODO restore
	// User exampleUser {
	// 	.firstName = "Gabe",
	// 	.lastName = "Itch",
	// 	.age = 27,
	// 	.consents = {
	// 		.emailContact = true,
	// 		.phoneContact = false
	// 	}
	// };
	// std::cout << to_str(exampleUser);
	return 0;
}
