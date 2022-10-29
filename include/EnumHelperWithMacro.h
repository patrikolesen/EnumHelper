/*
* EnumHelper version 0.1.0
* Licensed under the MIT License <http://opensource.org/licenses/MIT>.
* SPDX-License-Identifier: MIT
* Copyright (c) 2018 - 2022 Patrik Olesen <patrik@hemma.org>
*/

#include <cstddef>
#include <type_traits>
#include <iterator>
#include <utility>
#include <algorithm>

#if __cplusplus >= 201703L // C++ 17 code goes here
#define USING_STD_SEQUENCE
#define USE_STRING_VIEW
#define USING_STD_ARRAY

#ifdef USE_STRING_VIEW
#include <string_view>
#endif

#ifdef USING_STD_ARRAY
#include <array>
#endif

#endif

namespace EnumHelper
{
namespace detail
{
#ifdef USE_STRING_VIEW
    constexpr bool stringsEqual(char const *a, char const *b)
    {
        return std::string_view(a) == std::string_view(b);
    }
#else
    inline constexpr bool stringsEqual(char const *a, char const *b)
    {
        return *a == *b && (*b == '\0' || *a == '\0' || stringsEqual(a + 1, b + 1));
    }
#endif

template<typename T>
constexpr T max(T val)
{
    return val;
}

template<typename T, class...Args>
constexpr T max(T val, Args...args)
{
    return (val > max(args...)) ? val : max(args...);
}
/***************
 * KeyNameRetriever
 **************/

/***************
 * MakeSeq
 **************/
#ifdef USING_STD_SEQUENCE
    template <size_t... Size>
    using Seq = std::index_sequence<Size...>;
#else
    template <size_t... Size>
    struct Seq
    {
    };
#endif
    template <size_t Prepend, typename T>
    struct appender
    {
    };

    template <size_t Prepend, size_t... Sizes>
    struct appender<Prepend, Seq<Sizes...>>
    {
        using type = Seq<Prepend, Sizes...>;
    };

    template <size_t from, size_t to>
    struct MakeSeqImpl
    {
        using type = typename appender<from, typename MakeSeqImpl<from + 1, to>::type>::type;
    };

    template <size_t to>
    struct MakeSeqImpl<to, to>
    {
        using type = Seq<>;
    };

    template <size_t to>
    struct MakeSeqToImpl
    {
        using type = typename MakeSeqImpl<0, to>::type;
    };

/***************
 * ConstExprArray
 **************/
#ifdef USING_STD_ARRAY
    template <typename T, size_t dim>
    using ConstExprArray = std::array<T, dim>;
#else
    template <typename T, size_t dim>
    struct ConstExprArrayStruct
    {
        const T arr[dim];
        constexpr const T &operator[](size_t index) const
        {
            return arr[index];
        }
    };
    template <typename T, size_t dim>
    using ConstExprArray = ConstExprArrayStruct<T, dim>;
#endif

    template <typename T, size_t LL, size_t RL, size_t... LLs, size_t... RLs>
    constexpr ConstExprArray<T, LL + RL> join(ConstExprArray<T, LL> lhs, ConstExprArray<T, RL> rhs, detail::Seq<LLs...>, detail::Seq<RLs...>)
    {
        return {rhs[LLs]..., lhs[RLs]...};
    }

    template <typename T, size_t LL, size_t RL>
    constexpr ConstExprArray<T, LL + RL> join(ConstExprArray<T, LL> lhs, ConstExprArray<T, RL> rhs)
    {
        return join(rhs, lhs, typename detail::MakeSeqImpl<0, LL>::type(), typename detail::MakeSeqImpl<0, RL>::type());
    }

    template <size_t dim>
    constexpr ConstExprArray<const char, dim> createString(const char *csv, const size_t maxLength, const size_t index = 0)
    {
        return join(createString<dim / 2>(csv, maxLength, index), createString<dim - dim / 2>(csv, maxLength, index + dim / 2));
    }

    template <>
    constexpr ConstExprArray<const char, 1> createString<1>(const char *csv, const size_t maxLength, const size_t index)
    {
        return {(index >= maxLength) ? '\0' : csv[index]};
    }

