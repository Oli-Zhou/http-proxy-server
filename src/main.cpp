#include "proxy.h"

int main(int argc, char const* argv[]){
    const char * port = "12345";
    proxy proxy(port);
    proxy.start();

}