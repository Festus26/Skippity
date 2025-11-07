/*
BLM1031 - Final Project
    Semih Yıldırım
       23011035
2023-2024 Spring Semester

Video Link:
https://youtu.be/oqnFqhbfJzw
*/

//Skippity Game
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define EXIT_CODE -1
#define SKIP_CODE -2

// Macros for ANSI color codes
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Priority to pick the least exixting skipper (0 if no priority)
#define PRIORITY_OF_LEAST 100

// Struct to hold player information
typedef struct PLAYER{
    char skipperCounts[5];
    int leastCountIndex;
    char *name;
}PLAYER;

// Struct to hold game session information
typedef struct GAME{
    PLAYER p1;
    PLAYER p2; 
    char **table;
    char size;
    char gameType;
    char turn;
}GAME;

// Function prototypes
char** createTable(char *);
void printTable(char **, char);
void placeSkippers(char **, char);
char selectGameType();
void playGame(GAME, int);
void moveSkipper(char **, PLAYER *, GAME *);
char checkRemainingMoves(char **, char);
void endGame(PLAYER, PLAYER);
int checkValidMoves(char **, char, int *);
void computerMoveSkipper(char **, PLAYER *, GAME *);
int getInput(GAME *);
void exitGame(GAME);
char *getPlayerName(char);
void transferTable(char **, char **, char);
void transferPlayer(PLAYER *, PLAYER *);
void printPlayerStats(PLAYER, PLAYER);
void saveGame(const GAME *);
int loadGame(GAME *);
void savePlayer(FILE *, const PLAYER *);
void loadPlayer(FILE *, PLAYER *);
void validateGame(const GAME *);
void checkAllocation(void *);
void countCollectibleSkipper(GAME, int *, int, int[4], PLAYER *);
int selectWay(GAME, int *, int *, PLAYER *);
void pickColor(char);
char findLeastCount(PLAYER *);

int main()
{
    GAME game;
    int isLoaded = 0;
    srand(time(NULL));

    // Create new game or load existing game
    printf("1. New Game\n2. Load Game\nEnter your choice:");
    if(getInput(NULL) == 2){
        if(loadGame(&game)){ // Load game if it exists
            printf("Game loaded\n");
            isLoaded = 1;
        }
        else{
            printf("Game could not be loaded\n New game starting\n");
            game.gameType = selectGameType();
            game.table = createTable(&game.size);   // Create table
            placeSkippers(game.table, game.size);   // Place skippers on the table
        }
    }
    else{
        game.gameType = selectGameType();
        game.table = createTable(&game.size);
        placeSkippers(game.table, game.size);
    }

    playGame(game, isLoaded);   // Start the game
    return 0;
}

// Create table for the game
char** createTable(char *realSize)
{
    int size = 0,i;
    do{
        printf("Enter the size of the game board:");
        scanf("%d", &size);
        if(size > 20)
            printf("Size must be less than or equal to 20\n");
        else if(size < 5)
            printf("Size must be greater than 4\n");
    }
    while(size > 20 || size < 5);
    *realSize = size;
    char **table = (char **)malloc(size * sizeof(char *));
    checkAllocation(table);
    for (i = 0; i < size; i++){
        table[i] = (char *)malloc(size * sizeof(char));
        checkAllocation(table[i]);
    }
    return table;
}

