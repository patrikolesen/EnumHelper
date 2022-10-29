/*
* EnumHelper version 0.1.0
* Licensed under the MIT License <http://opensource.org/licenses/MIT>.
* SPDX-License-Identifier: MIT
* Copyright (c) 2018 - 2022 Patrik Olesen <patrik@hemma.org>
*/

#include <stdio.h>
 
#include <type_traits>
#if __cplusplus>=201703L // C++ 17 code goes here
#include <string_view>
template< bool B, class T = void >
using enable_if_t = std::enable_if_t<B,T>;
#define USE_STRING_VIEW
#define USING_STD_SEQUENCE
#define USE_STRING_VIEW
#define USING_STD_ARRAY

#elif __cplusplus>=201402L // C++ 14 code goes here
template< bool B, class T = void >
using enable_if_t = std::enable_if_t<B,T>;

#elif __cplusplus>=201103L // C++ 11 code goes here
template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B,T>::type;

#elif __cplusplus>199711L
template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B,T>::type;

#else
    #error "Requires C++11 or higher"
#endif

#ifdef USE_STRING_VIEW
#include <string_view>
#endif

#ifdef USING_STD_ARRAY
#include <array>
#endif

/***************
 * MakeSeq
 **************/
#include <cstddef>
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

#ifdef USING_STD_SEQUENCE
    template <size_t... Size>
    using Seq = std::index_sequence<Size...>;
#else
    template <size_t... Size>
    struct Seq
    {
    };
#endif

    template <std::size_t Prepend, typename T>
    struct appender
    {
    };

    template <std::size_t Prepend, std::size_t... Sizes>
    struct appender<Prepend, Seq<Sizes...> >
    {
        using type = Seq<Prepend, Sizes...>;
    };

    template <std::size_t from, std::size_t to>
    struct MakeSeqImpl
    {
        using type = typename appender<from, typename MakeSeqImpl<from + 1, to>::type>::type;
    };

    template <std::size_t to>
    struct MakeSeqImpl<to, to>
    {
        using type = Seq<>;
    };

} // namespace detail

/***************
 * ConstExprArray
 **************/
#ifdef USING_STD_ARRAY
    template <typename T, size_t dim>
    using ConstExprArray = std::array<T, dim>;
#else
    template <typename T, std::size_t dim>
    struct ConstExprArrayStruct
    {
        const T arr[dim];
        constexpr ConstExprArrayStruct() = default;

        constexpr const T& operator[](std::size_t index) const
        {
            return arr[index];
        }

        T const *begin() const
        {
            return arr;
        }
        T const *end() const
        {
            return arr + dim;
        }

        constexpr operator const T*() const {
            return &arr[0];
        }
    };

    template <typename T, size_t dim>
    struct ConstExprArrayStruct_tmp
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

 
template <typename T, std::size_t LL, std::size_t RL, std::size_t... LLs, std::size_t... RLs>
constexpr ConstExprArray<T, LL + RL> join(ConstExprArray<T, LL> lhs, ConstExprArray<T, RL> rhs, detail::Seq<LLs...>, detail::Seq<RLs...>)
{
    return {rhs[LLs]..., lhs[RLs]...};
}

template <typename T, std::size_t LL, std::size_t RL>
constexpr ConstExprArray<T, LL + RL> join(ConstExprArray<T, LL> lhs, ConstExprArray<T, RL> rhs)
{
    return join(rhs, lhs, typename detail::MakeSeqImpl<0, LL>::type(), typename detail::MakeSeqImpl<0, RL>::type());
}


/*********************************
* CSV
**********************************/
constexpr std::size_t findComma(const char *str, std::size_t startOffset = 0)
{
    return (str[startOffset] == ',') ? startOffset : (str[startOffset] == '\0') ? startOffset : findComma(str + 1, startOffset) + 1;
}

constexpr size_t findEnd(const char *str, size_t startOffset = 0)
{
    return (str[startOffset] == ',' || str[startOffset] == '\0') ? startOffset : (str[startOffset] == ' ' || str[startOffset] == '=') ? startOffset : findEnd(str + 1, startOffset) + 1;
}

