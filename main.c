#include <stdio.h>      //printf fgets
#include <time.h>       //time()
#include <unistd.h>     //getopt args
#include <stdlib.h>     //strtol
#include <limits.h>     //INT_MAX
#include <ctype.h>      //tolower()

#include "hands.h"
#include "display.h"

//default not full screen terminal window is 120 chars long
//fullscreen 1440p : 317 chars

//Programming project for my programming course, by Lukas Roncalli
//Game of Poker Texas Hold em 1v1 against an AI


//Every card is expressed by an integer from 0 to 51.
//The cards go from 2 to Ace, from Hearts to Clubs to Diamonds to Spades
//For example number 19 is 8 of Clubs, 35 is Jack of Diamonds, etc...
//From a number, the suit is the result of the integer division by 13 (0 = Hearts, 1 = Clubs, 2 = Diamonds, 3 = Spades)
//35 / 13 = 2 -> Diamonds     19 / 13 = 1 -> Clubs
//The rank of the card is obtained by doing the modulo base 13 (0 = '2', 1 = '3', ... , 11 = 'King', 12 = 'Ace')
// 35 % 13 = 9 -> Jack      19 % 13 = 6 -> 8


//Notations :
//A 'match' ends when either players banks reaches 0, a program instance is one match
//A 'game' when all the cards are shown or a player folds
//A 'round' is a single turn of betting, a game can have up to 3 rounds


void generate_hands(int *player_hand, int *ai_hand);
void play_game(int *player_bank_ptr, int *ai_bank_ptr, int blind, int seed, int first_goer);
int get_player_bet_input(int max_bet, int min_bet, int seed, char * input_message, int raise);
void resolve_game(int * player_hand, int * ai_hand, int player_bet, int ai_bet, int * player_bank_ptr, int * ai_bank_ptr);

//accepted arguments : bank, seed, blind bet
//-c (coin amount) to set the starting bank, -s (seed) to set the seed, -b to define the blind, -h for help


int main(int argc, char *argv[]){

    //default values for each argument
    int bank = 300;
    int blind = 20;
    int seed = time(NULL);

    //argument parsing
    int opt;
    while((opt = getopt(argc, argv, "hc:s:b:")) != -1){
        char *junk;
        //strtol to convert strings into integers
        //Long ints are first used and then transfered in regular ints if it fits
        long temp_coin;
        long temp_seed;
        long temp_blind;

        switch(opt){
            case 'h':
                printf("\n\nList of acceptable arguments for this program :\n\n");
                printf("-c followed by a positive integer >0 to set the total amount of coins each player will have.\n\n");
                printf("-s followed by a positive integer to set the seed for this game. The seed of a game can\n");
                printf("be obtained at any time by entering 'seed'.\n\n");
                printf("-b followed by a positive integer >0 to set the blind bet for each round. If this is greater\n");
                printf("than the bank the first round will be an all in!\n\n");
                printf("Any incorrect or unusable argument will be discarded and the default value(s) will be used instead.\n\n\n");
                return 0;
            case 'c':
                temp_coin = strtol(optarg, &junk, 10);
                //check that temp is in range of a regular int
                if (temp_coin > 0 && temp_coin <= INT_MAX)
                    bank = temp_coin;
                break;
            case 's':
                temp_seed = strtol(optarg, &junk, 10);
                //check that temp is in range of a regular int
                if (temp_seed > INT_MIN && temp_seed < INT_MAX && temp_seed != 0)
                    seed = temp_seed;
                break;
            case 'b':
                temp_blind = strtol(optarg, &junk, 10);
                //check that temp is in range of a regular int
                if (temp_blind > 0 && temp_blind <= INT_MAX)
                    blind = temp_blind;
                break;
        }
    }



    srand(seed);

    int player_bank = bank;
    int ai_bank = bank;
    //first_goer at 1 means the player starts, 0  for the AI to start
    int first_goer = rand() % 2;

    initialize_cards();
    initialize_hand_estimations();

    printf("\nYou are about to play Poker Texas Holdem!\nThis game is better experienced in a full screen window.\n\nBlind : %d\t\tBank : %d\t\tSeed : %d\n\nPress Enter to start : ", blind, bank, seed);
    char trash[100];
    fgets(trash, 100, stdin);

    //Main gamne loop, ends when a player loses (their bank reaches 0)
    while(player_bank && ai_bank){

        printf("\n\n\n\n\n\n\n\n");
        if (first_goer){
            printf("The AI goes first this game!\n");
            first_goer = 0;
        }
        else {
            printf("You go first this game!\n");
            first_goer = 1;
        }

        play_game(&player_bank, &ai_bank, blind, seed, first_goer);

        printf("\n\nPress Enter to continue and start the next game : ");
        char trash[100];
        //to catch the 'Enter' press, we use fgets rather than getchar() to avoid input buffer saturation
        //The User can write up to 99 characters that will be ignored
        //100 is arbitrary
        fgets(trash, 100, stdin);


    }


    //End of the loop, the winner is announced
    if (player_bank)
        printf("\n\n\nYou won! Congratulations!\n\n");
    else
        printf("\n\n\nYou ran out of coins! Better luck next time!\n\n");

    return 0;
}


