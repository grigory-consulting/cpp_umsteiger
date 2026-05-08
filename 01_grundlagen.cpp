

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>


namespace variablen {

void demo()
{
    int i = 3.14;
    double y = 2.0;
    char c = 'X';
    bool b = true;

    //int x{3.14};

    int x{7};

    auto z = 2.5;

    auto s = std::string("tschüss");

    std::string s1 = std::to_string(x);

    auto s2 = s+s1;


    const int N = 10;
    constexpr int MAX = 2*N; // wird zur Compile-Zeit berechnet


    std::cout << " x=" << x << " y=" << s+s1 << "\n";
}

} // namespace variablen


namespace kontrollfluss {

// lookup

std::pair<bool, int> lookup(int key)
{
    if (key == 1) return {true, 1337};
    return {false,0};

}


void demo(){
    if (auto [found, value] = lookup(1); found){
        std::cout << "gefunden \n";
    } else {
        std::cout << "nicht gefunden";
    }

    int mode = 2;
    switch (mode){
    case 1:
        std::cout << "Modus 1\n";
        break;
    case 2:
        std::cout << "Modus 2\n";
        [[fallthrough]];
    case 3:
        std::cout << "Modus 3\n";
        break;
    default:
        std::cout << "Default";
    }

    std::array<int,5> data{10,20,30,40,50};

    // range-based (C++11) - wenn kein Index gebraucht wird
    int sum = 0;
    for (const auto& xx : data) sum += xx;

    std::cout << sum << "\n";

    for(auto& n : data){
        n*=2;
    }

    for (const auto& xx : data) std::cout << xx << " ";


    // Klassische For-Schleife (aber nicht ganz klassisch)

    for (std::size_t i = 0; i < data.size() ; ++i){
        std::cout << i;
    }


}

}

class Sensor{

public:
    Sensor(std::string name, double threshold)
        : name_{std::move(name)}
        , threshold_{threshold}
    {}

    // typischer Getter
    const std::string& name() const noexcept {return name_;}

    bool triggers(double value) {return value > threshold_;}

private:
    std::string name_;
    double threshold_;

};



// -----------------------------------------------------------------------------
int main()
{

    variablen::demo();
    kontrollfluss::demo();
    Sensor temp{"Temp 1", 80.0};
    std::cout << temp.name() << "\n85 C -> triggers " << temp.triggers(85.0) << "\n";



}
