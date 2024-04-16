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

struct vertex_shader {
    void update() { std::cout << "Vertex shader update\n"; }
};

struct geometry_shader {
    void update() { std::cout << "Geometry shader update\n"; }
};

struct fragment_shader {
    void update() { std::cout << "Fragment shader update\n"; }
};

template<class T>
constexpr bool has_update_v{ requires(T& t) { t.update(); } };

static_assert(has_update_v<vertex_shader>);
static_assert(!has_update_v<int>);

template<class... Ts>
    requires (has_update_v<Ts> && ...)
using shader_program = std::tuple<Ts...>;

template<std::size_t... Is>
struct index_sequence{};

template<class T, class U>
struct concat_sequences;

template<std::size_t... Is, std::size_t... Js>
struct concat_sequences<index_sequence<Is...>, index_sequence<Js...>>
{
    using type = index_sequence<Is..., Js...>;
};

template<class T, class U>
using concat_sequences_t = typename concat_sequences<T, U>::type;

static_assert(std::is_same_v<concat_sequences_t<index_sequence<42>, index_sequence<7>> , index_sequence<42, 7>>);

template<std::size_t N>
struct do_make_index_sequence;

template<std::size_t N>
using make_index_sequence = typename do_make_index_sequence<N>::type;

template<>
struct do_make_index_sequence<0>
{
    using type = index_sequence<>;
};

template<std::size_t N>
struct do_make_index_sequence
{
    using type = concat_sequences_t<make_index_sequence<N - 1>, index_sequence<N - 1>>;
};

static_assert(std::is_same_v<make_index_sequence<0> , index_sequence<>>);
static_assert(std::is_same_v<make_index_sequence<1>, index_sequence<0>>);
static_assert(std::is_same_v<make_index_sequence<2>, index_sequence<0, 1>>);
static_assert(std::is_same_v<make_index_sequence<3>, index_sequence<0, 1, 2>>);
static_assert(std::is_same_v<make_index_sequence<10>, index_sequence<0, 1, 2,3,4,5,6,7,8,9>>);


template<std::size_t... Is, class... Ts>
    requires (has_update_v<Ts> && ...)
void update(index_sequence<Is...>, shader_program<Ts...>& prog)
{
    (std::get<Is>(prog).update(), ...);
}

template<class... Ts>
    requires (has_update_v<Ts> && ...)
void update(shader_program<Ts...>& prog)
{
    //update(make_index_sequence<sizeof...(Ts)>{}, prog);
    [&prog] <std::size_t... Is> (index_sequence<Is...>){
        (std::get<Is>(prog).update(), ...);
    }(make_index_sequence<sizeof...(Ts)>{});
}

int main()
{
    shader_program prog{vertex_shader{}, geometry_shader{}, fragment_shader{}};

    update(prog);
}