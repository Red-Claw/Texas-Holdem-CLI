#ifndef HANDS_H
#define HANDS_H

#include <stdlib.h>
#include <stdio.h>


//struct that contains 4 variables to indicate the exact power of the hand
struct hand_value {
    // hand_combination is the basic hand type (0 = high card, 1 = pair, 2 = two pair, 3 = three of a kind, 4 = straight, ...)
    int hand_combination;

    //if both players have the same hand_combination, then whoever has the highest tiebreaker wins
	//example : pair of 8 would have hand_combination = 1 and tie_breaker = 8, which would beat a pair of 5 with values 1 and 5
    int tie_breaker;

    //second tie_breaker in the specific case where the hand is a two-pair, this is the value of the lower pair
    int secondary;

    //if the 3 values above are identical, this here is the value of the highest card outside of the hand combination
    int high_card;
};

int min_int(int a, int b);
int max_int(int a, int b);
void initialize_hand_estimations();
void get_hand_value(int *hand, int len, struct hand_value * result);
void sort_list(int * arr, int len);
int get_high_card(int *hand, int len, int first_ignore, int second_ignore);
int get_ai_bet(int hand_combination, int bank, int blind, int betting_round, int ai_bluff);
int get_ai_raise(int hand_combination, int round, int raise_ratio);
void print_list(int * arr, int len);    //debug only
void print_hand_string(struct hand_value * hand_eval);

int hand_estimations[3][10];




#endif // HANDS_H