// Print table
void printTable(char **table, char size)
{
    int i;
    for (i = 0; i <= size + 1; i++){
        int j;
        for (j = 0; j <= size + 1; j++){
            if(i == 0 || i == size+1)
                if(j == 0 || j == size+1)
                    printf("+ ");
                else
                    printf("%d ",j % 10);
            else if(j == 0 || j == size+1) 
                printf("%d ", i % 10);
            else{
                pickColor(table[i-1][j-1]);
                printf("%c ", table[i-1][j-1]);
                printf(ANSI_COLOR_RESET);
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Place skippers on the table
void placeSkippers(char **table, char size)
{
    char totalSkippers[5], skipperCount = size * size / 5;

    // Find max number of each skipper
    int i;
    for (i = 0; i < 5; i++)
        totalSkippers[i] = skipperCount+1;
    
    for (i = 0; i < size; i++){
        int j;
        for (j = 0; j < size; j++){
            int skipperType = 0;
            do{
                skipperType = rand() % 5;
            }
            while(totalSkippers[skipperType] == 0);
            totalSkippers[skipperType]--;
            table[i][j] = 'A' + skipperType;
        }
    }
    int middle = size / 2;
    if(size % 2 == 0){  // Make the middle 4 cells empty if the size is even
        table[middle][middle] = ' ';
        table[(middle)-1][(middle)-1] = ' ';
        table[(middle)-1][middle] = ' ';
        table[middle][(middle)-1] = ' ';
    }
    else // Make the middle cell empty if the size is odd
        table[middle][middle] = ' ';
    printTable(table, size);
}
char selectGameType()
{
    char gameType;
    do{
        printf("Enter the game type\n1. PvP\n2. PvE\n3. EvE\nYour Choice:");
        scanf(" %c", &gameType);
        if(gameType != '1' && gameType != '2' && gameType != '3')
            printf("Invalid game type\n");
    }
    while(gameType != '1' && gameType != '2' && gameType != '3');
    return gameType;
}
char *getPlayerName(char player)
{
    char *name = (char *)malloc(30 * sizeof(char));
    checkAllocation(name);
    printf("Enter the name of player %c:", player);
    scanf("%s", name);
    return name;
}
// Transfer table from oldTable to newTable (undo-redo)
void transferTable(char **oldTable, char **newTable, char size)
{
    int i, j;
    char **tempTable = (char **)malloc(size * sizeof(char *));
    checkAllocation(tempTable);
    for (i = 0; i < size; i++){
        tempTable[i] = (char *)malloc(size * sizeof(char));
        checkAllocation(tempTable[i]);
    }
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            tempTable[i][j] = newTable[i][j];
        }
    }
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            newTable[i][j] = oldTable[i][j];
        }
    }
    for (i = 0; i < size; i++)
        for(j = 0; j < size; j++)
            oldTable[i][j] = tempTable[i][j];
    // Free allocated memory
    for (i = 0; i < size; i++)
        free(tempTable[i]);
    free(tempTable);
}
// Transfer player information from oldPlayer to newPlayer (undo-redo)
void transferPlayer(PLAYER *oldPlayer, PLAYER *newPlayer)
{
    int i;
    PLAYER tempPlayer;
    for(i = 0; i < 5; i++){
        tempPlayer.skipperCounts[i] = newPlayer->skipperCounts[i];
    }
    tempPlayer.leastCountIndex = newPlayer->leastCountIndex;
    tempPlayer.name = newPlayer->name;
    for(i = 0; i < 5; i++){
        newPlayer->skipperCounts[i] = oldPlayer->skipperCounts[i];
    }
    newPlayer->leastCountIndex = oldPlayer->leastCountIndex;
    newPlayer->name = oldPlayer->name;
    for(i = 0; i < 5; i++){
        oldPlayer->skipperCounts[i] = tempPlayer.skipperCounts[i];
    }
    oldPlayer->leastCountIndex = tempPlayer.leastCountIndex;
    oldPlayer->name = tempPlayer.name;
}
void playGame(GAME game, int isLoaded)
{
    printf("Game started\n");
    // Initialize players
    PLAYER *player1, *player2;
    player1 = &game.p1;
    player2 = &game.p2;
    if(isLoaded == 0){
        if(game.gameType == '1'){
            player1->name = getPlayerName('1');
            player2->name = getPlayerName('2');
        }
        else if(game.gameType == '2'){
            player1->name = getPlayerName('1');
            player2->name = "Computer";     // Computer is player 2
        }
        else if(game.gameType == '3'){
            player1->name = "Computer 1";   // Computer 1 is player 1
            player2->name = "Computer 2";   // Computer 2 is player 2
        }
        int i;
        for(i = 0; i < 5; i++)
            player1->skipperCounts[i] = player2->skipperCounts[i] = 0;
        player1->leastCountIndex = player2->leastCountIndex = 0;
        game.turn = 1;
    }
    printf("{Enter -1 to exit the game}\n{Enter -2 to skip the turn}\n");

    // Start the game loop
    while(checkRemainingMoves(game.table, game.size)){
        printPlayerStats(game.p1, *player2);
        printTable(game.table, game.size);
        if(game.turn == 1 && ((game.gameType == '1') || (game.gameType == '2'))){
            moveSkipper(game.table, player1, &game); // Player 1's turn
            game.turn = 2;
        }
        else if(game.turn == 1 && ((game.gameType == '3'))){
            computerMoveSkipper(game.table, player1, &game); // Computer 1's turn
            game.turn = 2;
        }
        else if(game.turn == 2 && (game.gameType == '1')){
            moveSkipper(game.table, player2, &game); // Player 2's turn
            game.turn = 1;
        }
        else if(game.turn == 2 && ((game.gameType == '2')||(game.gameType == '3'))){
            computerMoveSkipper(game.table, player2, &game); // Computer 2's turn
            game.turn = 1;
        }
    }
    // Free allocated memory
    int i;
    for (i = 0; i < game.size; i++)
        free(game.table[i]);
    free(game.table);

    endGame(*player1, *player2);    // End the game and declare the winner
}