constexpr size_t trimStart(const char *str, size_t pos)
{
    return (str[pos] != ' ') ? pos : trimStart(str, pos + 1);
}
constexpr size_t trimEnd(const char *str, size_t pos)
{
    return (str[pos] != ' ') ? pos : trimEnd(str, pos - 1);
}

constexpr std::size_t findIndex(int index, const char *str, std::size_t pos = 0)
{
    return (index == 0) ? pos : findIndex(index - 1, str + findComma(str) + 1, findComma(str) + pos + 1);
}

constexpr int nrOfCharsOld(const char *str)
{
    return (*str == '\0') ? 0 : nrOfCharsOld(str + 1) + 1;
}

constexpr size_t findLastIndex(const char *str, size_t startIndex = 0)
{
    return (str[findComma(str, findIndex(startIndex, str))] == '\0') ? startIndex : findLastIndex(str, startIndex+1);
}

constexpr int nrOfChars(const char *str)
{
    return findComma(str,findIndex(findLastIndex(str),str));
}

constexpr size_t max(size_t a, size_t b)
{
    return (a>b)?a:b;
}

constexpr size_t findKeyLength(size_t index, const char* str)
{
    return findEnd(str, findIndex(index ,str)+1)-trimStart(str,findIndex(index,str));    
}

constexpr size_t findMaxLength(const char *str, size_t index)
{
    return (index == 0) ? findEnd(str) : max(findKeyLength(index, str), findMaxLength(str, index-1))+1;
}
constexpr size_t findMaxLength(const char *str)
{
    return findMaxLength(str, findLastIndex(str));
}

constexpr size_t getEnumValueForCurrentIndex(const char* str, size_t value = 0, bool hasValue = false)
{
    return (*str == '\0' || *str == ',') ? (
           value
        ) :
        (*str == '=') ?  getEnumValueForCurrentIndex(str+1, 0, true) :
            (hasValue && *str >= '0' && *str <= '9') ? getEnumValueForCurrentIndex(str+1, value*10 + *str - '0', hasValue) :
                getEnumValueForCurrentIndex(str+1, value, hasValue);
}

constexpr size_t getEnumValueForIndex(const char* str, size_t index)
{
    return (index == 0) ? getEnumValueForCurrentIndex(&str[findIndex(index,str)]) : 
        (getEnumValueForCurrentIndex(&str[findIndex(index,str)]) != 0) ? getEnumValueForCurrentIndex(&str[findIndex(index,str)]) : getEnumValueForIndex(str, index-1) + 1;
}

/*********************************
* Build lookup
**********************************/
template <typename EnumType, std::size_t N>
class MagicValue
{
    public:
    const ConstExprArray<char,N> key;
    size_t value;

    constexpr MagicValue() = delete;
    constexpr MagicValue(const ConstExprArray<char,N> key, size_t value) : key(key), value(value) {};

    constexpr const char* toString() const
    {
        return &key[0];
    }

    constexpr const EnumType getValue() const
    {
        return static_cast<EnumType>(value);
    }

    constexpr size_t getIntValue() const
    {
        return value;
    }

    constexpr bool hasValue() const
    {
        return value != static_cast<size_t>(EnumType::Invalid);
    }

    constexpr ConstExprArray<char,N> getConstArray()  const { return key;}

    constexpr operator size_t() const { return getIntValue();}
    constexpr operator char*()  const { return toString();}
    constexpr operator ConstExprArray<char,N>()  const { return key;}
    constexpr operator EnumType()  const { return getValue();}
};


template<std::size_t dim>
constexpr ConstExprArray<char, dim> createString(const char* csv, const size_t maxLength, const size_t index = 0)
{
    return join(createString<dim/2>(csv, maxLength, index), createString<dim-dim/2>(csv, maxLength, index+dim/2));
}

template<>
constexpr ConstExprArray<char, 1> createString<1>(const char* csv, const size_t maxLength, const size_t index)
{
    return {(index >= maxLength)? '\0':csv[index]};
}

template<typename ElementType, std::size_t N, std::size_t dim, class = enable_if_t<dim == 1>>
constexpr ConstExprArray<ElementType, 1> createLookupTable(const char* csv, const size_t index)
{
    return {ElementType(createString<N>(&csv[trimStart(csv,findIndex(index, csv))], findKeyLength(index, csv)), getEnumValueForIndex(csv, index))};
}

