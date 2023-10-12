#include <qreflect.hpp>

#include <string>
#include <iostream>
#include <format>

template<typename T>
struct primary_key {
	struct primary_key_tag {};
	T value;
};

template<typename T>
std::string sql_type_name()
{
	if constexpr (requires{typename T::primary_key_tag;}) {
		return sql_type_name<decltype(std::declval<T>().value)>() + " PRIMARY KEY";
	} else if constexpr (std::integral<T>) {
		return "INTEGER";
	} else if constexpr (std::floating_point<T>) {
		return "REAL";
	} else if constexpr (std::convertible_to<T, std::string_view>) {
		return "TEXT";
	} else {
		return "NONE";
	}
}

template<qreflect::inspectable T>
std::string create_ddl()
{
	std::string result;
	result += std::format("CREATE TABLE {} (\n", qreflect::type_info<T>::name);
	qreflect::for_each_member_decl<T>([&]<qreflect::field_info_type FieldInfoT>(){
		result += std::format("    {} {},\n", FieldInfoT::name, sql_type_name<typename FieldInfoT::type>());
	});
	result += ");";
	return result;
}

struct Table {
	primary_key<int> id;
	float a;
	char b;
};
QREFL_DESCRIBE(Table, id, a, b);

int main() {
	std::cout << create_ddl<Table>();
}