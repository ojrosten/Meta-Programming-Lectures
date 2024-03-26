#include <iostream>
#include <array>
#include <vector>

template<class T, T val>
struct integral_constant
{
    using value_type = T;
    constexpr static T value{val};

    using type = integral_constant<T, val>;
};

template<bool B>
using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template<class T>
struct is_floating_point : false_type {};

template<>
struct is_floating_point<float> : true_type {};

template<>
struct is_floating_point<double> : true_type {};

template<class T>
constexpr bool is_floating_point_v{is_floating_point<T>::value};

static_assert(!is_floating_point_v<int>);
static_assert(is_floating_point_v<float>);
static_assert(is_floating_point_v<double>);

static_assert(std::is_same_v<typename is_floating_point<float>::value_type, bool>);
static_assert(std::is_same_v<is_floating_point<float>::type, true_type>);


template<class T>
   requires is_floating_point_v<T>
class radians
{
public:
    //static_assert(is_floating_point_v<T>, "Type must be floating-point");

    radians() = default;

    explicit radians(T angle) : m_Angle{angle} {}
private:
    T m_Angle{};
};

template<class U, class Wrapper>
constexpr bool resolves_to_copy_v{ std::is_same_v<std::remove_cvref_t<U>, Wrapper> };

template<bool B, class T=void>
struct enable_if {};

template<class T>
struct enable_if<true, T>
{
    using type = T;
};

template<bool B, class T= void>
using enable_if_t = typename enable_if<B, T>::type;

template<class T>
class wrapper
{
public:
    // SFINAE
    // Substitution Failure is NOT an error
    template<class U, class = enable_if_t<!resolves_to_copy_v<U, wrapper<T>> && !std::is_arithmetic_v<U>> >
        //requires (!resolves_to_copy_v<U, wrapper<T>>)
    wrapper(U&& t) : m_Value{std::forward<U>(t)}
    {
        std::cout << "Constructor\n";
    }

    template<class U, enable_if_t<std::is_integral_v<U>, int> = 42 >
    wrapper(U u) : m_Value{u}
    {
        std::cout << "Integral Constructor\n";
    }

    template<class U, enable_if_t<std::is_floating_point_v<U>, float> = 3.14f>
    wrapper(U u) : m_Value{std::isnan(u) ? throw 1 : u}
    {
        std::cout << "Floating-point Constructor\n";
    }



    wrapper(const wrapper& other)
        : m_Value{other.m_Value}
    {
        std::cout << "Copy constructor\n";
    }
private:
    T m_Value;
};

int main()
{
    wrapper<double> w{1.0}, x{w};
}