template<typename ElementType, std::size_t N, std::size_t dim,class = enable_if_t<! (dim == 1)> >
constexpr ConstExprArray<ElementType, dim> createLookupTable(const char* csv, const size_t index=0)
{
    return join(createLookupTable<ElementType, N, dim/2>(csv, index), createLookupTable<ElementType, N, dim-dim/2>(csv, index+dim/2));
}


/*********************************
* Magic Enum
**********************************/
template <typename LookupTable, typename T, typename LookupTableElementType>
class MagicEnum
{
    public:
    const LookupTable &lookupTable;
    constexpr MagicEnum(const LookupTable& lookupTable) :  lookupTable(lookupTable) {}

    template <typename EnumClassType>
    constexpr size_t indexOf(const EnumClassType value, size_t index = 0) const
    {
        return (index >= sizeof(lookupTable)/sizeof(lookupTable[0])) ? -1 :
            (static_cast<size_t>(value) == lookupTable[index].value) ? index :
                indexOf<EnumClassType>(value, index+1);
    }

    constexpr size_t indexOf(const char* name, size_t index = 0) const
    {
        return (index >= sizeof(lookupTable)/sizeof(lookupTable[0])) ? -1 :
            (detail::stringsEqual(name, lookupTable[index].toString())) ? index :
                indexOf(name, index+1);
    }

    template <typename EnumClassType>
    constexpr const LookupTableElementType& operator()(const EnumClassType value) const
    {
        return (indexOf(value) == (size_t)-1) ? lookupTable[indexOf("Invalid")] : lookupTable[indexOf(value)];
    }

    constexpr const LookupTableElementType& operator()(const size_t value) const
    {
        return (indexOf(value) == (size_t)-1) ? lookupTable[indexOf("Invalid")] : lookupTable[indexOf(value)];
    }

    constexpr const LookupTableElementType& operator()(const char* name) const
    {
        return (indexOf(name) == (size_t)-1) ? lookupTable[indexOf("Invalid")] : lookupTable[indexOf(name)];
    }

    class const_iterator 
    {
        public:
        friend class MagicEnum;
        const LookupTableElementType* m_element{};
        size_t m_index = 0;
        T m_value{};

        const_iterator (const LookupTableElementType* element) : m_element(element) {}

        const LookupTableElementType& operator * ()  {
            return m_element[m_index];
        }

        const_iterator& operator++ ()
        {   
            m_index++;
            return *this;
        }
        
        const_iterator& operator-- ()
        {   
            m_index--;
            return *this;
        }

        bool operator != (const const_iterator& rhs) const
        {
            return &m_element[m_index] != &rhs.m_element[rhs.m_index];
        }

        const_iterator () : m_element(0) {}
    };
    
    const_iterator  begin() const { return const_iterator(lookupTable.begin()); }
    /* Remove the last Invalid Enum */
    const_iterator  end() const { return const_iterator(lookupTable.end() - 1); }
};
} //EnumHelper

#define EnumHelper2(ClassName, ...) \
    enum class ClassName : size_t  \
    {                              \
        __VA_ARGS__                \
    };                             \
    constexpr static auto *ClassName##Str = static_cast<const char *>(#__VA_ARGS__); \
    constexpr static auto ClassName##LookupTable = EnumHelper::createLookupTable<EnumHelper::MagicValue<ClassName, EnumHelper::findMaxLength(ClassName##Str)>, EnumHelper::findMaxLength(ClassName##Str), EnumHelper::findLastIndex(ClassName##Str)+1>(ClassName##Str); \
    constexpr static auto ClassName##MagicEnum = EnumHelper::MagicEnum<decltype(ClassName##LookupTable), ClassName, EnumHelper::MagicValue<ClassName, EnumHelper::findMaxLength(ClassName##Str)>>(ClassName##LookupTable); \
    struct ClassName##MagicValue : public EnumHelper::MagicValue<ClassName, EnumHelper::findMaxLength(ClassName##Str)>{        \
        constexpr ClassName##MagicValue(const Color &value) : EnumHelper::MagicValue<ClassName, EnumHelper::findMaxLength(ClassName##Str)>(ClassName##MagicEnum(value)) { } \
        constexpr ClassName##MagicValue(const char* value) : EnumHelper::MagicValue<ClassName, EnumHelper::findMaxLength(ClassName##Str)>(ClassName##MagicEnum(value)) { } \
        constexpr ClassName##MagicValue(const size_t value) : EnumHelper::MagicValue<ClassName, EnumHelper::findMaxLength(ClassName##Str)>(ClassName##MagicEnum(value)) { } \
    };