//main function called for every instance of a new game
//The function works as a choice tree with a different text dialogue output depending on the scenario
//Bank values passed as pointers since we wish to modify them
void play_game(int * player_bank_ptr, int *ai_bank_ptr, int blind, int seed, int first_goer){

    //Hand are expressed two lists of 7 ints (1 for the player, 1 for the AI), the first 2 ints of each list
	//are their private cards, the last 5 are the shared cards
    //The 5 shared cards are stored twice which is redundant, but it makes the hand analyzing simpler
    int player_hand[7];
    int ai_hand[7];

    generate_hands(player_hand, ai_hand);


    int player_bet = 0;
    int ai_bet = 0;

    //struct that contains the exact power level of any 7 card hand
    //see definition in hands.h
    //Here we only analyze the AIs hand to adjust its betting strategy, the final player vs AI breakdown is done later
    struct hand_value ai_v;
    struct hand_value *ai_value = &ai_v;

    //numbers of shared cards that have been revealed so far (0, 3, 4 or 5)
    int cards_revealed = 0;
    //Every game the AI has a 25% chance to bluff
    int ai_bluff = 0;
    if (rand()%4 == 0)
        ai_bluff = 1;

    //loop of single game, each iteration is 1 round
    for (int round_count = 0; round_count < 3; round_count++){

        int player_round_bet = 0;
        int ai_round_bet = 0;
        if (round_count)
            cards_revealed = round_count + 2;

        display_game(player_hand, ai_hand, *player_bank_ptr, *ai_bank_ptr, player_bet, ai_bet, cards_revealed);

        // case of the first round (dialogue is different and the blind bet is used as minimum)
        if (!round_count){
            get_hand_value(ai_hand, 2, ai_value);
            //player goes first
            if (first_goer){
                printf("You go first this round. The blind bet is %d.", blind);
                int first_bet = min_int(blind, *player_bank_ptr);
                //the blind forces the player to all in
                if (*player_bank_ptr == first_bet){
                    *player_bank_ptr = 0;
                    player_bet = first_bet;
                    printf(" You go all in!\n");
                    //the AI has to all in as well
                    if (*ai_bank_ptr <= first_bet){
                        printf("The AI goes all in as well!");
                        ai_bet = *ai_bank_ptr;
                        *ai_bank_ptr = 0;
                    }
                    //AI does not need to all in to match your all in
                    else {
                        printf("The AI matches your all in!");
                        ai_bet = first_bet;
                        *ai_bank_ptr -= first_bet;
                    }
                    resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                    return;
                }
                //Players has coins left in the bank after the blind
                else {
                    player_bet = first_bet;
                    *player_bank_ptr -= first_bet;
                    printf(" You bet %d.\n", blind);
                    //AI has to all in to follow the blind
                    if (*ai_bank_ptr <= first_bet){
                        printf("The AI has to all in to match the blind!\n");
                        ai_bet = *ai_bank_ptr;
                        *ai_bank_ptr = 0;

                        return;
                    }
                    //AI does not need to all in to follow the blind
                    else {
                        int ai_raise = max_int(first_bet, get_ai_bet(ai_value->hand_combination, *ai_bank_ptr, blind, 0, 0));
                        //If the AIs bet was calculated to be higher than the base blind, it has 50% chance to use the higher value
                        if (rand()%2 && ai_raise > first_bet){
                            printf("The AI raises and bets %d. Do you accept the raise?\n", ai_raise);
                            *ai_bank_ptr -= ai_raise;
                            ai_bet += ai_raise;
                            //The AIs raise makes it all in
                            if (*ai_bank_ptr == 0){
                                printf("The AIs raise was an all in!\n");
                            }
                            //the player folds against the all in raise
                            if (!get_player_bet_input(0, 0, seed, "'Yes' to accept, 'Fold' to refuse : ", 1)){
                                printf("\n\nYou fold and lose this game!");
                                *ai_bank_ptr += (ai_bet + player_bet);
                                return;
                            }
                            //The players accepts the raise
                            //player has to all in to accept the raise
                            if (*player_bank_ptr <= ai_bet - player_bet){
                                printf("\nYou all in to match the AI!\n");
                                player_bet += *player_bank_ptr;
                                *player_bank_ptr = 0;
                                resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                                return;
                            }
                            //gplayer doesn't have to all in to accept the raise
                            *player_bank_ptr -= (ai_bet - player_bet);
                            player_bet = ai_bet;
                            //If the AI was all in the game resolves immediately
                            if (*ai_bank_ptr == 0){
                                resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                                return;
                            }
                        }
                        else {
                            printf("The AI matches your blind bet.\n");
                            *ai_bank_ptr -= first_bet;
                            ai_bet += first_bet;
                        }
                    }
                }
            }
            //AI goes first
            else {
                printf("The AI goes first this round. The blind bet is %d.", blind);
                int first_bet = min_int(blind, *ai_bank_ptr);
                //AI has to all in to put the blind
                if (*ai_bank_ptr == first_bet){
                    printf(" The AI has to all in!\nDo you want to match the bet or fold?\n");
                    ai_bet += *ai_bank_ptr;
                    *ai_bank_ptr = 0;
                    //player folds against the blind
                    if (!get_player_bet_input(0, 0, seed, "'Yes' to accept, 'Fold' to refuse : ", 1)){
                        printf("\n\nYou fold and lose this game!");
                        *ai_bank_ptr += (ai_bet + player_bet);
                        return;
                    }
                    //player accepts the AIs blind
                    //player has to all in to follow the AIs blind
                    if (*player_bank_ptr  <= ai_bet)
                        printf("\nYou all in as well!");
                    player_bet += min_int(*player_bank_ptr, first_bet);
                    *player_bank_ptr -= min_int(*player_bank_ptr, first_bet);
                    resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                    return;
                }
                //AI does not have to all in to place the blind
                else {
                    *ai_bank_ptr -= first_bet;
                    ai_bet += first_bet;
                    //player doesn't have enough in the bank to follow the blind
                    if (*player_bank_ptr < first_bet){
                        printf("\nYou don't have enough to match the blind, you can either all in or fold.");
                        //player folds against the blind
                        if(!get_player_bet_input(0, 0, seed, "Enter 'Yes' to all in, or 'Fold' : ", 1)){
                            printf("\n\nYou fold and lose this game!");
                            *ai_bank_ptr += (ai_bet + player_bet);
                            return;
                        }
                        player_bet = *player_bank_ptr;
                        *player_bank_ptr = 0;
                        resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                        return;
                    }
                    //player has enough in the bank to follow the blind
                    if (*player_bank_ptr == first_bet)
                        printf("\nYou can either match the blind or fold.");
                    else
                        printf("\nYou can match the blind, raise or fold.");
                    player_bet = get_player_bet_input(*player_bank_ptr, first_bet, seed, "\nEnter a bet or 'Fold' : ", 0);
                    //the player folds
                    if (player_bet == -1){
                        printf("\n\nYou fold and lose this game!");
                        *ai_bank_ptr += (ai_bet);
                        return;
                    }
                    *player_bank_ptr -= player_bet;
                    //player went all in
                    if (*player_bank_ptr == 0)
                        printf("\nYou went all in!");
                    //if the player bet more than the AI, check if it wants to follow
                    if (player_bet > first_bet && !get_ai_raise(ai_value->hand_combination, round_count, (player_bet - first_bet) / first_bet)){
                        printf("\nThe AI folds against your raise, you win this game!\n");
                        *player_bank_ptr += (ai_bet + player_bet);
                        return;
                    }
                    ai_bet += min_int(*ai_bank_ptr, player_bet - ai_bet);
                    *ai_bank_ptr -= (ai_bet - first_bet);
                    if (player_bet > first_bet)
                        printf("\nThe AI matched your raise.");
                    if (!*ai_bank_ptr)
                        printf("\nThe AI went all in!");
                    if (!*ai_bank_ptr || !*player_bank_ptr){
                        resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                        return;
                    }

                }
            }
        }
        //any round except the first
        else {
            get_hand_value(ai_hand, cards_revealed + 2, ai_value);
            if (round_count == 1)
                printf("\n3 new cards are revealed!\n");
            else
                printf("\nA new card is revealed!\n");
            //player starts
            if (first_goer){
                printf("\nYou go first this round, make a bet or fold!\n");
                player_round_bet = get_player_bet_input(*player_bank_ptr, 1, seed, "Enter a bet or 'Fold' : ", 0);
                if (player_round_bet == -1){
                    printf("\n\nYou fold and lose this game!");
                    *ai_bank_ptr += (ai_bet + player_bet);
                    return;
                }
                player_bet += player_round_bet;
                *player_bank_ptr -= player_round_bet;
                ai_round_bet = get_ai_bet(ai_value->hand_combination, *ai_bank_ptr, blind, round_count, ai_bluff);
                //player all ins
                if (*player_bank_ptr == 0){
                    printf("\nYou went all in!");
                    //AI voleva all in con un valore minore o uguale del giocatore
                    if (*ai_bank_ptr == ai_round_bet && ai_round_bet <= player_round_bet){
                        printf("\nThe AI goes all in as well!");
                        ai_bet += ai_round_bet;
                        *ai_bank_ptr = 0;
                        resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                        return;
                    }
                    //AI intended to bet more than the players AI
                    if (ai_round_bet >= player_round_bet){
                        printf("\nThe AI matches your all in.");
                        ai_bet += player_round_bet;
                        *ai_bank_ptr -= player_round_bet;
                        resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                        return;
                    }
                    //only remaining scerario : AI wanted to bet less than the player (not all in)
                    //check if the AI wants to follow the bet
                    if (get_ai_raise(ai_value->hand_combination, round_count, (player_round_bet - ai_round_bet) / blind)){
                        //The AIs raise could be an all in
                        printf("\nThe AI matches your all in.");
                        ai_round_bet = min_int(*ai_bank_ptr, player_round_bet);
                        ai_bet += ai_round_bet;
                        *ai_bank_ptr -= ai_round_bet;
                        resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                        return;
                    }
                    //AI refuses the raise
                    printf("\nThe AI folded against your all in, you win this game!\n");
                    *player_bank_ptr += (ai_bet + player_bet);
                    return;
                }
                //The AI does not all in
                if (ai_round_bet == *ai_bank_ptr){
                    printf("\nThe AI has gone all in!");
                    ai_bet += ai_round_bet;
                    *ai_bank_ptr = 0;
                    //AIs all in is a raise of the players bet
                    if (ai_round_bet > player_round_bet){
                        printf("\nYou have to match the all in or fold!");
                        if (*player_bank_ptr == (ai_round_bet - player_round_bet))
                            printf("\nMatching this will require you to all in as well.");
                        else if (*player_bank_ptr < (ai_round_bet - player_round_bet))
                            printf("\nYou don't have enough to match, but you can go all in too.");
                        //player folds
                        if (!get_player_bet_input(0, 0, seed, "\n'Yes' to accept, 'Fold' to refuse : ", 1)){
                            *ai_bank_ptr += (ai_bet + player_bet);
                            printf("\n\nYou fold and lose this game!");
                            return;
                        }
                        *player_bank_ptr -= min_int(*player_bank_ptr, ai_round_bet - player_round_bet);
                        player_bet += min_int(*player_bank_ptr, ai_round_bet - player_round_bet);
                    }
                    resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                    return;
                }
                //AIs bet is not an all in
                //AI bet more than the player
                if (ai_round_bet > player_round_bet){
                    printf("\nThe AI raised and bet %d! You have to bet %d more or fold.", ai_round_bet, ai_round_bet - player_round_bet);
                    if (ai_round_bet - player_round_bet == *player_bank_ptr)
                        printf("\nMatching this will require going all in!");
                    else if (ai_round_bet - player_round_bet > *player_bank_ptr)
                        printf("\nYou don't have that much, but you can still all in!");
                    //player folds against the raise
                    if (!get_player_bet_input(0, 0, seed, "\n'Yes' to accept, 'Fold' to refuse : ", 1)){
                        *ai_bank_ptr += (ai_bet + player_bet);
                        printf("\n\nYou fold and lose this game!");
                        return;
                    }
                    *player_bank_ptr -= min_int(*player_bank_ptr, ai_round_bet - player_round_bet);
                    player_bet += min_int(*player_bank_ptr, ai_round_bet - player_round_bet);
                    player_round_bet = ai_round_bet;
                    //the players raise is an all in
                    if (*player_bank_ptr == 0){
                        resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                        return;
                    }
                }
                //AI bet less and does not follow
                if (ai_round_bet < player_round_bet && !get_ai_raise(ai_value->hand_combination, round_count, (player_round_bet - ai_round_bet) / blind)){
                    printf("\nThe AI folds, you win this game!\n");
                    *player_bank_ptr += (ai_bet + player_bet);
                    return;
                }
                //AI follows or had the same bet
                if (ai_round_bet < player_round_bet)
                    printf("\nThe AI matches your bet.");
                ai_round_bet = min_int(ai_round_bet, min_int(*ai_bank_ptr, player_round_bet));
                ai_bet += ai_round_bet;
                *ai_bank_ptr -= ai_round_bet;
            }
            //AI goes first
            else {
                ai_round_bet = get_ai_bet(ai_value->hand_combination, *ai_bank_ptr, blind, round_count, ai_bluff);
                printf("\nThe AI goes first this round! It bet %d.\n", ai_round_bet);
                //AIs bet is an all in
                if (ai_round_bet == *ai_bank_ptr){
                    printf("It went all in!\n");
                    //player has less in bank than the AIs all in
                    if (*player_bank_ptr < ai_round_bet)
                        printf("Although you don't have %d, you can match it by going all in too!\n", ai_round_bet);
                    printf("Do you want to match the all in or fold?\n");
                    //player folds against the all in
                    if (!get_player_bet_input(0, 0, seed, "Enter 'Yes' or 'Fold' : ", 1)){
                        *ai_bank_ptr += (ai_bet + player_bet);
                        printf("\n\nYou fold and lose this game!");
                        return;
                    }
                    player_bet += min_int(*player_bank_ptr, ai_round_bet);
                    *player_bank_ptr -= min_int(*player_bank_ptr, ai_round_bet);
                    ai_bet += ai_round_bet;
                    *ai_bank_ptr = 0;
                    if (!*player_bank_ptr)
                        printf("\nYou went all in!");
                    resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                    return;
                }
                //AI is not all in
                //player has less in bank than AIs all in
                if (*player_bank_ptr < ai_round_bet){
                    printf("You have less than that in Bank, you can still match by going all in.\n");
                    if (!get_player_bet_input(0, 0, seed, "'Yes' to all in or 'Fold' : ", 1)){
                        *ai_bank_ptr += (ai_bet + player_bet);
                        printf("\n\nYou fold and lose this game!");
                        return;
                    }
                    player_bet += *player_bank_ptr;
                    *player_bank_ptr = 0;
                    ai_bet += ai_round_bet;
                    *ai_bank_ptr -= ai_round_bet;
                    resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                    return;
                }
                //Players bank is greater or equal than AIs bet
                player_round_bet = get_player_bet_input(*player_bank_ptr, ai_round_bet, seed, "Enter your bet or 'Fold' : ", 0);
                if (player_round_bet == -1){
                    printf("\n\nYou fold and lose this game!");
                    *ai_bank_ptr += (ai_bet + player_bet);
                    return;
                }
                if (player_round_bet == *player_bank_ptr)
                    printf("You went all in!\n");
                player_bet += player_round_bet;
                *player_bank_ptr -= player_round_bet;
                //Player bet more than the AI and it does not want to follow
                if (player_round_bet > ai_round_bet && !get_ai_raise(ai_value->hand_combination, round_count, (player_round_bet - ai_round_bet) / blind)){
                    printf("\nThe AI folded against your raise, you win this game!\n");
                    *player_bank_ptr += (ai_bet + player_bet);
                    return;
                }
                //AI follows the bet if it was smaller
                //AI has to all in to follow
                if (player_round_bet > ai_round_bet && player_round_bet >= *ai_bank_ptr)
                    printf("\nThe AI all ins in order to match your bet!");
                else if (player_round_bet > ai_round_bet)
                    printf("\nThe AI accepts your raise.");
                ai_round_bet = min_int(*ai_bank_ptr, player_round_bet);
                ai_bet += ai_round_bet;
                *ai_bank_ptr -= ai_round_bet;
                if (!*ai_bank_ptr){
                    resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
                    return;
                }
            }
        }

        //first_goer is inverted, the player that goes first swaps each round
        if (first_goer)
            first_goer = 0;
        else
            first_goer = 1;
        //100 characets for the press enter like before
        printf("\nPress Enter to continue to the next round : ");
        char trash[100];
        fgets(trash, 100, stdin);
    }
    //method called when the game ends, analyzes which player has the better hand and distributes funds to the winner
    resolve_game(player_hand, ai_hand, player_bet, ai_bet, player_bank_ptr, ai_bank_ptr);
}



