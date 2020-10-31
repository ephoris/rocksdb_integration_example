#include "demodb/db.hpp"

using namespace demodb;

int random::generate_random(int upper_limit)
{
    return (std::rand() % upper_limit) + 1;
}
