
#include <iostream>

#include "client.hpp"

int main(){
    Client client{"localhost", "12345"};
    client.Run();


    return 0;
}