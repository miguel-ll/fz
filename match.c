#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "fz.h"

int has_match(const char *needle, const char *haystack){
	while(*needle){
		if(!*haystack)
			return 0;
		while(*haystack && tolower(*needle) == tolower(*haystack++))
			needle++;
	}
	return 1;
}

#define max(a, b) (((a) > (b)) ? (a) : (b))
typedef double score_t;
#define SCORE_MAX 1000000.00
#define SCORE_MIN -1000000.00

double calculate_score(const char *needle, const char *haystack, size_t *positions){
	if(!*haystack || !*needle)
	    return SCORE_MIN;

	int n = strlen(needle);
	int m = strlen(haystack);

	if(m > 1024)
	    return SCORE_MIN;
	/* unreasonably large candidate: return no score; if it is a valid match it will still be returned, it will just be ranked below any reasonably sized candidates */

	score_t match_bonus[m];
	score_t D[n][m], M[n][m];
	bzero(D, sizeof(D));
	bzero(M, sizeof(M));

	/* D[][] Stores the best score for this position ending with a match. M[][] Stores the best possible score at this position. */

#define SCORE_GAP_LEADING      -0.005
#define SCORE_GAP_TRAILING     -0.005
#define SCORE_GAP_INNER        -0.01
#define SCORE_MATCH_CONSECUTIVE 1.0
#define SCORE_MATCH_SLASH       1.5
#define SCORE_MATCH_WORD        1.2
#define SCORE_MATCH_CAPITAL     1.1
#define SCORE_MATCH_DOT         0.8

	/* Which positions are beginning of words */
	char last_ch = '\0';
	for(int i = 0; i < m; i++){
		char ch = haystack[i];

		score_t score = 0;
		if(isalnum(ch)){
			if(!last_ch || last_ch == '/'){
				score = SCORE_MATCH_SLASH;
			}else if(last_ch == '-' || last_ch == '_' || last_ch == ' ' || (last_ch >= '0' && last_ch <= '9')) {
				score = SCORE_MATCH_WORD;
			}else if(last_ch >= 'a' && last_ch <= 'z' &&
					ch >= 'A' && ch <= 'Z'){
				/* CamelCase */
				score = SCORE_MATCH_CAPITAL;
			}else if(last_ch == '.'){
				score = SCORE_MATCH_DOT;
			}
		}

		match_bonus[i] = score;
		last_ch = ch;
	}

	for(int i = 0; i < n; i++){
		for(int j = 0; j < m; j++){
			score_t score = (j || i) ? SCORE_MIN : 0;
			int match = tolower(needle[i]) == tolower(haystack[j]);
			D[i][j] = SCORE_MIN;
			if(match){
				if(i && j){
					score = max(score, M[i-1][j-1] + match_bonus[j]);

					/* consecutive match, doesn't stack with match_bonus */
					score = max(score, D[i-1][j-1] + SCORE_MATCH_CONSECUTIVE);
				}else if(!i){
					score = (j * SCORE_GAP_LEADING) + match_bonus[j];
				}
				D[i][j] = score;
			}
			if(j){
				if(i == n-1){
					score = max(score, M[i][j-1] + SCORE_GAP_TRAILING);
				}else{
					score = max(score, M[i][j-1] + SCORE_GAP_INNER);
				}
			}
			M[i][j] = score;
		}
	}
	/* backtrace to find the positions of optimal matching */
	if(positions){
		for(int i = n-1, j = m-1; i >= 0; i--){
			for(; j >= 0; j--){
				/* there may be multiple paths which result in the optimal weight. For simplicity, we will pick the first one we encounter, the latest in the candidate string. */
				if(tolower(needle[i]) == tolower(haystack[j]) && D[i][j] == M[i][j]){
					positions[i] = j--;
					break;
				}
			}
		}
	}
	return M[n-1][m-1];
}

double match_positions(const char *needle, const char *haystack, size_t *positions) {
	if(!*needle)
	    return SCORE_MAX;
	else if(!has_match(needle, haystack)){
	    return SCORE_MIN;
	} else if(!strcasecmp(needle, haystack)){
		if(positions){
			int n = strlen(needle);
			for(int i = 0; i < n; i++)
				positions[i] = i;
		}
		return SCORE_MAX;
	}
	return calculate_score(needle, haystack, positions);
}

double match(const char *needle, const char *haystack){
	return match_positions(needle, haystack, NULL);
}

