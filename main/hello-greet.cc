#include "main/hello-greet.h"
#include <string>

using namespace std;

string get_greet(const string &who) {
    return "Hello " + who;
}