    /***************
    * CSV, Used for finding max key length
    **************/
    constexpr std::size_t findComma(const char *str, std::size_t startOffset = 0)
    {
        return (str[startOffset] == ',') ? startOffset : (str[startOffset] == '\0') ? startOffset
                                                                                    : findComma(str + 1, startOffset) + 1;
    }

    constexpr size_t findEnd(const char *str, size_t startOffset = 0)
    {
        return (str[startOffset] == ',' || str[startOffset] == '\0') ? startOffset : (str[startOffset] == ' ' || str[startOffset] == '=') ? startOffset
                                                                                                                                          : findEnd(str + 1, startOffset) + 1;
    }

    constexpr size_t trimStart(const char *str, size_t pos)
    {
        return (str[pos] != ' ') ? pos : trimStart(str, pos + 1);
    }

    constexpr std::size_t findIndex(int index, const char *str, std::size_t pos = 0)
    {
        return (index == 0) ? pos : findIndex(index - 1, str + findComma(str) + 1, findComma(str) + pos + 1);
    }

    constexpr size_t findLastIndex(const char *str, size_t startIndex = 0)
    {
        return (str[findComma(str, findIndex(startIndex, str))] == '\0') ? startIndex : findLastIndex(str, startIndex + 1);
    }

    constexpr size_t findKeyLength(const char *str)
    {
        return findEnd(str, 0) - trimStart(str, 0);
    }

    constexpr size_t findKeyLength(size_t index, const char *str)
    {
        return findEnd(str, findIndex(index, str) + 1) - trimStart(str, findIndex(index, str));
    }
    constexpr size_t max(size_t a, size_t b)
    {
        return (a > b) ? a : b;
    }

    constexpr size_t findMaxLength(const char *str, size_t index)
    {
        return (index == 0) ? findEnd(str) : max(findKeyLength(index, str), findMaxLength(str, index - 1));
    }

    constexpr size_t findMaxLength(const char *str)
    {
        return findMaxLength(str, findLastIndex(str));
    }

    /***************
    * ValueRetreiver
    **************/
    template <typename Enum>
    struct ignoreAssignment final
    {
        Enum value;

        constexpr explicit ignoreAssignment(Enum value) noexcept
            : value(value) {}

        template <typename Other>
        constexpr const ignoreAssignment &operator=(Other) const noexcept
        {
            return *this;
        }
    };
}

/***************
 * EnumBuilder
 **************/
template <typename EnumType, size_t N>
class EnumPair
{
public:
    const char name[N];
    EnumType value;
    constexpr const char *toString() const
    {
        return name;
    }
    constexpr size_t getIntValue() const
    {
        return static_cast<size_t>(value);
    }
};

template <typename EnumType, size_t N, size_t... Idx>
constexpr EnumPair<EnumType, N> toEnumPair(const detail::ConstExprArray<const char, N> &arr, const EnumType &value, detail::Seq<Idx...>)
{
    return {arr[Idx]..., value};
}

template <typename EnumType, size_t N>
constexpr EnumPair<EnumType, N> toEnumPair(const detail::ConstExprArray<const char, N> &arr, const EnumType &value)
{
    return toEnumPair(arr, value, typename detail::MakeSeqToImpl<N>::type());
}

template <typename EnumType, size_t N>
constexpr EnumPair<EnumType, N> toEnumPair(const char *arr, size_t maxLen, const EnumType &value)
{
    return toEnumPair(detail::createString<N>(arr, maxLen), value);
}

template <typename LookupTable, typename EnumPairType>
class MagicEnum
{
    const LookupTable &lookupTable;

public:
    constexpr MagicEnum(const LookupTable &lookupTable) : lookupTable(lookupTable) {}

    template <typename EnumType>
    constexpr size_t indexOf(const EnumType &value, size_t index = 0) const
    {
        return (index >= sizeof(lookupTable) / sizeof(lookupTable[0])) ? (size_t)-1 : (value == lookupTable[index].value) ? index
                                                                                                                          : indexOf<EnumType>(value, index + 1);
    }

    constexpr size_t indexOf(const char *name, size_t index = 0) const
    {
        return (index >= sizeof(lookupTable) / sizeof(lookupTable[0])) ? (size_t)(-1) : (detail::stringsEqual(name, lookupTable[index].name)) ? index
                                                                                                                                              : indexOf(name, index + 1);
    }

