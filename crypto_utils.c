#include "crypto_utils.h"

void encrypt (char* text, int key) 
{
    for (int i = 0 ; text[i] != '\0' ; i++) 
    {
        if (isalpha(text[i])) 
        {
            char base = islower(text[i]) ? 'a' : 'A';
            text[i] = (text[i] - base + key) % 26 + base;
        }
    }
}

void decrypt (char* text, int key) 
{	encrypt(text, 26 - (key % 26));	} 

