#ifndef DISPLAY_H
#define DISPLAY_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HEART 0
#define CLUB 1
#define DIAMOND 2
#define SPADE 3
#define BACK 4



//matrix of strings : each string is a line for a card * height of 12 per card * 5 types of cards (each suit + face down)
char *card_parts[5][12];
char card_value_initial[13];

void display_game(int *player_hand, int *ai_hand, int player_bank, int ai_bank, int player_bet, int ai_bet, int cards_revealed);
void print_hand(int *hand, int hand_length, int cards_revealed, int show_all);
void initialize_cards();



#endif // DISPLAY_H
