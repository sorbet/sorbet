#include "main/hello-greet.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
    string who = "world";
    if (argc > 1) {
        who = argv[1];
    }
    cout << get_greet(who) << endl;
    return 0;
}
