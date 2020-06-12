/*
 * An algorithm which parses through an input text file and
 *  capitalizes the first letter of each sentence, if it isn't already.
 *
 * Takes an input filename and output filename as arguments.
 */

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

int capitalize(const char* input, const char* output);