//generates the 2 hand arrays, for the player and the AI
void generate_hands(int *player_hand, int *ai_hand){
    //creating a 52 card deck
    int deck_size = 52;
    int full_deck[deck_size];
    for (int i = 0; i < deck_size; i++)
    //deck is currently ordered : {0, 1, 2, ...}
        full_deck[i] = i;

    //generation of the 5 shared cards
    //saved from index 2 to 6
    for(int i = 2; i < 7; i++){
        int pick = rand() % deck_size;
        player_hand[i] = full_deck[pick];
        ai_hand[i] = full_deck[pick];
        for(int k = pick; k < deck_size - 1; k++)
            full_deck[k] = full_deck[k+1];
        deck_size--;
    }

    //generation of the 2 private cards for the player
    //index of 0 and 1
    for(int i = 0; i < 2; i++){
        int pick = rand() % deck_size;
        player_hand[i] = full_deck[pick];
        for(int k = pick; k < deck_size - 1; k++)
            full_deck[k] = full_deck[k+1];
        deck_size--;
    }

    //generation of the 2 private cards for the AI
    //index of 0 and 1
    for(int i = 0; i < 2; i++){
        int pick = rand() % deck_size;
        ai_hand[i] = full_deck[pick];
        for(int k = pick; k < deck_size - 1; k++)
            full_deck[k] = full_deck[k+1];
        deck_size--;
    }


    //debug function to print the 2 arrays
    /*
    for(int i = 0; i < 7; i++){
        printf("%d ", player_hand[i]);
    }
    printf("\t\t");
    for(int i = 0; i < 7; i++){
        printf("%d ", ai_hand[i]);
    }
    printf("\n\n");
    */
}