// Exit the game and save if needed
void exitGame(GAME game){
    int i;
    do{
        printf("Dou you want to save before exit? (1:Yes, 0:No):");
        i = getInput(NULL);
    }
    while(i != 0 && i != 1);
    if(i == 1){
        saveGame(&game);
        printf("Game saved\n");
    }
    printf("Game ended\n");
    exit(0);
}
int getInput(GAME *game){
    int input;
    scanf("%d", &input);
    if(input == EXIT_CODE && game != NULL)
        exitGame(*game);
    return input;
}

// Pick color according to the skipper type
void pickColor(char c){
    switch (c)
    {
    case 'A':
        printf(ANSI_COLOR_RED);
        break;
    case 'B':
        printf(ANSI_COLOR_GREEN);
        break;
    case 'C':
        printf(ANSI_COLOR_YELLOW);
        break;
    case 'D':
        printf(ANSI_COLOR_BLUE);
        break;
    case 'E':
        printf(ANSI_COLOR_MAGENTA);
        break;
    default:
        printf(ANSI_COLOR_RESET);
        break;
    }
}

// Print player stats
void printPlayerStats(PLAYER player1, PLAYER player2){
    int i;
    printf("%s: ", player1.name);
    for(i = 0; i < 5; i++){
        pickColor('A'+i), printf("%c: %d ", 'A'+i, player1.skipperCounts[i]);   // Print player 1 skipper counts
        printf(ANSI_COLOR_RESET);
    }
    printf("\n");
    printf("%s: ", player2.name);
    for(i = 0; i < 5; i++){
        pickColor('A'+i), printf("%c: %d ", 'A'+i, player2.skipperCounts[i]);   // Print player 2 skipper counts
        printf(ANSI_COLOR_RESET);
    }
    printf("\n");
}

// Count the number of collectible skippers for a given position
void countCollectibleSkipper(GAME game, int *skipper, int oldWay, int *count, PLAYER *player){
    char leastChar = findLeastCount(player);    // Finds the least existing skipper of the player
    int counts[4] = {0, 0, 0, 0};
    if(skipper[0]-2 >= 0 && game.table[skipper[0]-1][skipper[1]] != ' ' && game.table[skipper[0]-2][skipper[1]] == ' ' && oldWay != 2){
        counts[0]++;
        if(game.table[skipper[0]-1][skipper[1]] == leastChar) 
            counts[0] += PRIORITY_OF_LEAST;     // If the the skipper that being collected is the least skipper of player prioritise it
        countCollectibleSkipper(game, (int[]){skipper[0]-2, skipper[1]}, 1, &counts[0], player);    // Check the chain moves and count collectible skippers   
    }
    if(skipper[0]+2 < game.size && game.table[skipper[0]+1][skipper[1]] != ' ' && game.table[skipper[0]+2][skipper[1]] == ' ' && oldWay != 1){
        counts[1]++;
        if(game.table[skipper[0]+1][skipper[1]] == leastChar)
            counts[1] += PRIORITY_OF_LEAST;
        countCollectibleSkipper(game, (int[]){skipper[0]+2, skipper[1]}, 2, &counts[1], player);
    }
    if(skipper[1]-2 >= 0 && game.table[skipper[0]][skipper[1]-1] != ' ' && game.table[skipper[0]][skipper[1]-2] == ' ' && oldWay != 4){
        counts[2]++;
        if(game.table[skipper[0]][skipper[1]-1] == leastChar)
            counts[2] += PRIORITY_OF_LEAST;
        countCollectibleSkipper(game, (int[]){skipper[0], skipper[1]-2}, 3, &counts[2], player);
    }
    if(skipper[1]+2 < game.size && game.table[skipper[0]][skipper[1]+1] != ' ' && game.table[skipper[0]][skipper[1]+2] == ' '  && oldWay != 3){
        counts[3]++;
        if(game.table[skipper[0]][skipper[1]+1] == leastChar)
            counts[3] += PRIORITY_OF_LEAST;
        countCollectibleSkipper(game, (int[]){skipper[0], skipper[1]+2}, 4, &counts[3], player);
    }

    // Find the max skipper collectible path
    int max = 0, i;
    for(i = 0; i < 4; i++){
        if(counts[i] > counts[max])
            max = i;
    }
    *count += counts[max];
}

