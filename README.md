# Quick Reflection

A C++20 single-header library for reflection on aggregate types, designed mainly for serialization.

The library is meant as a tool for making your own serializers and is not concerned
with any specific file format or protocol, but it can be effective in combination with libraries that are - for
an example, see [Example: nlohmann/json](#example-nlohmannjson).


## Installation

Copy [include/qreflect.hpp]() to your include path.

If using CMake, you may use this repo as a submodule and add
```cmake
add_subdirectory(qreflect)
target_link_libraries(myproject PRIVATE qreflect)
```
to your CMakeLists.txt.

## Introduction and usage

Given
```c++
struct User {
    std::string name;
    std::string email;
    int age;
};
```

we may write the following at global namespace scope:

```c++
QREFL_DESCRIBE(User, name, email, age)
```

This macro expands to specializations of `qreflect::type_info` and `qreflect::basic_field_info` templates,
giving us access to information about the structure:

```c++
static_assert(qreflect::type_info<User>::name == "User");
static_assert(qreflect::type_info<User>::num_fields == 3);
static_assert(qreflect::basic_field_info<User, 0>::name == "name");
static_assert(qreflect::basic_field_info<User, 1>::name == "email");
static_assert(qreflect::basic_field_info<User, 2>::name == "age");
static_assert(std::is_same_v<int, typename qreflect::basic_field_info<User, 2>::type>);
```

Redundancy between the definition and description is unavoidable, so `qreflect` does the
next best thing by failing the compilation if you don't list all members:

```c++
QREFL_DESCRIBE(User, name, email) // compilation error
QREFL_DESCRIBE(User, name, email, age) // OK
QREFL_DESCRIBE(User, name, email, age, oldRemovedThing) // compilation error
```

This does not work for non-aggregates, so you can opt out of this behavior by replacing
`QREFL_DESCRIBE` with `QREFL_DESCRIBE_PARTIAL`:

```c++
QREFL_DESCRIBE_PARTIAL(User, name, email) // OK
```

Convenience functions are provided to iterate over members:

```c++
User user1 = {
    .name = "jdaciuk",
    .email = "deathtojs@jandaciuk.pl",
    .age = 61
};

qreflect::for_each_member_of(user1, [&](const std::string_view key, auto&& value){
    std::cout << std::format("{} = {}\n", key, value);
});
```
Running the above code will print:
```
name = jdaciuk
email = deathtojs@jandaciuk.pl
age = 61
```

A similar function is offered to iterate only over member declarations, which
will be demonstrated in the [DDL example](#example-sql-ddl).

## Example: nlohmann/json

An example usage is serializing structs to JSON using the well-known library 
[nlohmann/json](https://github.com/nlohmann/json).  The example's source is available at [examples/json.cpp]() 
and will be built automatically if CMake can find nlohmann/json.


```c++
#include <qreflect.hpp>
#include <nlohmann/json.hpp>

template<typename T>
	requires std::constructible_from<nlohmann::json, T>
nlohmann::json to_json(T&& value)
{
	return value;
}

template<qreflect::inspectable T>
nlohmann::json to_json(T&& object)
{
	nlohmann::json result = nlohmann::json::object();
	qreflect::for_each_member_of(object, [&](const std::string_view key, auto&& value) {
		result[key] = to_json(value);
	});
	return result;
}
```
The first overload utilizes nlohmann/json's builtin conversions from numbers and strings to JSON values, while
the second overload is concerned with mapping structs to JSON objects using qreflect.

The 16 lines above turn out to be enough to deal with this data structure that features a nested object:
```c++
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
```

If, given the above, we define a `main` function like so

```c++
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
```
the program will print:

```
{
    "d": 5.559999942779541,
    "e": 107,
    "f": {
        "a": "This is a test",
        "b": 6,
        "c": 7
    }
}
```

## Example: SQL DDL

The example [sqlite_ddl.cpp](examples/sqlite_ddl.cpp) utilizes `qreflect::for_each_member_decl`
to create SQLite DDL for a reflected struct.

## Advanced usage

### `QREFL_MAX_ARITY`
The library is guaranteed to correctly handle aggregates that have at most
`QREFL_MAX_ARITY` members. This macro is defined as 32 by default, and may be
set by the user as high as 512. The value affects preprocessing times and
memory usage due to the nature of for-each macros used by qreflect, and its
effect is especially noticeable when using code completion tools such as clangd.

If you need more than 32 fields, redefine the value before including qreflect:
```c++
#define QREFL_MAX_ARITY 128
#include <qreflect.hpp>
```

### qreflect::type_info

Given
```c++
struct User {
    std::string name;
    std::string email;
    int age;
};
```

the type `qreflect::type_info<Example>`
has a definition equivalent to:

```c++
/* namespace qreflect */
template<>
struct type_info<User> {
    using type = User;
    static inline constexpr int num_fields = 3;
    static inline constexpr std::string_view name = "User";
};
```

### qreflect::basic_field_info
Given `struct User` from the previous section, the type `qreflect::basic_field_info<User, 2>`
has a definition equivalent to:

```c++
/* namespace qreflect */
template<>
struct basic_field_info<User, 2> {
    using parent_type = User;
    using type = int;   
    static inline constexpr int index = 2; 
    static inline constexpr std::string_view name = "age"; 
    static inline const auto ptr_to_member = &User::age; 
};
```


## TODO

 - `QREFL_DESCRIBE_METHODS`
 - annotations (somehow)

<hr />

WG21 give us reflection pls :(