//reads user input string to get a bet amount, or a yes or no to accept a raise
int get_player_bet_input(int max_bet, int min_bet, int seed, char * input_message, int raise){
    //none of the inputs are close to 100 characters, marging of error for input buffer saturation
    //Using a high value like 100 increases the programs reliability in case the user falls asleep on their keyboard
    //100 is an arbitrary value
    char user_input[100];
    int bet_read;
    char *junk;
    int len;
    //infinite loop until the input is accepted, the returns with the input
    while(1)
    {
        printf("%s", input_message);

        fgets(user_input, 100, stdin);
        //remove the final '\n' of the received input string
        user_input[strcspn(user_input, "\n")] = 0;
        len = strlen(user_input);


        //input is case insensitive
        for (int i = 0; i < len; i++)
            user_input[i] = tolower(user_input[i]);

        if (!strcmp(user_input, "exit"))
            exit(0);
        if (!strcmp(user_input, "seed"))
        {
            printf("\n%d\n\n", seed);
            continue;
        }
        //if raise=0 in the arguments, a numerical value is expected as the input
        if (!raise){
            if (!strcmp(user_input, "fold"))
                return -1;
            bet_read = strtol(user_input, &junk, 10);
            if (!bet_read)
            {
                printf("\nInvalid input.\n");
                continue;
            }
            if (bet_read > max_bet)
            {
                printf("\nYou don't have enough in the bank to bet that much!\n");
                continue;
            }
            if (bet_read < min_bet)
            {
                printf("\nYou have to bet more than that!\n");
                continue;
            }
            return bet_read;
        }
        //if raise=1, a yes/no type answer is expected
        if (!strcmp(user_input, "fold") || !strcmp(user_input, "no") || !strcmp(user_input, "n"))
            return 0;
        if (!strcmp(user_input, "accept") || !strcmp(user_input, "yes") || !strcmp(user_input, "y"))
            return 1;
        printf("\nInvalid input.\n");
    }
}


