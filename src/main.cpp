#include "demodb/db.hpp"


int main(int argc, char * argv[])
{
    printf("Random number %d\n", demodb::generate_random(10));

    return 0;
}
