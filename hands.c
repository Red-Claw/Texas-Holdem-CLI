#include "hands.h"


//function that fills the struct hand_value with the appropriate values
//len can be 2, 5, 6 or 7
//reminder that the first 2 cards are private, e the 3/4/5 other are shared/in common
void get_hand_value(int *hand, int len, struct hand_value * result)
{

    //default of 0 for every value
    result->hand_combination = 0;
    result->tie_breaker = 0;
    result->secondary = 0;
    result->high_card = 0;

    //case where there are no shared cards yet (round 1)
    if (len == 2){
        //error
        if (hand[0] == hand[1]){
            result->hand_combination = 1;
            return;
        }
        result->high_card = max_int(hand[0], hand[1]);
        return;
    }

    //the 2 main arrays used to analyze hand combination
    //numbers found is an array of length 13 where every number of index n is equal to the number of elements in the hand of that rank (mod 13)
    //example : player_hand = {18, 25, 36, 17, 42, 4, 20} -> numbers_found = {0, 0, 0, 1, 2, 0, 0, 1, 0, 0, 1, 0, 1}
    //symbols_found is the same but for the suit
    //same example : player_hand = {18, 25, 36, 17, 42, 4, 20} -> symbols_found = {1, 4, 1, 1}
    int numbers_found[13] = {0};
    int symbols_found[4] = {0};
    for (int i = 0; i < len; i++){
        numbers_found[hand[i]%13] += 1;
        symbols_found[hand[i]/13] += 1;
    }


    //copy of the hand that will be ordered
    int hand_sorted[7];
    for (int i = 0; i < len; i++){
        hand_sorted[i] = hand[i];
    }
    sort_list(hand_sorted, len);


    //hand combination analyzed from most powerful to least powerful
    //This is because a hand can contain multiple combination, but we only car
    //about the highest one
    //example : a full house contains a ToaK, two-pair and pair


    //Straight flush and flush royal
    int highest_value = 0;
    int streak = 1;
    int highest_streak = 1;
    for (int i = 1; i < len; i++){
        if (hand_sorted[i-1] + 1 == hand_sorted[i] && hand_sorted[i-1] / 13 == hand_sorted[i] / 13){
            streak++;
            highest_streak = max_int(highest_streak, streak);
            if (highest_streak >= 5)
                highest_value = i;
        }
        else {
            streak = 1;
        }
    }
    if (highest_streak >= 5){
        if (highest_value == 12){
            result->hand_combination = 9;
            return;
        }
        result->hand_combination = 8;
        result->tie_breaker = highest_value;
        return;
    }


    //Four of a kind
    int biggest_pair = 1;
    highest_value = 0;
    for (int i = 0; i < 13; i++){
        if (numbers_found[i] > biggest_pair){
            biggest_pair = numbers_found[i];
            highest_value = i;
        }
    }
    if (biggest_pair == 4){
        result->hand_combination = 7;
        result->tie_breaker = highest_value;
        result->high_card = get_high_card(hand, len, highest_value, -1);
        return;
    }


    //full house
    int triple_count = 0;
    int triple_value = 0;
    int pair_count = 0;
    for (int i = 0; i < 13; i++){
        if (numbers_found[i] == 3){
            triple_count++;
            triple_value = i;
        }
        if (numbers_found[i] == 2){
            pair_count++;
        }
    }
    if (triple_count && pair_count){
        result->hand_combination = 6;
        result->tie_breaker = triple_value;
        return;
    }


    //flush
    int highest_symbol = 0;
    int flush_symbol = 0;
    for (int i = 0; i < 4; i++){
        if (symbols_found[i] > highest_symbol){
            highest_symbol = symbols_found[i];
            flush_symbol = i;
        }
    }
    if (highest_symbol >= 5){
        result->hand_combination = 5;
        //technically impossible for both players to have different flushes
        result->tie_breaker = flush_symbol;
        return;
    }

    //straight
    highest_value = 0;
    streak = 1;
    highest_streak = 1;
    for (int i = 1; i < 13; i++){
        if (numbers_found[i-1] && numbers_found[i]){
            streak += 1;
            highest_streak = max_int(highest_streak, streak);
            if (streak >= 5){
                highest_value = i;
            }
        }
        else {
            streak = 1;
        }
    }
    if (highest_streak >= 5){
        result->hand_combination = 4;
        result->tie_breaker = highest_value;
    }

    //Three of a kind
    if (triple_count){
        result->hand_combination = 3;
        result->tie_breaker = triple_value;
        return;
    }

    //2 pairs
    if (pair_count > 1){
        int i = 12;
        int low_pair = -1;
        int high_pair = -1;
        while (i >= 0 && (low_pair == -1 || high_pair == -1)){
            if (numbers_found[i] == 2){
                if (high_pair == -1)
                    high_pair = i;
                else
                    low_pair = i;
            }
            i--;
        }
        result->hand_combination = 2;
        result->tie_breaker = high_pair;
        result->secondary = low_pair;
        result->high_card = get_high_card(hand, len, high_pair, low_pair);
        return;

    }

    //1 pair
    if (pair_count){
        int pair_value = 0;
        for (int i = 0; i < 13; i++){
            if (numbers_found[i] == 2)
                pair_value = i;
        }
        result->hand_combination = 1;
        result->tie_breaker = pair_value;
        result->high_card = get_high_card(hand, len, pair_value, -1);
        return;
    }

    //last case, High card
    result->high_card = get_high_card(hand, len, -1, -1);
    return;

}


