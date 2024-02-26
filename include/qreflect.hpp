#ifndef QREFLECT_QREFLECT_HPP
#define QREFLECT_QREFLECT_HPP

#include <algorithm>
#include <array>
#include <span>
#include <concepts>
#include <source_location>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#define QREFL_ENABLE_ANNOTATIONS() \
	protected: \
		static constexpr size_t qreflect_counter_base = __COUNTER__; \
		template<size_t N> static constexpr auto qreflect_annotation_val = ::qreflect::no_value{}; \
		template<auto MemPtr> static constexpr auto qreflect_annotation = ::qreflect::no_value{}; \
	public: \
		using qreflect_annotation_enabled_tag = void;

#define QREFL_INTERNAL_ANN_END(memptr, idx_) \
	static_assert((idx_ - qreflect_counter_base) % 2 == 0, "QREFL_ANN_BEGIN / QREFL_ANN_END mismatch detected"); \
	template<> static constexpr auto qreflect_annotation<memptr> = qreflect_annotation_val<idx_ - 1>;

#define QREFL_ANN_BEGIN() \
	template<> static constexpr auto qreflect_annotation_val<__COUNTER__>

#define QREFL_ANN_END(memptr) \
	QREFL_INTERNAL_ANN_END(memptr, __COUNTER__)


namespace qreflect {

	struct no_value {};

	template<typename T>
	inline constexpr auto fields_description = no_value {};

	template<typename T>
	inline constexpr auto methods_description = no_value {};


	namespace detail {

		struct convertible_to_any {
			template<typename T>
			operator T();
		};

		template<typename T>
		struct class_of_mem_ptr_helper {};

		template<typename MemT, typename ClassT>
		struct class_of_mem_ptr_helper<MemT ClassT::*> {
			using MemberType = MemT;
			using ClassType = ClassT;
		};

		template<typename T>
		concept aggregate = std::is_aggregate_v<T>;

		template<typename T>
		concept member_object_ptr = std::is_member_object_pointer_v<T>;

		template<typename T>
		concept member_function_ptr = std::is_member_function_pointer_v<T>;

		template<typename T>
		concept member_ptr = std::is_member_pointer_v<T>;

		template<member_ptr T>
		using class_of_member_ptr = typename class_of_mem_ptr_helper<T>::ClassType;

		enum class member_list_type_requirement {
			none,
			field,
			method
		};

		template<typename TupleT>
		struct tagged_member_list {
			TupleT value {};
			member_list_type_requirement require_type = member_list_type_requirement::field;
			bool require_valid = true;
			bool require_complete = false;
			bool require_unique = false;
		};

		template<typename T>
		consteval auto find_field_list() {
			if constexpr(requires{T::qrefl_fields;}) {
				return T::qrefl_fields;
			} else if constexpr(not std::is_same_v<no_value, std::remove_cvref_t<decltype(fields_description<T>)>>) {
				return fields_description<T>;
			} else {
				throw "no field description is available for this type";
			}
		}

		template<typename T>
		consteval auto find_method_list() {
			if constexpr(requires{T::qrefl_methods;}) {
				return T::qrefl_methods;
			} else if constexpr(not std::is_same_v<no_value, std::remove_cvref_t<decltype(methods_description<T>)>>) {
				return methods_description<T>;
			} else {
				throw "no method description is available for this type";
			}
		}

		template<typename T>
		struct field_list {
			static constexpr auto value = find_field_list<T>();
		};

		template<typename T>
		struct method_list {
			static constexpr auto value = find_method_list<T>();
		};

		template<auto V>
		constexpr const char* fn_name() {
			return std::source_location::current().function_name();
		}

		template<typename T>
		constexpr const char* fn_name() {
			return std::source_location::current().function_name();
		}

		consteval std::string_view parse_member_name_gcc_clang(const std::string_view fnsig) {
			size_t begin = fnsig.rfind("::") + 2;
			size_t end = fnsig.find_last_not_of(" })]>");
			return fnsig.substr(begin, 1+end-begin);
		}

		consteval std::string_view parse_type_name_gcc_clang(const std::string_view fnsig) {
			size_t begin = fnsig.find("T = ") + 4;
			size_t dcolon = fnsig.rfind("::");
			if(dcolon > begin && dcolon != std::string_view::npos) {
				begin = dcolon+2;
			}
			size_t end = fnsig.find_last_not_of(" })]>");
			return fnsig.substr(begin, 1+end-begin);
		}

		template<typename T>
		consteval std::string_view type_name() {
			auto funcsig = fn_name<T>();
			return parse_type_name_gcc_clang(funcsig);
		}