// Selects the way to move 
int selectWay(GAME game, int *skipper, int *wayToSelect, PLAYER *player){
    char leastChar = findLeastCount(player);    // Finds the least existing skipper of the player
    int counts[4] = {0, 0, 0, 0};
    // Check every possible way to collect
    if(skipper[0]-2 >= 0 && game.table[skipper[0]-1][skipper[1]] != ' ' && game.table[skipper[0]-2][skipper[1]] == ' '){
        if(game.table[skipper[0]-1][skipper[1]] == leastChar)
            counts[0] += PRIORITY_OF_LEAST;     // If the the skipper that being collected is the least skipper of player prioritise it
        counts[0]++;
        countCollectibleSkipper(game, (int[]){skipper[0]-2, skipper[1]}, 1, &counts[0], player);    // Check the chain moves and count collectible skippers
    }
    if(skipper[0]+2 < game.size && game.table[skipper[0]+1][skipper[1]] != ' ' && game.table[skipper[0]+2][skipper[1]] == ' '){
        if(game.table[skipper[0]+1][skipper[1]] == leastChar)
            counts[1] += PRIORITY_OF_LEAST;         
        counts[1]++;
        countCollectibleSkipper(game, (int[]){skipper[0]+2, skipper[1]}, 2, &counts[1], player);
    }
    if(skipper[1]-2 >= 0 && game.table[skipper[0]][skipper[1]-1] != ' ' && game.table[skipper[0]][skipper[1]-2] == ' '){
        if(game.table[skipper[0]][skipper[1]-1] == leastChar)          
            counts[2] += PRIORITY_OF_LEAST;
        counts[2]++;
        countCollectibleSkipper(game, (int[]){skipper[0], skipper[1]-2}, 3, &counts[2], player);
    }
    if(skipper[1]+2 < game.size && game.table[skipper[0]][skipper[1]+1] != ' ' && game.table[skipper[0]][skipper[1]+2] == ' '){
        if(game.table[skipper[0]][skipper[1]+1] == leastChar)
            counts[3] += PRIORITY_OF_LEAST;
        counts[3]++;
        countCollectibleSkipper(game, (int[]){skipper[0], skipper[1]+2}, 4, &counts[3], player);
    }

    // Find the max collectible first move
    int max = 0, i;
    for(i = 0; i < 4; i++){
        if(counts[i] > counts[max])
            max = i;
    }
    *wayToSelect = max + 1; // Set the way to go for checked skipper
    return counts[max];
}

