# Quick Reflection

Yet another C++20 single-header library for static reflection on aggregates.

Here's a program:

```c++
#include <qreflect.hpp>
#include <iostream>

struct User {
	std::string name;
	std::string email;
	int age;
	
	static constexpr std::tuple qrefl_fields = {
		&User::name, &User::email, &User::age
	};
};

template<qreflect::has_field_info T>
void print_struct(const T& v) {
	qreflect::for_each_member_of(v, [](const std::string_view name, auto&& value){
		std::cout << name << " = " << value << "\n";
	});
}

int main() {
	User user1 = {
		.name = "Mike",
		.email = "mike800@gmail.com",
		.age = 26
	};
	print_struct(user1);
}
```

And here's what it prints:
```
name = Mike
email = mike800@gmail.com
age = 26
```

Currently, C++ does not offer static reflection and any workarounds to emulate it require significant compromises
on the programmer's side. The aim of this library is to increase the probability that you will find a reflection
library that offers a set of compromises that suits your use case well.

qreflect's main downside is that you have to manually create redundant type descriptions.
In exchange, it can offer:

 - O(n*log(n)) compilation time in the number of members
 - No abuse of structured bindings
 - No pseudo-recursive macros
 - Annotations support
 - Compile-time consistency checks
 - No practical upper limit on the arity
 - Partial description support for non-aggregates


## Basic usage




## TODO

 - a better README & docs
 - MSVC support
 - better API for annotation access

<hr />

WG21 give us reflection pls :(