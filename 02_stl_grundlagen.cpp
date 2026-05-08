

#include <algorithm>
#include <deque>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>


namespace container {

void demo()
{
    // Sequenz-Container
    std::vector<int> vec{5,2,8,1,9};
    std::deque<int> deq{1,2,3};
    std::list<int> lst{10,20,30};

    vec.push_back(10); // O(1)
    deq.push_front(0); // O(1), aber beim vector O(n)
    lst.push_back(40); // O(1)

    // Assoziative Container
    std::map<std::string, int> ages{
        {"Alice", 30},
        {"Bob", 40},
        {"Carol", 25},
    };
    ages["Dave"] = 35; // Einfügen und Updaten

    int age = ages["Eve"]; // Operator [] legt nicht-existierende Keys an!

    if (auto it = ages.find("Frank"); it != ages.end()){
        std::cout << it->second; // first Schlüssel, second Wert
    }

    for (const auto& [name, age] : ages){
        std::cout << "  " << name << " = " << age << '\n';
    }

    // --- Hash-basiert: std::unordered_map (amortisiert O(1) lookup) ---
    std::unordered_map<int, std::string> id_to_name{
                                                    {1, "Sensor A"},
                                                    {2, "Sensor B"},
                                                    };
    std::cout << "ID 1 → " << id_to_name[1] << '\n';

    // --- std::set: sortierte, eindeutige Werte ---
    std::set<int> unique_ids{3, 1, 4, 1, 5, 9, 2, 6};   // Duplikate verschwinden
    std::cout << "Eindeutige IDs:";
    for (int id : unique_ids) std::cout << ' ' << id;
    std::cout << '\n';

}
}

namespace algorithmen {

void demo(){

    std::vector<int> v{1,2,3,1,9,7,4,6};
    std::sort(v.begin(),v.end(), std::greater<int>()); // Liste Sortieren

    for (const auto& x:v){
        std::cout << x << " ";
    }

    // Suchen

    auto it = std::find(v.begin(),v.end(),1);
    if (it != v.end()){
        std::cout << std::distance(v.begin(),it) << "\n";
    }

    // Aggregationen

    int sum = std::accumulate(v.begin(),v.end(),0);

    // Zählen mit Prädikat (Lambda)

    auto gt3 = std::count_if(v.begin(),v.end(),
                             [](int x) {return x > 3;});

    std::vector<int> evens;
    std::copy_if(v.begin(),v.end(),std::back_inserter(evens),[](int x){return x%2 ==0;});



}

}

namespace iteratoren {

void demo(){
    // -- input iterator: einmal vorwärts lesen
    std::istringstream input{"10 1.5 30"};
    std::istream_iterator<double> in{input};


    std::cout << "input:   " << *in;
    ++in;
    std::cout << ' ' << *in << "\n";
    ++in;
    std::cout << ' ' << *in << "\n";

    // einmal vorwärts schreiben
    std::vector<int> out;
    auto inserter = std::back_inserter(out);
    *inserter = 1;
    *inserter = 2;
    *inserter = 3;
    for (int x : out) std::cout << x << ' ';

    std::cout << '\n';
    // bidirectional

    std::list lst{1,2,3};
    auto lit = lst.end();
    --lit;
    std::cout << *lit;
    std::cout << '\n';

    std::vector<int> v{1,2,3,1,9,7,4,6};
    auto vit = v.begin();
    vit +=2;
    std::cout << *vit;



}

}


int main()
{
    std::cout << "=== Container ===\n";
    container::demo();
    algorithmen::demo();
    iteratoren::demo();


}