// Move logic of Computer
void computerMoveSkipper(char **table, PLAYER *player, GAME *game)
{
    int skipper[2], movePos[2], middlePos[2], i, j, isSelected = 0, isMoved = 0, wayToSelect = 0, realWay;
    // Move the skipper until there is no valid move with the selected skipper
    do{
        isSelected = 0;
        while(isSelected == 0){
            printf("Turn of %s\n", player->name);
            // Select the skipper to move if not selected
            if(isMoved == 0){
                int max = 0;
                for(i = 0; i < game->size; i++){
                    for(j = 0; j < game->size; j++){
                        if(table[i][j] != ' '){
                            int count = selectWay(*game, (int[]){i, j}, &wayToSelect, player);  // Calculate most collectible skippers for i and j
                            // Choose the move with the most collectible skipper
                            if(count > max){
                                max = count;
                                realWay = wayToSelect;
                                skipper[0] = i;
                                skipper[1] = j;
                            }
                        }
                    }
                }
            }
            else{
                // If the skipper is already selected, select the way to move
                selectWay(*game, skipper, &realWay, player);
            }
            printf("Skipper %d-%d is selected\n", skipper[0]+1, skipper[1]+1);
            printf("Enter the row and column of the skipper to move:");
            printf("Way to select: %d\n", realWay);
            // Implement the move according to the selected way
            switch(realWay){
                case 1:
                    movePos[0] = skipper[0]-2;
                    movePos[1] = skipper[1];
                    break;
                case 2:
                    movePos[0] = skipper[0]+2;
                    movePos[1] = skipper[1];
                    break;
                case 3:
                    movePos[0] = skipper[0];
                    movePos[1] = skipper[1]-2;
                    break;
                case 4:
                    movePos[0] = skipper[0];
                    movePos[1] = skipper[1]+2;
                    break;
            }
            printf("Moving to %d-%d\n", movePos[0]+1, movePos[1]+1);
            middlePos[0] = (skipper[0] + movePos[0]) / 2;
            middlePos[1] = (skipper[1] + movePos[1]) / 2;

            // Check if the move is valid
            if(movePos[0] < 0 || movePos[0] >= game->size || movePos[1] < 0 || movePos[1] >= game->size || table[movePos[0]][movePos[1]] != ' ' || table[middlePos[0]][middlePos[1]] == ' '){
                printf("Invalid move\n");
                exitGame(*game);
            }
            else{
                isSelected = 1;
            }
        }
        // Move the skipper, update the table and player stats
        player->skipperCounts[table[middlePos[0]][middlePos[1]] - 'A']++;
        table[movePos[0]][movePos[1]] = table[skipper[0]][skipper[1]];
        table[skipper[0]][skipper[1]] = ' ';
        table[middlePos[0]][middlePos[1]] = ' ';
        printPlayerStats(game->p1, game->p2);
        printTable(table, game->size);
        isMoved = 1;
        for(i = 0; i < 2; i++){
            skipper[i] = movePos[i];
        }
    }
    while(checkValidMoves(table, game->size, movePos) != 0);   
}
void moveSkipper(char **table, PLAYER *player, GAME *game)
{
    int skipper[2], movePos[2], middlePos[2], i, j, isSelected = 0, isMoved = 0, preIsModved = 0;

    // Create temporary variables to store the table and player information
    char **tempTable = (char **)malloc(game->size * sizeof(char *));
    checkAllocation(tempTable);
    for (i = 0; i < game->size; i++){
        tempTable[i] = (char *)malloc(game->size * sizeof(char));
        checkAllocation(tempTable[i]);
    }
    PLAYER tempPlayer;
    tempPlayer.name = player->name;
    for(i = 0; i < 5; i++){
        tempPlayer.skipperCounts[i] = player->skipperCounts[i];
    }
    tempPlayer.leastCountIndex = player->leastCountIndex;
    tempPlayer.name = player->name;

    // Move the skipper until there is no valid move with the selected skipper or skipped turn
    do{
        isSelected = 0;
        while(isSelected == 0){
            printf("Turn of %s\n", player->name);

            // Select valid skipper to move
            if(isMoved == 0){
                do{
                    printf("Enter the row and column of the skipper:");
                    for (i = 0; i < 2; i++){
                        skipper[i] = getInput(game)-1;
                        if(skipper[i] == SKIP_CODE - 1)
                            return;
                    }
                    if(skipper[0] < 0 || skipper[0] >= game->size || skipper[1] < 0 || skipper[1] >= game->size || table[skipper[0]][skipper[1]] == ' ' || checkValidMoves(table, game->size, skipper) == 0)
                        printf("Invalid skipper\n");
                }
                while(skipper[0] < 0 || skipper[0] >= game->size || skipper[1] < 0 || skipper[1] >= game->size || table[skipper[0]][skipper[1]] == ' ' || checkValidMoves(table, game->size, skipper) == 0);
            }
            printf("\nSelected skippker:[%d][%d]\n", skipper[0]+1, skipper[1]+1);

            // Select the way to move
            printf("Enter the row and column of the skipper to move:");
            for (i = 0; i < 2; i++){
                movePos[i] = getInput(game)-1;
                if(movePos[i] == SKIP_CODE - 1)
                    return;
            }
            middlePos[0] = (skipper[0] + movePos[0]) / 2;
            middlePos[1] = (skipper[1] + movePos[1]) / 2;

            // Check if the move is valid
            if(movePos[0] < 0 || movePos[0] >= game->size || movePos[1] < 0 || movePos[1] >= game->size || table[movePos[0]][movePos[1]] != ' ' || table[middlePos[0]][middlePos[1]] == ' '){
                printf("Invalid move\n");
            }
            else{
                isSelected = 1;
            }
        }
        for(i = 0; i < game->size; i++){
            for(j = 0; j < game->size; j++){
                tempTable[i][j] = table[i][j];
            }
        }

        // Move the skipper, update the table and player stats
        player->skipperCounts[table[middlePos[0]][middlePos[1]] - 'A']++;
        table[movePos[0]][movePos[1]] = table[skipper[0]][skipper[1]];
        table[skipper[0]][skipper[1]] = ' ';
        table[middlePos[0]][middlePos[1]] = ' ';
        printPlayerStats(game->p1, game->p2);
        printTable(table, game->size);

        preIsModved = isMoved;
        isMoved = 1;

        int tempMovePos[2];
        for(i = 0; i < 2; i++){
            tempMovePos[i] = movePos[i];
        }
        int redo;
        int undo;
        // Undo-Redo logic
        do {
            redo = 0;
            undo = 0;
            printf("Do you want to undo the move? (1:Yes, 0:No):");
            undo = getInput(game);
            if(undo == 1){
                transferTable(tempTable, table, game->size);    // Transfer the table to the previous state
                transferPlayer(&tempPlayer, player);            // Transfer the player information to the previous state
                printf("\n");
                printPlayerStats(game->p1, game->p2);
                printTable(table, game->size);
                isMoved = preIsModved;
                for(i = 0; i < 2; i++){
                    movePos[i] = skipper[i];
                }
                printf("Do you want to redo the move? (1:Yes, 0:No):");
                redo = getInput(game);
                // Redo the move if needed 
                if(redo == 1){
                    transferTable(tempTable, table, game->size);
                    transferPlayer(&tempPlayer, player);
                    printf("\n");
                    printPlayerStats(game->p1, game->p2);
                    printTable(table, game->size);
                    movePos[0] = tempMovePos[0];
                    movePos[1] = tempMovePos[1];
                    isMoved = 1;
                }
            }
        } while (redo == 1 );

        for(i = 0; i < 2; i++)
            skipper[i] = movePos[i];
        for(i = 0; i < 5; i++){
            tempPlayer.skipperCounts[i] = player->skipperCounts[i];
    }
    }
    while(checkValidMoves(table, game->size, movePos) != 0 || isMoved == 0);
    //Free Aloocated Memory
    for (i = 0; i < game->size; i++)
        free(tempTable[i]);
    free(tempTable); 
}