//fucntions to generate the bets of the A"I"
//uses the values from hand_estimations to create a bet value based on the round number and the hand (see initialize_hand_estimations)
//this value is then scaled based on the base blind
//Heavy rng component in the AIs behavior, to prevent the player from predicting its actions
int get_ai_bet(int hand_combination, int bank, int blind, int betting_round, int ai_bluff){
    //if bluff is active, it will bet as if it had a stronger hand than it has
    if (hand_combination < 5 && ai_bluff)
        hand_combination += 4;
    int base_bet = hand_estimations[betting_round][hand_combination];
    //hand_estimations goes from 0 to 100, then mapped to a value between 0 and 5*blind
    base_bet *= (5 * blind);
    base_bet /= 100;
    //random value added in a range between -blind and blind
    base_bet += (rand() % (2 * blind + 1) - blind);
    //the final bet has to be at least half of the blind, but never more than the AI has in bank
    return min_int(bank, max_int(base_bet, 0.5 * blind));
}



//Function to determine if the AI accepts a raise, 0 to refuse and 1 to accept
//Generates a number between 0 and 99 and a comparaison value
//If the comparaison value is greater than the random value, return 1, else 0
//comparaison value reduced based on the blind amount
//the minimum comparaison value e and its reduction rate depend
//on how strong the hand is
//raise ratio is : (raise value / blind)
int get_ai_raise(int hand_combination, int round, int raise_ratio){
    //lowest possible floor is 30, and can be up to 100
    int odd_floor = 30 + hand_estimations[round][hand_combination] * 7 / 10;
    //scale_down is how much the comparaison value is reduced per raise ratio
    //scale_down è from 0 to 10 depending on the hand
    int scale_down = 12 - hand_estimations[round][hand_combination] / 10;
    //comparaison value is max(floor, value obtained with the scale)
    return max_int(odd_floor, 100 - scale_down * raise_ratio) > rand() % 100;
}


//function that prints a text string expressing the hand, example : 2 Pairrs of Jack and 7, with high card Ace
void print_hand_string(struct hand_value * hand_eval){

    char *card_value[13];
    char *card_symbols[4];
    card_value[0] = "2";
    card_value[1] = "3";
    card_value[2] = "4";
    card_value[3] = "5";
    card_value[4] = "6";
    card_value[5] = "7";
    card_value[6] = "8";
    card_value[7] = "9";
    card_value[8] = "10";
    card_value[9] = "Jack";
    card_value[10] = "Queen";
    card_value[11] = "King";
    card_value[12] = "Ace";
    card_symbols[0] = "Hearts";
    card_symbols[1] = "Clubs";
    card_symbols[2] = "Diamonds";
    card_symbols[3] = "Spades";

    switch (hand_eval->hand_combination){
        case 0:
            printf("High card %s", card_value[hand_eval->high_card]);
            break;
        case 1:
            printf("Pair of %ss with high card %s", card_value[hand_eval->tie_breaker], card_value[hand_eval->high_card]);
            break;
        case 2:
            printf("2 Pairs of %s and %s with high card %s", card_value[hand_eval->tie_breaker], card_value[hand_eval->secondary], card_value[hand_eval->high_card]);
            break;
        case 3:
            printf("Three of a kind with %ss", card_value[hand_eval->tie_breaker]);
            break;
        case 4:
            printf("Straight from %s to %s", card_value[hand_eval->tie_breaker - 4], card_value[hand_eval->tie_breaker]);
            break;
        case 5:
            printf("Flush of %s", card_symbols[hand_eval->tie_breaker]);
            break;
        case 6:
            printf("Full House with a triple of %ss", card_value[hand_eval->tie_breaker]);
            break;
        case 7:
            printf("Four of a kind with %ss and high card %s", card_value[hand_eval->tie_breaker], card_value[hand_eval->high_card]);
            break;
        case 8:
            printf("Straight Flush from %s to %s", card_value[hand_eval->tie_breaker - 4], card_value[hand_eval->tie_breaker]);
            break;
        case 9:
            printf("Royal Flush!");
            break;
    }
}





