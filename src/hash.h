#ifndef HASH_H
#define HASh_H

#include <stddef.h>



unsigned int first_level_hashing(const char *s); 
unsigned int second_level_hashing(const char *s, unsigned int seed);

#endif