// Check if there is a valid move for the selected skipper  
int checkValidMoves(char **table, char size, int *skipper)
{
    if(skipper[0]-2 >= 0 && table[skipper[0]-1][skipper[1]] != ' ' && table[skipper[0]-2][skipper[1]] == ' ')
        return 1;
    else if(skipper[0]+2 < size && table[skipper[0]+1][skipper[1]] != ' ' && table[skipper[0]+2][skipper[1]] == ' ')
        return 2;
    else if(skipper[1]-2 >= 0 && table[skipper[0]][skipper[1]-1] != ' ' && table[skipper[0]][skipper[1]-2] == ' ')
        return 3;
    else if(skipper[1]+2 < size && table[skipper[0]][skipper[1]+1] != ' ' && table[skipper[0]][skipper[1]+2] == ' ')
        return 4;
    else
        return 0;
}

// Check if there is a remaining move for the player
char checkRemainingMoves(char **table, char size){
    int i, j;
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            if(table[i][j] != ' '){
                if(checkValidMoves(table, size, (int[]){i, j}) != 0)
                    return 1;
            }
        }
    }
    return 0;
}

// Find the least count skipper of the player
char findLeastCount(PLAYER *player){
    int i = 0;
    player->leastCountIndex = 0;
    for(i = 0; i < 5; i++){
        if(player->skipperCounts[i] < player->skipperCounts[player->leastCountIndex])
            player->leastCountIndex = i;
    }
    return 'A'+player->leastCountIndex;
}

