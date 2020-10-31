#include "demodb/db.hpp"
#include "rocksdb/db.h"

int main(int argc, char * argv[])
{
    printf("Random number %d\n", demodb::random::generate_random(10)); 

    printf("Example status: %s\n", rocksdb::Status::OK().ToString().c_str());

    return 0;
}
