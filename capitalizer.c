/*
 * An algorithm which parses through an input text file and
 *  capitalizes the first letter of the first word of each sentence,
 *  if it isn't already.
 *
 * Takes an input filename and output filename as arguments.
 */

#include "capitalizer.h"

#ifndef HAVE_STRSEP // for maximum portability
char * strsep(char **sp, char *sep)
{
    char *p, *s;
    if (sp == NULL || *sp == NULL || **sp == '\0') return(NULL);
    s = *sp;
    p = s + strcspn(s, sep);
    if (*p != '\0') *p++ = '\0';
    *sp = p;
    return(s);
}
#endif

#define DELIMITER " " // used to separate words

/* some common abbreviations that might be tricky */
const char* abbrevs[] = {
    "e.g.",
    "i.e.",
    "a.m.",
    "p.m.",
    "a.m",
    "p.m",
    "etc.",
};

int capitalize(const char* input, const char* output) {

    /* check for invalid input */
    if (input == output) {
        fprintf(stderr, "Error: Please select two different files.\n");
        return -1;
    }

    /* set up file pointers and access modes */
    FILE* fp_in;
    FILE* fp_out;
    fp_in  = fopen(input,  "r");
    fp_out = fopen(output, "w");

    if (fp_in == NULL || fp_out == NULL) {
        fprintf(stderr, "Error: Could not open files.\n");
        return -1;
    }

    /* Sentence splitting strategy:
     *  0. Tokenize with space characters as delimiters.
     *  0b. Identify newlines and try to account for them.
     *  1. Check for punctuation. (periods, exclamation points, question marks)
     *  1b. If a period is found, check if it's part of an abbreviation
     *  2. Check if the next word is capitalized.
     *
     *  Although this strategy isn't as effective as what could be implemented
     *      using more advanced lexical analysis such as with NLTK, it covers
     *      the majority of cases and works as a quick solution.
     */

    /* read the entire file into memory. This avoids clipping
        sentences in places where we shouldn't. */
    char* input_text;
    char* output_text;
    fseek(fp_in, 0, SEEK_END);
    int in_size = ftell(fp_in); // gets the number of bytes in the file
    fseek(fp_in, 0, SEEK_SET); // rewind
    input_text  = (char*) calloc(in_size, sizeof(char)); // malloc and set to 0
    output_text = (char*) calloc(in_size, sizeof(char));

    /* tokenize the text from the input file. */
    if (input_text != NULL && output_text != NULL) {
        fread(input_text, sizeof(char), in_size, fp_in);
        char* prev  = strsep(&input_text, DELIMITER); // get the first token
        char* token = strsep(&input_text, DELIMITER); // get the second token
        bool is_abbrev = false;

        // capitalize the first letter of the first word by default
        if (prev[0] != '\0') prev[0] = toupper(prev[0]);
        else token[0] = toupper(token[0]);

        while (token != NULL) { // loop through the rest

            // printf("prev:[%s], token:[%s]\n", prev, token); // DEBUG

            // when there's a newline in a token we try to split along it
            if (strchr(prev, '\n')) {
                char* temp = (char*) calloc(128, sizeof(char));
                strncpy(temp, prev, 128);
                char* inter = strsep(&temp, "\n");

                // now inter is the first half and temp is the second
                // we make the same punctation checks in inter as we do below
                if (strchr(inter, '!') || strchr(inter, '?')){
                    temp[0] = toupper(temp[0]);
                }
                if (strchr(inter, '.')) {
                    is_abbrev = false;
                    for (int i = 0; i < (sizeof(abbrevs)/sizeof(abbrevs[0])); i++) {
                        if (strstr(inter, abbrevs[i])) {
                            is_abbrev = true;
                            break;
                        }
                    }
                    if (strchr(inter, '.') == strrchr(inter, '.')) {
                        if (!is_abbrev) temp[0] = toupper(temp[0]);
                    }
                }
                char* buf = (char*) calloc(128, sizeof(char));
                strncat(buf, inter, 128);
                strncat(buf, "\n", 128);
                strncat(buf, temp, 128);

                /* write back to output one token at a time */
                fputs(DELIMITER, fp_out); // recover the delimiter
                fputs(buf, fp_out);

                // release memory
                free(temp);
                free(buf);
            } else {

                if (strchr(prev, '!') || strchr(prev, '?')){
                    // end of a sentence
                    token[0] = toupper(token[0]);
                }
                if (strchr(prev, '.')) {
                    // check for common abbreviations
                    is_abbrev = false;
                    for (int i = 0; i < (sizeof(abbrevs)/sizeof(abbrevs[0])); i++) {
                        if (strstr(prev, abbrevs[i])) {
                            is_abbrev = true;
                            break;
                        }
                    }

                    // If there are multiple periods in a token, the token is
                    //   likely an abbreviation (such as i.e., e.g., a.m.).
                    // But if there's only one, it's probably a sentence end
                    if (strchr(prev, '.') == strrchr(prev, '.')) {
                        if (!is_abbrev) token[0] = toupper(token[0]);
                    }
                }

                /* write back to output one token at a time */
                fputs(DELIMITER, fp_out); // recover the delimiter
                fputs(prev, fp_out);
            }
            // fetch the next token
            prev  = token;
            token = strsep(&input_text, DELIMITER);

            // catch the last token in the loop and write it
            if (token == NULL) {
                fputs(DELIMITER, fp_out);
                fputs(prev, fp_out);
            }
        }

    }

    /* close files */
    fclose(fp_in);
    fclose(fp_out);
    free(input_text);  // release memory
    free(output_text);
    return 0;
}

int main() {
    if (-1 == capitalize("input.txt", "output.txt")) fprintf(stderr,"Failed.\n");
    return 0;
}
