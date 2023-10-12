#ifndef QREFLECT_QREFLECT_HPP
#define QREFLECT_QREFLECT_HPP

#include <concepts>
#include <string_view>
#include <utility>

#ifndef QREFL_MAX_ARITY
#define QREFL_MAX_ARITY 32
#endif

#define QREFL_STATIC_ASSERT_FALSE(T, msg) \
	static_assert(requires(T x){{x.fq2358798c7f78q26hfc897hr6h6};}, msg)

#define QREFL_PARENS ()

#define QREFL_EXPAND(...) QREFL_EXPANDMAX(__VA_ARGS__)

#if QREFL_MAX_ARITY > 512
#error "QREFL_MAX_ARITY cannot exceed 512"
#elif QREFL_MAX_ARITY > 1+256
#define QREFL_EXPANDMAX QREFL_EXPAND8
#elif QREFL_MAX_ARITY > 1+128
#define QREFL_EXPANDMAX QREFL_EXPAND7
#elif QREFL_MAX_ARITY > 1+64
#define QREFL_EXPANDMAX QREFL_EXPAND6
#elif QREFL_MAX_ARITY > 1+32
#define QREFL_EXPANDMAX QREFL_EXPAND5
#elif QREFL_MAX_ARITY > 1+16
#define QREFL_EXPANDMAX QREFL_EXPAND4
#elif QREFL_MAX_ARITY > 1+8
#define QREFL_EXPANDMAX QREFL_EXPAND3
#elif QREFL_MAX_ARITY > 1+4
#define QREFL_EXPANDMAX QREFL_EXPAND2
#elif QREFL_MAX_ARITY > 1+2
#define QREFL_EXPANDMAX QREFL_EXPAND1
#elif QREFL_MAX_ARITY == 1+2
#define QREFL_EXPANDMAX QREFL_EXPAND0
#else
#error "QREFL_MAX_ARITY must be at least 3"
#endif


#define QREFL_EXPAND8(...) QREFL_EXPAND7(QREFL_EXPAND7(__VA_ARGS__))
#define QREFL_EXPAND7(...) QREFL_EXPAND6(QREFL_EXPAND6(__VA_ARGS__))
#define QREFL_EXPAND6(...) QREFL_EXPAND5(QREFL_EXPAND5(__VA_ARGS__))
#define QREFL_EXPAND5(...) QREFL_EXPAND4(QREFL_EXPAND4(__VA_ARGS__))
#define QREFL_EXPAND4(...) QREFL_EXPAND3(QREFL_EXPAND3(__VA_ARGS__))
#define QREFL_EXPAND3(...) QREFL_EXPAND2(QREFL_EXPAND2(__VA_ARGS__))
#define QREFL_EXPAND2(...) QREFL_EXPAND1(QREFL_EXPAND1(__VA_ARGS__))
#define QREFL_EXPAND1(...) QREFL_EXPAND0(QREFL_EXPAND0(__VA_ARGS__))
#define QREFL_EXPAND0(...) __VA_ARGS__

#define QREFL_FOR_EACH(macro, type_name, counter, ...)                                    \
	__VA_OPT__(QREFL_EXPAND(QREFL_FOR_EACH_HELPER(macro, type_name, counter, __VA_ARGS__)))
#define QREFL_FOR_EACH_HELPER(macro, type_name, counter, a1, ...)                         \
	macro(type_name, counter, a1)                                                     \
	__VA_OPT__(QREFL_FOR_EACH_AGAIN QREFL_PARENS (macro, type_name, (counter+1), __VA_ARGS__))
#define QREFL_FOR_EACH_AGAIN() QREFL_FOR_EACH_HELPER

#define QREFL_CHECK_NUM_OF_FIELDS(type_name, ...) \
	namespace qreflect {                             \
        template<> \
		void fail_compilation_if_num_of_fields_does_not_match(type_name&& qrefl_val) { \
        	const auto& [__VA_ARGS__] = qrefl_val; \
		} \
	} \

#define QREFL_DESCRIBE_FIELD(type_name, field_idx, field_name) \
	namespace qreflect { template<> struct field_info<type_name, field_idx> { \
        using parent_type = type_name; \
    	using type = std::remove_cvref_t<decltype(type_name{}.field_name)>;   \
        static inline constexpr int index = field_idx; \
        static inline constexpr std::string_view name = #field_name; \
		static inline const auto ptr_to_member = &type_name::field_name; \
	};};

#define QREFL_COUNT_ARGS_HELPER(dummy1, dummy2, dummy3) \
	argcount++;

#define QREFL_COUNT_ARGS(...) []{ \
	int argcount = 0;                \
    QREFL_FOR_EACH(QREFL_COUNT_ARGS_HELPER, 0, 0, __VA_ARGS__) \
	return argcount; \
}(); \


#define QREFL_DESCRIBE_PARTIAL(type_name, ...) \
	QREFL_FOR_EACH(QREFL_DESCRIBE_FIELD, type_name, 0, __VA_ARGS__) \
	namespace qreflect {                          \
        template<> constexpr inline bool is_inspectable_v<type_name> = true; \
		template<> struct type_info<type_name> { \
			using type = type_name; \
			static inline constexpr int num_fields = QREFL_COUNT_ARGS(__VA_ARGS__) \
			static inline constexpr std::string_view name = #type_name; \
		};                                            \
	};

#define QREFL_DESCRIBE(type_name, ...) \
    QREFL_DESCRIBE_PARTIAL(type_name, __VA_ARGS__) \
    QREFL_CHECK_NUM_OF_FIELDS(type_name, __VA_ARGS__) \
	static_assert(std::is_aggregate_v<type_name>, "Cannot fully reflect non-aggregate types");



namespace qreflect {

	template<typename T, int N>
	struct field_info {

	};

	template<typename T>
	struct type_info {

	};

	template<typename TVal, typename TValAgain>
	struct has_correct_num_fields: public std::false_type {
		void dummy(TValAgain&& v) {}
	};

	template<typename T>
	void fail_compilation_if_num_of_fields_does_not_match(T&&) {

	}

	template<typename T>
	inline constexpr bool has_correct_num_fields_v = has_correct_num_fields<T, T>::value;

	template<typename T>
	concept field_info_type = requires {
		typename T::parent_type;
		typename T::type;
		{T::index} -> std::convertible_to<int>;
		{T::name} -> std::convertible_to<std::string_view>;
		{T::ptr_to_member};
	};

	template<typename T>
	concept type_info_type = requires {
		typename T::type;
		{T::num_fields} -> std::convertible_to<int>;
		{T::name} -> std::convertible_to<std::string_view>;
	};

	template<typename T>
	constexpr inline bool is_inspectable_v = false;

	template<typename T>
	concept inspectable = is_inspectable_v<std::remove_cvref_t<T>>;

	template<typename TData, typename TFn>
		requires inspectable<std::remove_cvref_t<TData>>
	void for_each_member_decl(TFn&& fn) {
		using TDataV = std::remove_cvref_t<TData>;
		[&]<size_t... Idxs>(std::index_sequence<Idxs...> seq) {
			((fn.template operator()<field_info<TDataV, Idxs>>()), ...);
		}(std::make_index_sequence<type_info<TDataV>::num_fields>{});
	}

	template<typename TData, typename TFn>
		requires inspectable<std::remove_cvref_t<TData>>
	void for_each_member_of(TData&& value, TFn&& fn) {
		for_each_member_decl<TData>([&]<field_info_type FieldInfoT>(){
			fn(FieldInfoT::name, value.*(FieldInfoT::ptr_to_member));
		});
	}


}

#endif //QREFLECT_QREFLECT_HPP