//function to determine who wins, and distribute the bets to the winners bank
//banks passed as pointers so they can be modified
void resolve_game(int * player_hand, int * ai_hand, int player_bet, int ai_bet, int * player_bank_ptr, int * ai_bank_ptr){
    display_game(player_hand, ai_hand, *player_bank_ptr, *ai_bank_ptr, player_bet, ai_bet, 5);
    struct hand_value a, b;
    struct hand_value * player_eval = &a;
    struct hand_value * ai_eval = &b;
    get_hand_value(player_hand, 7, player_eval);
    get_hand_value(ai_hand, 7, ai_eval);

    //each value in the struct is include between 0 and 12
    //since each variable has an evaluation priority, each of them is multiplied by a base of 20 ( in their order of priority)
    //This insure that the variables are tested against eachother in descending order of priority
    //The order of priority is  hand_combination > tie_breaker > secondary > high_card
    int player_score = 8000 * player_eval->hand_combination + 400 * player_eval->tie_breaker + 20 * player_eval->secondary + player_eval->high_card;
    int ai_score = 8000 * ai_eval->hand_combination + 400 * ai_eval->tie_breaker + 20 * ai_eval->secondary + ai_eval->high_card;

    printf("Player has :\t");
    print_hand_string(player_eval);
    printf("\nAI has :\t");
    print_hand_string(ai_eval);

    if (player_score > ai_score){
        printf("\n\nYou win this game! You receive %d points.", player_bet + ai_bet);
        *player_bank_ptr += (ai_bet + player_bet);
    }
    else if (player_score < ai_score){
        printf("\n\nThe AI wins this game! It receives %d points.", player_bet + ai_bet);
        *ai_bank_ptr += (ai_bet + player_bet);
    }
    else {
        printf("\n\nIt's a tie! The bets are refunded.");
        *ai_bank_ptr += ai_bet;
        *player_bank_ptr += player_bet;
    }
    return;
}