void initialize_hand_estimations(){

    //first index is the round indication/number of revealed cards : 0 = round 1 / 3 cards revealed, 1 = round 2 / 4 cards revealed, ...
    //second number is the poker hand combination : 0 = high card, 1 = pair, 2 = 2 pairs, ...

    //hand estimation goes form 0 to 100, 0 is the worst and 100 is the top
    //these values are made up and not based on poker hand statistics of any kind
    hand_estimations[0][0] = 25;
    hand_estimations[0][1] = 60;
    hand_estimations[0][2] = 0;
    hand_estimations[0][3] = 0;
    hand_estimations[0][4] = 0;
    hand_estimations[0][5] = 0;
    hand_estimations[0][6] = 0;
    hand_estimations[0][7] = 0;
    hand_estimations[0][8] = 0;
    hand_estimations[0][9] = 0;

    hand_estimations[1][0] = 5;
    hand_estimations[1][1] = 15;
    hand_estimations[1][2] = 24;
    hand_estimations[1][3] = 28;
    hand_estimations[1][4] = 44;
    hand_estimations[1][5] = 61;
    hand_estimations[1][6] = 76;
    hand_estimations[1][7] = 88;
    hand_estimations[1][8] = 96;
    hand_estimations[1][9] = 100;

    hand_estimations[2][0] = 3;
    hand_estimations[2][1] = 10;
    hand_estimations[2][2] = 22;
    hand_estimations[2][3] = 24;
    hand_estimations[2][4] = 40;
    hand_estimations[2][5] = 58;
    hand_estimations[2][6] = 72;
    hand_estimations[2][7] = 86;
    hand_estimations[2][8] = 95;
    hand_estimations[2][9] = 100;
}


//basic min/max functions

int min_int(int a, int b){
    if (a >= b)
        return b;
    return a;
}

int max_int(int a, int b){
    if (a >= b)
        return a;
    return b;
}



//insertion sort algorithm, inefficient for long lists but easy to implement
void sort_list(int * arr, int len){
    //every element is always below 100
    for (int i = 0; i < len; i++){
        int smallest = 100;
        int smallest_index = 0;
        int temp;
        for (int j = i; j < len; j++){
            if (arr[j] < smallest){
                smallest = arr[j];
                smallest_index = j;
            }
        }
        temp = arr[i];
        arr[i] = arr[smallest_index];
        arr[smallest_index] = temp;
    }
}


void set_zero_list(int *arr, int len){
    for (int i = 0; i < len; i++)
        arr[i] = 0;
}


//function to find high-card, ignoring 0 or 1 or 2 values
//(in case of pair, ToaK or FoaK 1 value is ignored, for two-pair two values are ignored)
int get_high_card(int *hand, int len, int first_ignore, int second_ignore){
    int highest = 0;
    for (int i = 0; i < len; i++){
        if (hand[i]%13 > highest && hand[i]%13 != first_ignore && hand[i]%13 != second_ignore)
            highest = hand[i]%13;
    }
    return highest;
}


//debug only, prints a list
void print_list(int * arr, int len){
    printf("\n[");
    for (int i = 0; i < len; i++){
        printf("%d", arr[i]);
        if (i != len - 1)
            printf(" ,");
    }
    printf("]\n");
}

