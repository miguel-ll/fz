#ifndef FZY_H
#define FZY_H

int has_match(const char *needle, const char *haystack);
double match_positions(const char *needle, const char *haystack, size_t *positions);
double match(const char *needle, const char *haystack);

#endif

