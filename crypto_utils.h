#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define KEY_VALUE 4

void encrypt (char* text, int key); 
void decrypt (char* text, int key);

#endif