    template <typename EnumType>
    constexpr const EnumPairType &operator()(const EnumType value) const
    {
        return (indexOf(value) == (size_t)(-1)) ? lookupTable[indexOf(EnumType::Invalid)] : lookupTable[indexOf(value)];
    }

    constexpr const EnumPairType &operator()(const size_t value) const
    {
        return (indexOf(static_cast<decltype(EnumPairType::value)>(value)) == (size_t)(-1)) ? lookupTable[indexOf("Invalid")] : lookupTable[indexOf(static_cast<decltype(EnumPairType::value)>(value))];
    }

    constexpr const EnumPairType &operator()(const char *name) const
    {
        return (indexOf(name) == (size_t)(-1)) ? lookupTable[indexOf("Invalid")] : lookupTable[indexOf(name)];
    }

    constexpr const EnumPairType &at(const size_t index) const
    {
        return lookupTable[index];
    }

    constexpr const EnumPairType *begin() const { return std::begin(lookupTable); }
    /* Remove the last elemnt */
    constexpr const EnumPairType *end() const { return std::end(lookupTable) - 1; }
};

/***************
 * Enum flag handling
 **************/
namespace enumflags
{
    template <typename EnumClass>
    class EnumFlags
    {
    public:
        typename std::underlying_type<EnumClass>::type flags{};