// End the game and declare the winner
void endGame(PLAYER player1, PLAYER player2)
{
    findLeastCount(&player1);
    findLeastCount(&player2);
    printf("%s: %d\n", player1.name, player1.skipperCounts[player1.leastCountIndex]);
    printf("%s: %d\n", player2.name, player2.skipperCounts[player2.leastCountIndex]);
    if(player1.skipperCounts[player1.leastCountIndex] > player2.skipperCounts[player2.leastCountIndex])
        printf("%s wins\n", player1.name);
    else if(player1.skipperCounts[player1.leastCountIndex] < player2.skipperCounts[player2.leastCountIndex])
        printf("%s wins\n", player2.name);
    else
        printf("Draw\n");
}

// Save player information to the file
void savePlayer(FILE *file, const PLAYER *player) {
    // Write skipperCounts and leastCount
    fwrite(player->skipperCounts, sizeof(char), 5, file);
    fwrite(&player->leastCountIndex, sizeof(char), 1, file);

    // Write length of name and then name itself
    if (player->name == NULL) {
        printf("Error: Player name is NULL\n");
        int nameLength = 0;
        fwrite(&nameLength, sizeof(int), 1, file);
    } else {
        int nameLength = strlen(player->name) + 1; // Including null terminator
        fwrite(&nameLength, sizeof(int), 1, file);
        fwrite(player->name, sizeof(char), nameLength, file);
    }
}

// Load player information from the file
void loadPlayer(FILE *file, PLAYER *player) {
    // Read skipperCounts and leastCount
    fread(player->skipperCounts, sizeof(char), 5, file);
    fread(&player->leastCountIndex, sizeof(char), 1, file);

    // Read length of name and then name itself
    int nameLength;
    fread(&nameLength, sizeof(int), 1, file);
    player->name = (char *)malloc(nameLength * sizeof(char));
    checkAllocation(player->name);
    fread(player->name, sizeof(char), nameLength, file);
}

// Validate the game structure
void validateGame(const GAME *game) {
    if (game->p1.name == NULL || game->p2.name == NULL) {
        printf("Error: One of the player names is NULL\n");
        return;
    }
    int i;
    for (i = 0; i < game->size; ++i) {
        if (game->table[i] == NULL) {
            printf("Error: Table row %d is NULL\n", i);
            return;
        }
    }
}

// Save the game to a file
void saveGame(const GAME *game) {
    validateGame(game);

    FILE *file = fopen("game_state.txt", "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Write players
    savePlayer(file, &game->p1);
    savePlayer(file, &game->p2);

    // Write size, gameType, and turn
    fwrite(&game->size, sizeof(char), 1, file);
    fwrite(&game->gameType, sizeof(char), 1, file);
    fwrite(&game->turn, sizeof(char), 1, file);

    // Write table
    int i;
    for (i = 0; i < game->size; ++i) {
        fwrite(game->table[i], sizeof(char), game->size, file);
    }

    fclose(file);
}

int loadGame(GAME *game) {
    FILE *file = fopen("game_state.txt", "rb");
    if (file == NULL) {
        perror("Error opening file for reading");
        return 0;
    }

    // Read players
    loadPlayer(file, &game->p1);
    loadPlayer(file, &game->p2);

    // Read size, gameType, and turn
    fread(&game->size, sizeof(char), 1, file);
    fread(&game->gameType, sizeof(char), 1, file);
    fread(&game->turn, sizeof(char), 1, file);

    // Allocate memory for table
    game->table = (char **)malloc(game->size * sizeof(char *));
    checkAllocation(game->table);
    int i;
    for (i = 0; i < game->size; ++i) {
        game->table[i] = (char *)malloc(game->size * sizeof(char));
        checkAllocation(game->table[i]);
    }

    // Read table
    for (i = 0; i < game->size; ++i) {
        fread(game->table[i], sizeof(char), game->size, file);
    }

    fclose(file);
    return 1;
}
// Check if the memory allocation is successful
void checkAllocation(void *ptr) {
    if (ptr == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
}