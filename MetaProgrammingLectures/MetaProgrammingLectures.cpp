#include <iostream>
#include <array>
#include <vector>
#include <variant>

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

template<class T, class... Us>
constexpr bool is_initializable_v{
    requires (Us&&... us) { T{std::forward<Us>(us)...}; }
};

static_assert(is_initializable_v<int, int>);
static_assert(!is_initializable_v<double, int>);

template<class T>
class wrapper
{
public:
    template<class... Us>
        requires is_initializable_v<T, Us...> && ((sizeof...(Us) != 1) || (!resolves_to_copy_v<Us, wrapper<T>> && ...))
    explicit(sizeof...(Us) == 1) wrapper(Us&&... t) : m_Value{std::forward<Us>(t)...}
    {
        std::cout << "Constructor\n";
    }

private:
    T m_Value;
};

struct platypus
{
    void walk() { std::cout << "I am a walking platypus!\n"; }

    void swim() { std::cout << "I am a swimming platypus!\n"; }
};

struct penguin
{
    void walk() { std::cout << "I am a walking penguin!\n"; }

    void swim() { std::cout << "I am a swimming penguin!\n"; }
};

struct bee
{
    void walk() { std::cout << "I am a walking bee!\n"; }

    void fly() { std::cout << "I am a flying bee!\n"; }
};

struct bat
{
    void walk() { std::cout << "I am a walking bat!\n"; }

    void fly() { std::cout << "I am a flying bat!\n"; }
};

struct duck
{
    void swim() { std::cout << "I am a swimming duck!\n"; }

    void walk() { std::cout << "I am a walking duck!\n"; }

    void fly() { std::cout << "I am a flying duck!\n"; }
};

template<class T>
constexpr bool can_swim{requires(T & t) { t.swim(); }};

template<class T>
constexpr bool can_fly{requires(T & t) { t.fly(); }};

static_assert(can_swim<platypus>);
static_assert(!can_swim<bee>);

template<class T>
T&& declval();

template<class...>
using void_t = void;

template<class T, class U=void>
struct can_swim2 : std::false_type {};

template<class T>
struct can_swim2<T, void_t<decltype(declval<T>().swim())>> : std::true_type{};

template<class T>
using can_swim2_t = typename can_swim2<T>::type;

template<class T>
constexpr bool can_swim2_v{can_swim2<T>::value};

static_assert(can_swim2_v<platypus>);
static_assert(!can_swim2_v<bee>);

struct special_power
{
    template<class T>
        requires (!can_fly<T>) && can_swim<T>
    void operator()(T& t) { t.swim(); }

    template<class T>
        requires can_fly<T> && (!can_swim<T>)
    void operator()(T& t) { t.fly(); }

    template<class T>
        requires can_fly<T> && can_swim<T>
    void operator()(T& t) { t.fly(); t.swim(); }
};

template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

int main()
{
    using animal_variants = std::variant<platypus, bee, penguin, bat>;
    using animal_container = std::vector<animal_variants>;

    animal_container animals{platypus{}, bee{}, penguin{}, bat{}};

    for(auto& a : animals)
    {
        std::visit([](auto& e){ e.walk(); }, a);
    }

    for(auto& a : animals)
    {
        overloaded visitor{
            []<class T> requires can_swim<T> (T& t){ t.swim(); },
            []<class T> requires can_fly<T>  (T& t){ t.fly(); }
        };

        std::visit(visitor, a);
    }
}