        EnumFlags(EnumClass flag) : flags(static_cast<typename std::underlying_type<EnumClass>::type>(flag))
        {
        }
    };
    template <typename EnumClass>
    constexpr EnumFlags<EnumClass> operator|(EnumClass l, EnumClass r) noexcept
    {
        return EnumFlags<EnumClass>{l} | EnumFlags<EnumClass>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator|(Enum l, EnumFlags<Enum> r) noexcept
    {
        return EnumFlags<Enum>{l} | r;
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator|(EnumFlags<Enum> l, Enum r) noexcept
    {
        return l | EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator|(EnumFlags<Enum> l, EnumFlags<Enum> r) noexcept
    {
        return static_cast<Enum>(l.flags | r.flags);
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> &operator|=(EnumFlags<Enum> &l, Enum r) noexcept
    {
        return l = l | EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> &operator|=(EnumFlags<Enum> &l, EnumFlags<Enum> r) noexcept
    {
        return l = l | r;
    }

    template <typename Enum>
    constexpr EnumFlags<Enum> operator&(Enum l, Enum r) noexcept
    {
        return EnumFlags<Enum>{l} & EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator&(Enum l, EnumFlags<Enum> r) noexcept
    {
        return EnumFlags<Enum>{l} & r;
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator&(EnumFlags<Enum> l, Enum r) noexcept
    {
        return l & EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator&(EnumFlags<Enum> l, EnumFlags<Enum> r) noexcept
    {
        return static_cast<Enum>(l.flags & r.flags);
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> &operator&=(EnumFlags<Enum> &l, Enum r) noexcept
    {
        return l = l & EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> &operator&=(EnumFlags<Enum> &l, EnumFlags<Enum> r) noexcept
    {
        return l = l & r;
    }

    template <typename Enum>
    constexpr EnumFlags<Enum> operator^(Enum l, Enum r) noexcept
    {
        return EnumFlags<Enum>{l} ^ EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator^(Enum l, EnumFlags<Enum> r) noexcept
    {
        return EnumFlags<Enum>{l} ^ r;
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator^(EnumFlags<Enum> l, Enum r) noexcept
    {
        return l ^ EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> operator^(EnumFlags<Enum> l, EnumFlags<Enum> r) noexcept
    {
        return static_cast<Enum>(l.flags ^ r.flags);
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> &operator^=(EnumFlags<Enum> &l, Enum r) noexcept
    {
        return l = l ^ EnumFlags<Enum>{r};
    }
    template <typename Enum>
    constexpr EnumFlags<Enum> &operator^=(EnumFlags<Enum> &l, EnumFlags<Enum> r) noexcept
    {
        return l = l ^ r;
    }
}
}

/***************
 * MacroMagic
 **************/
// Use g++ -E a.cpp to only check pre parser
#define EMPTY()
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0, )
#define PROBE(x) x, 1,

#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 PROBE(~)
#define NOT_ PROBE(~)

#define BOOL(x) COMPL(NOT(x))

#define EVAL(...) EVAL1024(__VA_ARGS__)
#define EVAL1024(...) EVAL256(EVAL256(EVAL256(EVAL256(__VA_ARGS__))))
#define EVAL256(...) EVAL64(EVAL64(EVAL64(EVAL64(__VA_ARGS__))))
#define EVAL64(...) EVAL16(EVAL16(EVAL16(EVAL16(__VA_ARGS__))))
#define EVAL16(...) EVAL4(EVAL4(EVAL4(EVAL4(__VA_ARGS__))))
#define EVAL4(...) EVAL1(EVAL1(EVAL1(EVAL1(__VA_ARGS__))))
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) __VA_ARGS__

#define DEFER2(m) m EMPTY EMPTY()()

#define IF_ELSE(condition) _IF_ELSE(BOOL(condition))
#define _IF_ELSE(condition) CAT(_IF_, condition)

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...) _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__

#define FIRST(a, ...) a

#define HAS_ARGS(...) BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)())
#define _END_OF_ARGUMENTS_() 0

#define MAP(m, first, ...)                                                   \
    m(first)                                                                 \
        IF_ELSE(HAS_ARGS(__VA_ARGS__))(                                      \
            DEFER2(_MAP)()(m, __VA_ARGS__))(/* Do nothing, just terminate */ \
        )
#define _MAP() MAP

#define MAP2(m, data1, data2, first, ...)                                                   \
    m(data1, data2, first)                                                                  \
        IF_ELSE(HAS_ARGS(__VA_ARGS__))(                                                     \
            DEFER2(_MAP2)()(m, data1, data2, __VA_ARGS__))(/* Do nothing, just terminate */ \
        )
#define _MAP2() MAP2

#define ENUM_HELPER_(EnumType, ...)                                                                                                                                                        \
    enum class EnumType                                                                                                                                                                    \
    {                                                                                                                                                                                      \
        __VA_ARGS__                                                                                                                                                                        \
    };                                                                                                                                                                                     \
    static constexpr size_t EnumType##MaxKeyLength = EnumHelper::detail::findMaxLength(#__VA_ARGS__) + 1;                                                                                  \
    static constexpr const EnumHelper::EnumPair<EnumType, EnumType##MaxKeyLength> EnumType##Map[] = {EVAL(MAP2(ENUM_HELPER_PAIR_CREATOR, EnumType, EnumType##MaxKeyLength, __VA_ARGS__))}; \
    static constexpr auto EnumType##MagicEnum = EnumHelper::MagicEnum<decltype(EnumType##Map), EnumHelper::EnumPair<EnumType, EnumType##MaxKeyLength>>(EnumType##Map);                     \
    struct EnumType##MagicValue : public EnumHelper::EnumPair<EnumType, EnumType##MaxKeyLength>                                                                                            \
    {                                                                                                                                                                                      \
        constexpr EnumType##MagicValue(const EnumType &value) : EnumHelper::EnumPair<EnumType, EnumType##MaxKeyLength>(EnumType##MagicEnum(value)) {}                                      \
        constexpr EnumType##MagicValue(const char *name) : EnumHelper::EnumPair<EnumType, EnumType##MaxKeyLength>(EnumType##MagicEnum(name)) {}                                            \
        constexpr EnumType##MagicValue(const size_t value) : EnumHelper::EnumPair<EnumType, EnumType##MaxKeyLength>(EnumType##MagicEnum(value)) {}                                         \
    };

#define EnumHelper(...) ENUM_HELPER_(__VA_ARGS__, Invalid)

#define ENUM_HELPER_PAIR_CREATOR(Enum, N, x) \
    EnumHelper::toEnumPair<Enum, N>((const char *)#x, EnumHelper::detail::findKeyLength(#x), ((EnumHelper::detail::ignoreAssignment<Enum>)Enum::x).value),

#define ENUM_HELPER_KEY_LENGHT(x) \
    EnumHelper::detail::findKeyLength(#x),