		template<size_t N>
		constexpr bool has_no_duplicate_strs(std::array<std::string_view, N> strings) {
			std::sort(strings.begin(), strings.end());
			const auto new_end = std::unique(strings.begin(), strings.end());
			return std::distance(strings.end(), new_end) == 0;
		}

		template<auto MemPtr>
		consteval std::string_view member_name() {
			auto funcsig = fn_name<MemPtr>();
			return parse_member_name_gcc_clang(funcsig);
		}

		template<typename T, size_t... Idxs>
		consteval auto member_names_helper(std::index_sequence<Idxs...>) {
			return std::array {member_name<std::get<Idxs>(T::value)>()...};
		}

		template<typename ListT>
		consteval auto member_names_impl() {
			constexpr size_t len = std::tuple_size_v<decltype(ListT::value)>;
			return member_names_helper<ListT>(std::make_index_sequence<len>{});
		}

		template<typename T>
		consteval auto field_names() {
			return member_names_impl<field_list<T>>();
		}

		template<typename T>
		consteval auto method_names() {
			return member_names_impl<method_list<T>>();
		}



		template<typename T, typename... Ms>
		consteval bool is_member_list_valid(const std::tuple<Ms...>&) {
			return (std::is_same_v<T, class_of_member_ptr<Ms>> && ...);
		}

		template<typename T, typename... Ms>
		consteval bool is_member_list_field_list(const std::tuple<Ms...>&) {
			return (member_object_ptr<Ms> && ...);
		}

		template<typename T, typename... Ms>
		consteval bool is_member_list_method_list(const std::tuple<Ms...>&) {
			return (member_function_ptr<Ms> && ...);
		}

		template<typename T, member_object_ptr... Ms>
		consteval bool is_aggregate_field_list_complete_impl([[maybe_unused]] Ms... mem_ptrs)
		{
			return     requires(T v){T{v.*mem_ptrs...};}
				&& not requires(T v){T{v.*mem_ptrs..., convertible_to_any{}};};
		}

		template<typename T, member_object_ptr... Ms>
		consteval bool is_aggregate_field_list_complete(const std::tuple<Ms...>& mem_ptrs) {
			return std::apply(is_aggregate_field_list_complete_impl<T, Ms...>, mem_ptrs);
		}


		template<typename T, typename ListT>
		struct basic_member_list_info {
			static constexpr auto member_list = ListT::value;
			static constexpr auto member_name_list = member_names_impl<ListT>();

			static constexpr bool is_valid = is_member_list_valid<T>(member_list);
			static constexpr bool is_field_list = is_member_list_field_list<T>(member_list);
			static constexpr bool is_method_list = is_member_list_method_list<T>(member_list);
			static constexpr bool is_complete = is_aggregate_field_list_complete<T>(member_list);
			static constexpr bool has_no_duplicates = has_no_duplicate_strs(member_name_list);
		};

		template<typename T>
		using field_info = basic_member_list_info<T, field_list<T>>;

		template<typename T>
		using method_info = basic_member_list_info<T, method_list<T>>;
	}

	template<typename T>
	concept has_field_info = requires
	{
		/* Description must be valid - it must be an std::tuple of pointers-to-members of T */
		requires detail::field_info<T>::is_valid;

		/* Description must be comprised entirely of pointers to data members (as opposed to methods) */
		requires detail::field_info<T>::is_field_list;

		/* If T is aggregate, the description must be complete, i.e. it must describe ALL of the aggregate's fields */
		requires !std::is_aggregate_v<T> || detail::field_info<T>::is_complete;

		/* If T is aggregate, the description must not have duplicate entries */
		requires !std::is_aggregate_v<T> || detail::field_info<T>::has_no_duplicates;
	};

	template<typename T>
	concept has_method_info = requires
	{
		/* Description must be valid - it must be an std::tuple of pointers-to-members of T */
		requires detail::method_info<T>::is_valid;

		/* Description must be comprised entirely of pointers to methods (as opposed to data members) */
		requires detail::method_info<T>::is_method_list;
	};

	template<typename T>
	inline constexpr auto field_list_v = detail::field_info<T>::member_list;

	template<typename T>
	inline constexpr auto method_list_v = detail::method_info<T>::member_list;

	template<typename T>
	inline constexpr auto field_names_v = detail::field_info<T>::member_name_list;

	template<typename T>
	inline constexpr auto method_names_v = detail::method_info<T>::member_name_list;

	template<typename T>
	inline constexpr auto type_name_v = detail::type_name<T>();

	template<auto V>
		requires detail::member_ptr<decltype(V)>
	inline constexpr auto member_annotation_v = detail::class_of_member_ptr<decltype(V)>::template qreflect_annotation<V>;

	template<typename T>
	inline constexpr auto type_annotation_v = no_value{};

}

#endif //QREFLECT_QREFLECT_HPP
