#include "display.h"





//function to print the 9 crds (2 player + 5 in common + 2 AI) + the banks / bets
//xy in the cards gets replaced by the actual value (see initialize_cards)
//cards_revealed is 0, 3, 4, 5
void display_game(int *player_hand, int *ai_hand, int player_bank, int ai_bank, int player_bet, int ai_bet, int cards_revealed){

    printf("\n\n\n\n\n\n");

    int show_ai_hand = 0;

    //The AI's hand is revelead only once all other cards are shown
    if (cards_revealed == 5)
        show_ai_hand = 1;

    //print AI hand
    print_hand(ai_hand, 2, cards_revealed, show_ai_hand);
    if (cards_revealed)
        printf("\t\t\t\t\t\tAI Bank: %d\t\tCurrent bet: %d\n\n", ai_bank, ai_bet);
    //at round 0 there are no bets
    else
        printf("\t\t\t\t\t\tAI Bank: %d\n\n", ai_bank);

    //print the cards in common
    print_hand(player_hand, 5, cards_revealed, 1);
    printf("\n");

    //print the player's hand
    print_hand(player_hand, 2, cards_revealed, 1);
    if (cards_revealed)
        printf("\t\t\t\t\t\tPlayer Bank: %d\t\tCurrent bet: %d\n\n", player_bank, player_bet);
    else
        printf("\t\t\t\t\t\tPlayer Bank: %d\n\n", player_bank);

    return;
}


void print_hand(int *hand, int hand_length, int cards_revealed, int show_all){
    //if the function has to print a private hand, it only looks at the first 2 elements of the list (elem 0 and 2 excluded)
    //if instead it prints the shared cards it looks at the last 5 (da 2 a 7 escluso)
    int upper = 2;
    int lower = 0;
    if (hand_length == 5){
        upper = 7;
        lower = 2;
    }
    //each card has a visual height of 12 lines, each iteration prints the nth line of every card the funtiuon has to print
    for (int i=0; i < 12; i++){
        for (int j=lower; j < upper; j++){
            //j is the index of array hand[], so if 3 shared cards have to be revealed it takes hand[2] to hand[2+3], since the first 2 are the private cards
            //show_all is always true except when the AI's hand is printed, since it is only shown on the final round
            if (cards_revealed + 2 > j && show_all)
            {
                char temp[16];
                //replace the top left 'xy'
                if(i == 2){
                    strcpy(temp, card_parts[0][i]);
                    //If it's a '10' it occupies 2 characters, otherwise only 1
                    if (hand[j]%13 == 8){
                        temp[2] = '1';
                        temp[3] = '0';
                    }
                    else{
                        temp[2] = card_value_initial[hand[j]%13];
                        temp[3] = ' ';
                    }
                    printf("    %s", temp);
                }
                //replace bottom right 'xy'
                else if(i == 10){
                    strcpy(temp, card_parts[0][i]);
                    //If it's a '10' it occupies 2 characters, otherwise only 1
                    if (hand[j]%13 == 8){
                        temp[12] = '1';
                        temp[13] = '0';
                    }
                    else{
                        temp[12] = ' ';
                        temp[13] = card_value_initial[hand[j]%13];
                    }
                    printf("    %s", temp);
                }
                else {
                    strcpy(temp, card_parts[hand[j]/13][i]);
                    printf("    %s", temp);
                }

            }
            //case where the card to print is face down
            else {
                printf("    %s", card_parts[BACK][i]);
            }
        }
        printf("\n");
    }
    return;
}



//template ASCII art for the cards
void initialize_cards(){

    card_parts[HEART][0] = " ______________ ";
    card_parts[HEART][1] = "/              \\";
    card_parts[HEART][2] = "| xy           |";
    card_parts[HEART][3] = "|    __  __    |";
    card_parts[HEART][4] = "|   /  \\/  \\   |";
    card_parts[HEART][5] = "|   \\      /   |";
    card_parts[HEART][6] = "|    \\    /    |";
    card_parts[HEART][7] = "|     \\  /     |";
    card_parts[HEART][8] = "|      \\/      |";
    card_parts[HEART][9] = "|              |";
    card_parts[HEART][10] = "|           xy |";
    card_parts[HEART][11] = "\\______________/";

    card_parts[CLUB][0] = " ______________ ";
    card_parts[CLUB][1] = "/              \\";
    card_parts[CLUB][2] = "| xy           |";
    card_parts[CLUB][3] = "|      __      |";
    card_parts[CLUB][4] = "|     /  \\     |";
    card_parts[CLUB][5] = "|   __\\  /__   |";
    card_parts[CLUB][6] = "|  /        \\  |";
    card_parts[CLUB][7] = "|  \\__/  \\__/  |";
    card_parts[CLUB][8] = "|     |__|     |";
    card_parts[CLUB][9] = "|              |";
    card_parts[CLUB][10] = "|           xy |";
    card_parts[CLUB][11] = "\\______________/";

    card_parts[DIAMOND][0] = " ______________ ";
    card_parts[DIAMOND][1] = "/              \\";
    card_parts[DIAMOND][2] = "| xy           |";
    card_parts[DIAMOND][3] = "|      /\\      |";
    card_parts[DIAMOND][4] = "|     /  \\     |";
    card_parts[DIAMOND][5] = "|    /    \\    |";
    card_parts[DIAMOND][6] = "|   (      )   |";
    card_parts[DIAMOND][7] = "|    \\    /    |";
    card_parts[DIAMOND][8] = "|     \\  /     |";
    card_parts[DIAMOND][9] = "|      \\/      |";
    card_parts[DIAMOND][10] = "|           xy |";
    card_parts[DIAMOND][11] = "\\______________/";

    card_parts[SPADE][0] = " ______________ ";
    card_parts[SPADE][1] = "/              \\";
    card_parts[SPADE][2] = "| xy           |";
    card_parts[SPADE][3] = "|      __      |";
    card_parts[SPADE][4] = "|     /  \\     |";
    card_parts[SPADE][5] = "|    /    \\    |";
    card_parts[SPADE][6] = "|   /      \\   |";
    card_parts[SPADE][7] = "|  (__    __)  |";
    card_parts[SPADE][8] = "|     |__|     |";
    card_parts[SPADE][9] = "|              |";
    card_parts[SPADE][10] = "|           xy |";
    card_parts[SPADE][11] = "\\______________/";

    card_parts[BACK][0] = " ______________ ";
    card_parts[BACK][1] = "/              \\";
    card_parts[BACK][2] = "| ############ |";
    card_parts[BACK][3] = "| #@@@@@@@@@@# |";
    card_parts[BACK][4] = "| #@########@# |";
    card_parts[BACK][5] = "| #@#@@@@@@#@# |";
    card_parts[BACK][6] = "| #@#@####@#@# |";
    card_parts[BACK][7] = "| #@#@@@@@@#@# |";
    card_parts[BACK][8] = "| #@########@# |";
    card_parts[BACK][9] = "| #@@@@@@@@@@# |";
    card_parts[BACK][10] = "| ############ |";
    card_parts[BACK][11] = "\\______________/";

    card_value_initial[0] = '2';
    card_value_initial[1] = '3';
    card_value_initial[2] = '4';
    card_value_initial[3] = '5';
    card_value_initial[4] = '6';
    card_value_initial[5] = '7';
    card_value_initial[6] = '8';
    card_value_initial[7] = '9';
    card_value_initial[8] = '%'; //this value is never directly printed since it becomes 10
    card_value_initial[9] = 'J';
    card_value_initial[10] = 'Q';
    card_value_initial[11] = 'K';
    card_value_initial[12] = 'A';
}