#define EnumHelper(...) EnumHelper2(__VA_ARGS__, Invalid)

namespace bitset
{
    template <typename EnumClass>
    class EnumFlags {

        public:
            typename std::underlying_type<EnumClass>::type flags{};
            EnumFlags() = default;

            EnumFlags(EnumClass flag)
            {
                flags = static_cast<typename std::underlying_type<EnumClass>::type>(flag);
            }
    };

    template < typename EnumClass >
    constexpr EnumFlags<EnumClass> operator | (EnumClass l, EnumClass r) noexcept {
        return EnumFlags<EnumClass>{l} | EnumFlags<EnumClass>{r};
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator | (Enum l, EnumFlags<Enum> r) noexcept {
        return EnumFlags<Enum>{l} | r;
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator | (EnumFlags<Enum> l, Enum r) noexcept {
        return l | EnumFlags<Enum>{r};
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator | (EnumFlags<Enum> l, EnumFlags<Enum> r) noexcept {
        return static_cast<Enum>(l.flags | r.flags);
    }
    template < typename Enum >
    constexpr EnumFlags<Enum>& operator |= (EnumFlags<Enum>& l, Enum r) noexcept {
        return l = l | EnumFlags<Enum>{r};
    }
    template < typename Enum >
        constexpr EnumFlags<Enum>& operator |= (EnumFlags<Enum>& l, EnumFlags<Enum> r) noexcept {
        return l = l | r;
    }

    template < typename Enum >
    constexpr EnumFlags<Enum> operator & (Enum l, Enum r) noexcept {
        return EnumFlags<Enum>{l} & EnumFlags<Enum>{r};
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator & (Enum l, EnumFlags<Enum> r) noexcept {
        return EnumFlags<Enum>{l} & r;
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator & (EnumFlags<Enum> l, Enum r) noexcept {
        return l & EnumFlags<Enum>{r};
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator & (EnumFlags<Enum> l, EnumFlags<Enum> r) noexcept {
        return static_cast<Enum>(l.flags & r.flags);
    }
    template < typename Enum >
    constexpr EnumFlags<Enum>& operator &= (EnumFlags<Enum>& l, Enum r) noexcept {
        return l = l & EnumFlags<Enum>{r};
    }
    template < typename Enum >
        constexpr EnumFlags<Enum>& operator &= (EnumFlags<Enum>& l, EnumFlags<Enum> r) noexcept {
        return l = l & r;
    }


    template < typename Enum >
    constexpr EnumFlags<Enum> operator ^ (Enum l, Enum r) noexcept {
        return EnumFlags<Enum>{l} ^ EnumFlags<Enum>{r};
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator ^ (Enum l, EnumFlags<Enum> r) noexcept {
        return EnumFlags<Enum>{l} ^ r;
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator ^ (EnumFlags<Enum> l, Enum r) noexcept {
        return l ^ EnumFlags<Enum>{r};
    }
    template < typename Enum >
    constexpr EnumFlags<Enum> operator ^ (EnumFlags<Enum> l, EnumFlags<Enum> r) noexcept {
        return static_cast<Enum>(l.flags ^ r.flags);
    }
    template < typename Enum >
    constexpr EnumFlags<Enum>& operator ^= (EnumFlags<Enum>& l, Enum r) noexcept {
        return l = l ^ EnumFlags<Enum>{r};
    }
    template < typename Enum >
        constexpr EnumFlags<Enum>& operator ^= (EnumFlags<Enum>& l, EnumFlags<Enum> r) noexcept {
        return l = l ^ r;
    }
}