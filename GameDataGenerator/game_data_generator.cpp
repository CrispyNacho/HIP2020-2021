#include <stdio.h>   /* printf */
#include <stdlib.h>  /* atoi */
#include <string>    /* string class */
#include <list>      /* list class */
#include <random>    /* random_device, mt19937, uniform_real_distribution */
#include <time.h>    /* clock */
#include <fstream>   /* ofstream class */

using namespace std;

// #define DEBUG // Uncomment this for debug prints

#define NUM_ARGS 3
#define NUM_STATS 3

#define MAX_NUM_TEAMS 30

#define MIN_WIP_RATIO 0.75
#define MAX_WIP_RATIO 1.25

#define MIN_RBI_RATIO 0.75
#define MAX_RBI_RATIO 1.25

#define MIN_WAR_RATIO 0.75
#define MAX_WAR_RATIO 1.25

#define GAME_STATS_FILENAME "game_stats.csv"
#define GAME_RESULTS_FILENAME "game_results.csv"

typedef struct {
    int   gameNum;  // Game Number
    int   homeCode; // Home Team Code
    int   awayCode; // Away Team Code
    float WIPRatio; // WIP Ratio (Home / Away)
    float RBIRatio; // RBI Ratio (Home / Away)
    float WARRatio; // WAR Ratio (Home / Away)
    bool  homeWin;  // Home Team Win or Loss
} GameData_t;

enum WinnerBias {
    NONE = 0,   // No Bias, flip a coin
    PREFER_WIP, // Prefer team with higher WIP
    PREFER_RBI, // Prefer team with higher RBI
    PREFER_WAR, // Prefer team with higher WAR
    PREFER_AVG  // Prefer team with higher average stat ratio
};

void generateBitmapImage(unsigned char *image, int height, int width, int pitch, const char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int width, int pitch, int paddingSize);
unsigned char* createBitmapInfoHeader(int height, int width);
void addDeformity(unsigned char *image, int height, int width, int pitch, int maxRadius, string fname);

void printUsage(char* progName)
{
    printf("USAGE: %s <number of teams> <number of times each team plays another> <winner bias>\n", progName);
    printf("    Winner Bias:\n");
    printf("        0 = None (random)\n");
    printf("        1 = Prefer Higher WIP Ratio\n");
    printf("        2 = Prefer Higher RBI Ratio\n");
    printf("        3 = Prefer Higher WAR Ratio\n");
    printf("        4 = Prefer Higher Avergae Ratio\n");
    return;
}

void writeGameDataFiles(std::list<GameData_t> & gameData) {
    std::ofstream gameStatsFile(GAME_STATS_FILENAME);
    std::ofstream gameResultsFile(GAME_RESULTS_FILENAME);

    int gameNum = 0;
    for(auto iter = gameData.begin(); iter != gameData.end(); iter++, gameNum++) {
        // Game Data Schema: Game #, Team code Home, Team code Away,  WIP Ratio Home to Away, RBI Ratio Home to Away, WAR Home to Away
        string homeAway = std::to_string(gameNum) + ',' + std::to_string(iter->homeCode) + ',' + std::to_string(iter->awayCode) + ',';
        string data = homeAway + std::to_string(iter->WIPRatio) + ',';
        data += std::to_string(iter->RBIRatio) + ',' + std::to_string(iter->WARRatio) + '\n';

        // Game Results Schema: Game #, Team code Home, Team code Away,  Home Team Win?
        string results = homeAway + (iter->homeWin == true ? "true" : "false") + '\n';

#ifdef DEBUG
        printf("Data = \"%s\n\"", data.c_str());
        printf("Results = \"%s\n\"", results.c_str());
#endif
        gameStatsFile << data;
        gameResultsFile << results;
    }

    gameStatsFile.close();
    gameResultsFile.close();
}

