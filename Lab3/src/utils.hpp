#include <string>
#include <array>
#include <tuple>
using std::string;
using sym = string;
using ptr = int *;

int a, b, c;

template <int N>
struct EndSym
{
    std::array<ptr, N> params{};

    template <std::same_as<ptr>... Args>
        requires(sizeof...(Args) == N)
    constexpr EndSym(Args &&...args)
        : params{args...}
    {
    }
};

template <typename... Ts>
EndSym(Ts...) -> EndSym<sizeof...(Ts)>;

int main()
{
    EndSym fuck(&a, &b, &c);
}