void generateGameData(std::list<GameData_t> & gameData, WinnerBias bias) {
    // Set up the random number generator for the WIP, RBI, and WAR values
    // Obtain a random number from hardware
    std::random_device rd;
    // Seed the generator
    std::mt19937 eng(rd());

    // Define the range the different ratios can be
    std::uniform_real_distribution<> WIPDistr(MIN_WIP_RATIO, MAX_WIP_RATIO);
    std::uniform_real_distribution<> RBIDistr(MIN_RBI_RATIO, MAX_RBI_RATIO);
    std::uniform_real_distribution<> WARDistr(MIN_WAR_RATIO, MAX_WAR_RATIO);

    std::binomial_distribution<> coinFlipper(1, 0.5); // Generate random numbers between 0 and 1 with 50% chance

    // TODO: Currently the ratios are all random and have no history. A possible
    // enhancment could be to have their ratios make sense over time
    for(auto iter = gameData.begin(); iter != gameData.end(); iter++) {
        iter->WIPRatio = WIPDistr(eng);
        iter->RBIRatio = RBIDistr(eng);
        iter->WARRatio = WARDistr(eng);

        float avg;
        switch(bias) {
            case PREFER_WIP: // Prefer team with higher WIP
                iter->homeWin = iter->WIPRatio > 1.0 ? true : false;
                break;
            case PREFER_RBI: // Prefer team with higher RBI
                iter->homeWin = iter->RBIRatio > 1.0 ? true : false;
                break;
            case PREFER_WAR: // Prefer team with higher WAR
                iter->homeWin = iter->WARRatio > 1.0 ? true : false;
                break;
            case PREFER_AVG: // Prefer team with higher average stat ratio
                avg = (iter->WARRatio + iter->RBIRatio + iter->WIPRatio) / NUM_STATS;
                iter->homeWin =  avg > 1.0 ? true : false;
                break;
            default: // No Bias, flip a coin
                iter->homeWin = coinFlipper(eng) == 1 ? true : false;
        }

#ifdef DEBUG
        printf("homeCode = %d, awayCode = %d\n", iter->homeCode, iter->awayCode);
        printf("WIPRatio = %f, RBIRatio = %f, WARRatio = %f\n", iter->WIPRatio, iter->RBIRatio, iter->WARRatio);
        printf("homeWin = %s\n\n", iter->homeWin == true ? "True" : "False");
#endif
    }
}

void determineTeamPermutations(int numTeams, int numGamesPerTeam, std::list<GameData_t> & gameData) {
    GameData_t newData;

    // Obtain a random number from hardware
    std::random_device rd;
    // Seed the generator
    std::mt19937 eng(rd());
    std::binomial_distribution<> coinFlipper(1, 0.5); // Generate random numbers between 0 and 1

    // Loop through the number of times each team should play every other team
    for(int i = 0; i < numGamesPerTeam; i++) {
        int currTeam = 0;
        // Iterate until each team has played every other team once
        while(currTeam != numTeams) {
            // Generate games of the current team playing the remaining teams
            for(int j = currTeam; j < numTeams - 1; j++) {
                int coinFlip = coinFlipper(eng); // Flip a coin to see who's the home team
                if(coinFlip) {
                    newData.homeCode = currTeam;
                    newData.awayCode = j + 1;
                }
                else {
                    newData.homeCode = j + 1;
                    newData.awayCode = currTeam;
                }
                gameData.push_back(newData);
            }
            currTeam++;
        }
    }
}

/***
 * main
 * Command Line Parameters -
 *   number of teams to generate data for
 *   number of times every team plays each other
 **/
int main(int argc, char* argv[])
{
    if(argc != NUM_ARGS + 1)
    {
        printf("ERROR: invalid number of command line parameters\n");
        printUsage(argv[0]);
        return -1;
    }

    clock_t tStart = clock();
    int numTeams = atoi(argv[1]);
    int numGamesPerTeam = atoi(argv[2]);
    WinnerBias bias = (WinnerBias)atoi(argv[3]);

    std::list<GameData_t> gameData;

    // Determine the possible team permutations based on the input arguments
    determineTeamPermutations(numTeams, numGamesPerTeam, gameData);

    // Generate game data for the different teams
    generateGameData(gameData, bias);

    // Write game data results to files
    writeGameDataFiles(gameData);

    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    return 0;
}
