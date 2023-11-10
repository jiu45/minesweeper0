#include <iostream>
#include <string>
#include <cstdlib> 
#include <time.h>
#include <chrono> 
#include <windows.h>     //Insert color
#include <sqlite3.h>     //Save data
#include <conio.h>       //_getch()
#include <iomanip>


using namespace std;

// Define maximum boxes

const int N = 32;

//Declare 1 box structure

struct box
{
    int attribute;   // 0 - not bomb, 1 - bomb
    int surrounded;  // tell the number of neighbour-boxes contain bomb
    int status;      // 0 - not opened, 1 - opened
    bool flagged;    // flag - true, not flag - false
};


// Create an array of boxes

box boxes[N][N];

// Functions needed

void setdefault();
void set_bombs(int bombs, int height, int width);
void update_surrounding(int y_located, int x_located);
int cal_3bd(int height, int width);
int draw(int height, int width, int game_mode, int hiscore, string cell);
string take_command(int height, int width, int thbd, int game_mode, int hiscore, string cell);
bool check_cell(string cmd, int height, int width);
string handleInput(string cell, int height, int width, int game_mode, int hiscore);
string move_with_arrow(string cell, int height, int width, int game_mode, int hiscore);
void open(int pos_x, int pos_y, int height, int width);
void reverse_open(int pos_x, int pos_y, int height, int width);
bool game_over(string cmd);
bool newgame();
bool save(int height, int width, int thbd, int time, int hiscore, int game_mode, int cases);
void showboard(int height, int width, int game_mode, int hiscore);
int receive_highscore(int game_mode);
bool manage_high_score(int score, int high_score, int game_mode);



//Added to change text color

string setcolor(unsigned short color)
{
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hcon, color);
    return "";
}



//MAIN

int main()
{

// Welcome user

    std::cout << endl;
    std::cout << "Welcome to Minsweeper on terminal :)" << endl;
    std::cout << "Press enter to continue";
    std::cin.get();
  
//Rules

    std::cout << std::endl;

    std::cout << "Rules:\n";

    std::cout << "To open: type in location with no spaces, row first(capital), column after and o  (eg. A05o)" << endl;
    std::cout << "To open: type in location with no spaces, row first(capital), column after and f  (eg. C16f)" << endl;
    std::cout << "To open: type in location with no spaces, row first(capital), column after and u  (eg. C06u)" << endl;
    std::cout << endl;
    std::cout << "When you're ready, press enter to continue" << endl;
    std::cin.get();
    std::cin.clear();    
    

//EXECUTES WHENEVER THE PROGRAM RUNS AND LOOPS IF is_new_game IS TRUE
    

    bool is_new_game = false;


    do
    {
        int board_height, board_width, bombs;
        int thbd;
        int saved_game_time = 0;
        int game_mode;
        
        //Set default 

        setdefault();

//Open data from last game if saved

        if (is_new_game == false)
        {
            sqlite3* db;
            sqlite3_stmt *st;
            int value = 0;
            int exit = 0;
            int s_height;
            int s_width;
            int s_thbd; 
            int s_mode;
            exit = sqlite3_open("minesdata.db", &db);

            
            string sql1 = "SELECT * FROM minesdata WHERE column = ? AND row = ?;";

            int rt = sqlite3_prepare_v2(db, sql1.c_str(), -1, &st, NULL);

            if (rt == SQLITE_OK)
            {
                sqlite3_bind_int(st, 1, value);
                sqlite3_bind_int(st, 2, value);
                sqlite3_step(st);

                s_height = sqlite3_column_int(st, 6);
                s_width = sqlite3_column_int(st, 7);
                s_thbd = sqlite3_column_int(st, 8);
                saved_game_time = sqlite3_column_int(st, 9);
                s_mode = sqlite3_column_int(st, 11);
                sqlite3_finalize(st);
            }

            

            //This can be executed when the program first run or new the last user did not save the process

            if (s_height == 0)
            {
                is_new_game = true;
                std::cout << "No records of last game, press enter to continue: ";
                std::cin.get();
                std::cout << endl;
            } 

            //If the data is saved succesfully
            //Assign the last game's data in to boxes

            if (s_height != 0)
            {
                
                board_height = s_height;
                board_width = s_width;
                thbd = s_thbd;
                game_mode = s_mode;
                is_new_game = false;        //change the value to skip the following part, go straight to play-time
                bombs = 0;

                std::cout << "Loading last record..." << endl;

                for (int i = 0; i < N; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        int rc = sqlite3_prepare_v2(db, sql1.c_str(), -1, &st, NULL);
                        if (rc == SQLITE_OK)
                        {
                            
                            sqlite3_bind_int(st, 1, i);
                            sqlite3_bind_int(st, 2, j);
                            sqlite3_step(st);
                            boxes[j][i].attribute = sqlite3_column_int(st, 2);
                            boxes[j][i].status = sqlite3_column_int(st, 3);
                            boxes[j][i].flagged = sqlite3_column_int(st, 4);
                            boxes[j][i].surrounded = sqlite3_column_int(st, 5);
                            if (boxes[j][i].attribute == 1)
                            {
                                bombs+= 1;
                            }

                            sqlite3_finalize(st);
                        }
                    }
                }
                

                std::cout << "Loaded. Press enter to continue: ";
                std::cin.get();
            }

            sqlite3_close(db);
        }

        
//Execute when new game is selected or the data of last game is not saved
//If the last game is saved, skip this part

        if (is_new_game)
        {
            // Choose level

            string str1;
            std::cout << "Please choose difficulty level:\n";
            std::cout << endl;
            std::cout << "A - Easy\n";          //mode 0
            std::cout << "B - Medium\n";        //mode 1
            std::cout << "C - Hard\n";          //mode 2
            std::cout << "M - Customize your board\n";       //mode 3
            std::cout << endl;
            

            do
            {
                
                std::cout << "Enter A or B or C or M: " << endl;
                std::cin >> str1;
                
                
            } while ((str1[0] != 'A' && str1[0] != 'B' && str1[0] != 'C' && str1[0] != 'M') || str1.size() > 1);

            std::cin.clear();

            std::cin.get();


            //SET UP FOR LEVEL
        

            switch (str1[0])
            {
            case 'A':

                board_height = 9;
                board_width = 9;
                bombs = 10;
                game_mode = 0;
                break;
            
            case 'B':
                board_height = 16;
                board_width = 16;
                bombs = 40;
                game_mode = 1;
                break;
            case 'C':
                board_height = 30;
                board_width = 16;
                bombs = 99;
                game_mode = 2;
                break;

            case 'M':
                game_mode = 3;
                std::cout << "Input the board's height (< 30): " << endl;
                std::cin >> board_height;
                std::cout << "Input the board's width (< 26): " << endl;
                std::cin >> board_width;
                std::cout << "Input number of bombs < " << board_height * board_width << " : " << endl;
                std::cin >> bombs;

            }

            //Set bombs

            set_bombs(bombs, board_height, board_width);   //mines are at location, be carefull !

            //Update number of bombs surrounding each box

            for (int i = 1; i <= board_height; i++)
            {
                for (int j = 1; j <= board_width; j++)
                {
                    if (boxes[j][i].attribute == 1)
                    {
                        continue;
                    }
                    update_surrounding(i, j);        //update to boxes.surrounded
                }
            }

            //Calculate 3bd

            thbd = cal_3bd(board_height, board_width);
        
        }


//PLAY-TIME

        int max = board_height * board_width - bombs;
        string cmd;
        bool win = false;
        bool show_board = true;
        bool start_timer = true;
        std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
        std::chrono::duration<long long, std::ratio<1, 1>> duration;
        int hi_score = receive_highscore(game_mode);
        string cell = "A01";      //Mark the first box of the game

        //Loop through the entire game

        while (true)
        {
            //If their is no more box to open -> win

            system("cls");

            if (draw(board_height, board_width, game_mode, hi_score, cell) >= max)
            {
                end = std::chrono::high_resolution_clock::now();        //if win -> timer stops
                win = true;
                std::cout << endl;
                std::cout << "You win !" << endl;
                std::cout << endl;
                std::cout << "Press enter to continue";
                std::cin.get();
                is_new_game = newgame();
                std::cout << endl;
                std::cout << "Press enter to show the board for last match: ";
                std::cin.get();
                break;
            }
        
            std::cout << endl;

            //Take command from user, return to check

            cmd = take_command(board_height, board_width, thbd, game_mode, hi_score, cell);

            //Update correct cell to mark
            cell = cmd;
            cell.resize(3);

            //Timer starts
            if (start_timer)
            {
                start = std::chrono::high_resolution_clock::now();
                start_timer = false;
            }

            //If new game is selected
            if (cmd == "new")
            {   
                end = std::chrono::high_resolution_clock::now();        //Timer stops
                is_new_game = true;
                break;
            }

            //If not save is selected
            if (cmd == "n")
            {
                end = std::chrono::high_resolution_clock::now();      //Timer stops
                is_new_game = false;
                break;
            }

            //If save is chosen
            if (cmd == "y")
            {
                end = std::chrono::high_resolution_clock::now();      //Timer stops
                is_new_game = false;
                show_board = false;        //Change the value to avoid show_board
                break;
            }

            //If clicked onto bomb
            if (game_over(cmd))
            {
                end = std::chrono::high_resolution_clock::now();        //Gsme-over -> timer stops
                std::cout << "Clicked onto bomb. Exploded." << endl;
                std::cout << endl;
                std::cout << "Game over" << endl;
                is_new_game = newgame();    // a small remind to revenge :)
                std::cout << endl;
                //If the player does not want to take revenge
                if (!is_new_game)
                {
                    std::cout << "Press enter to show board";
                }

                //Yeah...
                if (is_new_game)
                {
                    std::cout << "Enter to show the board for last match: ";
                }
                
                std::cin.get();
                break;
            }
    
        }

//END GAME, SHOW BOARD

        //Calculate the time span of this session

        duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);      
        int seconds = duration.count() + saved_game_time;

        //If the game is not saved

        if (show_board)
        {
            showboard(board_height, board_width, game_mode, hi_score);
            std::cout << endl;
            std::cout << "Game time: " << seconds << " seconds." << endl;
            std::cout << endl;
        
        }

        //If the game is saved

        if (!show_board)
        {
            save(board_height, board_width, thbd, seconds, -1, game_mode, 2);
        }

        //if win -> calculate points and save if points > highscore

        if (win)
        {
            int scores = (double) thbd / seconds * 1000;
            std::cout << endl;
            std::cout << "Score: " << scores << endl;
            std::cout << endl;
            
            if (manage_high_score(scores, hi_score, game_mode))
            {
                save(board_height, board_width, thbd, seconds, scores, game_mode, 3);
            }
            
        }
        
        std::cout << "Press enter to continue: ";
        std::cin.get();
        
    
    } while(is_new_game);


//Goodbye

    std::cout << "Press enter to log out";
    std::cin.get();

    std::cout << endl;

    std::cout << "Have you enjoyed :) ? \n";
    std::cout << endl;
    std::cout << "Follow ShiinaDidn'tBlush on ButtBook !" << endl;
    std::cout << endl;
    std::cout << "Come back anytime. I will always be here, in the land of dawn ! \n";
    std::cout << endl;

//Return

    return 0;

}


//SET ALL TO 0 AT THE BEGINNING OF THE GAME

void setdefault()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            boxes[j][i].attribute = 0;
            boxes[j][i].surrounded = 0;
            boxes[j][i].status = 0;
            boxes[j][i].flagged = false;
            
        }
    }
    return;
}


//SHOW RESULT AND UPDATE DATABASE TO BLANK AGAIN

void showboard(int height, int width, int game_mode, int hiscore)
{
    //Show board

    for (int i = 0; i <= height; i++)
    {
        for (int j = 0; j <= width; j++)
        {
            
            boxes[j][i].status = 1;
            
        }
    }


    draw(height, width, game_mode, hiscore, "A00");

    //Reset data

    int insert = 0;
 
    sqlite3* DB; 
    sqlite3_stmt *st;
    int exit = 0; 
    exit = sqlite3_open("minesdata.db", &DB);

    
    string sql3 = "UPDATE minesdata SET attribute = ?, status = ?, surrounded = ?, flagged = ?, height = ?, width = ?, thbd = ?, time = ? WHERE column = ? AND row = ?;"; 

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            int rc = sqlite3_prepare_v2(DB, sql3.c_str(), -1, &st, NULL);
            if (rc == SQLITE_OK)
            {
                
                sqlite3_bind_int(st, 1, insert);
                sqlite3_bind_int(st, 2, insert);
                sqlite3_bind_int(st, 3, insert);
                sqlite3_bind_int(st, 4, insert);
                sqlite3_bind_int(st, 5, insert);
                sqlite3_bind_int(st, 6, insert);
                sqlite3_bind_int(st, 7, insert);
                sqlite3_bind_int(st, 8, insert);
                sqlite3_bind_int(st, 9, i);
                sqlite3_bind_int(st, 10, j);
                sqlite3_step(st);
                sqlite3_finalize(st);
            }

            if (rc != SQLITE_OK)
            {
                std::cout << "fail" << endl;
            }
        }
    }
    

    sqlite3_close(DB); 

    return;
}


//SET BOMBS


void set_bombs(int bombs, int height, int width)
{
    int total = height * width;

    //2 dimensions boxes into 1 dimension

    //Generate an array of integer arr[0]=0, arr[1]=1,..

    struct coor_box
    {
        int x;
        int y;
    };
    
    coor_box cell[total];
    int arr[total];

    int count = 0;

    for (int i = 1; i <= height; i++)
    {
        for (int j = 1; j <= width; j++)
        {
            cell[count].x = j;
            cell[count].y = i;
            arr[count] = count;
            count++;     
        }
    }

    //Use generated array to random precisely number of bombs with no repitition

    int ran_arr[bombs];
    int random;

    srand(time(0));

    for (int i = 0; i < bombs; i++)
    {
        random = rand() % total;
        ran_arr[i] = arr[random];
        arr[random] = arr[total - 1];
        total--;
    }

    //Set bombs to original boxes

    for (int i = 0; i < bombs; i++)
    {
        boxes[cell[ran_arr[i]].x][cell[ran_arr[i]].y].attribute = 1;
    }

    return;

}


//UPDATE SURROUNDING

void update_surrounding(int y_located, int x_located)
{
    int start_on_vert = y_located - 1;
    int end_on_vert = y_located + 2;
    int start_on_hor = x_located - 1;
    int end_on_hor = x_located + 2;

    for (int i = start_on_vert; i < end_on_vert; i++)
    {
        for (int j = start_on_hor; j < end_on_hor; j++)
        {
            if (boxes[j][i].attribute == 1)
                boxes[x_located][y_located].surrounded+=1;
            
        }
    }

    return;
}


//Calculate 3bd

int cal_3bd(int height, int width)
{
    int count = 0;

    for (int i = 1; i <= height; i++)
    {
        for (int j = 1; j <= width; j++)
        {
            if (boxes[j][i].attribute == 0)
            {
                if (boxes[j][i].status == 0)
                {
                    count++;
                    reverse_open(i, j, height, width);
                }
                
            }
            
        }
    }

    for (int i = 0; i <= height; i++)
    {
        for (int j = 1; j <= width; j++)
        {
            boxes[j][i].status =0;
        }
    }

    return count;
}



//DRAW BOARD

int draw(int height, int width, int game_mode, int hiscore, string cell)
{
    int opened = 0;
    int vert_bound = height * 2 + 2;
    int hor_bound = width + 2;

    int column = ((int) cell[0]) - 64;
    int row = stoi(cell.substr(1));

    if (game_mode == 0)
    {
        std::cout << "Easy - Best score: " << hiscore << endl;
    }

    if (game_mode == 1)
    {
        std::cout << "Medium - Best score: " << hiscore << endl;
    }

    if (game_mode == 2)
    {
        std::cout << "Hard - Best score: " << hiscore << endl;
    }

    std::cout << endl;

    for (int i = 0; i < vert_bound; i++)
    {
        for (int j = 0; j < hor_bound; j++)
        {
            if (i == 0)
            {
                if (j == 0)
                {
                    std::cout << "  ";
                    continue;
                }
                if (j == 1)
                {
                    std::cout << "  ";
                    continue;
                }
                else
                {
                    std::cout << " " << char(63 + j) << "  ";
                    continue;
                }
             
            }

            if ( i == 1)
            {
                if (j == 0)
                {
                    std::cout << "  ";
                    continue;
                }
                if (j == 1)
                {
                    std::cout << "  ";
                    continue;
                }
                else
                {
                    if (row == 1 && j == column + 1)
                    {
                        setcolor(5);
                        std::cout << "___" << " "; 
                        setcolor(15);
                        continue;
                    }

                    std::cout << "___" << " ";
                    continue;
                }
            }

            if (j == 0)
            {
                if (i % 2 == 0)
                {
                    std::cout.width(2);
                    std::cout << i / 2 << " ";
                    continue;
                }
                else
                {
                    std::cout << "   "; 
                    continue;
                }            
            }

            if (j == 1)
            {
                if (column == 1 && (i == row * 2 || i == row * 2 + 1))
                {
                    setcolor(5);
                    std::cout << "|";
                    setcolor(15);
                    continue;
                }

                std::cout << "|";
                continue;
            }

            if (i % 2 != 0)
            {
                if (j == column + 1 && (i == row * 2 + 1 || i == row * 2 - 1))
                {
                    setcolor(5);
                    std::cout << "___";
                    setcolor(15);

                    if ((j == column + 1 || j == column) && i == row * 2 + 1)
                    {
                        setcolor(5);
                        std::cout << "|";
                        setcolor(15);
                        continue;
                    }

                    std::cout << "|";
                    continue;

                }

                if ((j == column + 1 || j == column) && i == row * 2 + 1)
                {
                    if (j == column + 1 && (i == row * 2 + 1 || i == row * 2 - 1))
                    {
                        setcolor(5);
                        std::cout << "___";
                        setcolor(15);
                    }
                    else
                    {
                        std::cout << "___";
                    }

                    setcolor(5);
                    std::cout << "|";
                    setcolor(15);
                    continue;

                }

                std::cout << "___" << "|";
                continue;
            }

            else
            {
                if (boxes[j - 1][i / 2].status == 1)
                {
                    opened++;
                    if (boxes[j - 1][i / 2].attribute == 1)
                    {
                        std::cout << " ";
                        setcolor(8);
                        std::cout << "#";
                        setcolor(15);
                        std::cout << " ";

                        if (i == row * 2 && (j == column + 1 || j == column))
                        {
                            setcolor(5);
                            std::cout << "|";
                            setcolor(15);
                            continue;
                        }

                        std::cout << "|";
                                       
                        continue;
                    }
                    
                    if (boxes[j - 1][i / 2].surrounded == 0)
                    {
                        std::cout << " " << " " << " ";

                        if (i == row * 2 && (j == column + 1 || j == column))
                        {
                            setcolor(5);
                            std::cout << "|";
                            setcolor(15);
                            continue;
                        }

                        std::cout << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 1)
                    {
                        std::cout << " ";
                        setcolor(9);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " ";

                        if (i == row * 2 && (j == column + 1 || j == column))
                        {
                            setcolor(5);
                            std::cout << "|";
                            setcolor(15);
                            continue;
                        }

                        std::cout << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 2)
                    {
                        
                        std::cout << " ";
                        setcolor(2);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " ";

                        if (i == row * 2 && (j == column + 1 || j == column))
                        {
                            setcolor(5);
                            std::cout << "|";
                            setcolor(15);
                            continue;
                        }

                        std::cout << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 3)
                    {
                        
                        std::cout << " ";
                        setcolor(12);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " ";

                        if (i == row * 2 && (j == column + 1 || j == column))
                        {
                            setcolor(5);
                            std::cout << "|";
                            setcolor(15);
                            continue;
                        }

                        std::cout << "|";
            
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 4)
                    {
                        
                        std::cout << " ";
                        setcolor(1);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " ";

                        if (i == row * 2 && (j == column + 1 || j == column))
                        {
                            setcolor(5);
                            std::cout << "|";
                            setcolor(15);
                            continue;
                        }

                        std::cout << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 5)
                    {
                        
                        std::cout << " ";
                        setcolor(13);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " ";

                        if (i == row * 2 && (j == column + 1 || j == column))
                        {
                            setcolor(5);
                            std::cout << "|";
                            setcolor(15);
                            continue;
                        }

                        std::cout << "|";
                            
                        continue;
                    }


                    std::cout << " " << boxes[j - 1][i / 2].surrounded << " ";

                    if (i == row * 2 && (j == column + 1 || j == column))
                    {
                        setcolor(5);
                        std::cout << "|";
                        setcolor(15);
                        continue;
                    }

                    std::cout << "|";
                    continue;
                }

                if (boxes[j - 1][i / 2].flagged)
                {
                    
                    std::cout << " ";
                    setcolor(4);
                    std::cout << "*";
                    setcolor(15);
                    std::cout << " ";

                    if (i == row * 2 && (j == column + 1 || j == column))
                    {
                        setcolor(5);
                        std::cout << "|";
                        setcolor(15);
                        continue;
                    }

                    std::cout << "|";
                    
                    continue;
                }
                
                std::cout << " ";
                setcolor(6);
                std::cout << "?";
                setcolor(15);
                std::cout << " ";

                if (i == row * 2 && (j == column + 1 || j == column))
                {
                    setcolor(5);
                    std::cout << "|";
                    setcolor(15);
                    continue;
                }

                std::cout << "|";
                
            }
        }

        std::cout << endl;
    }

    return opened; 
}


//TAKE COMMANDS, EXECUTE COMMANDS

string take_command(int height, int width, int thbd, int game_mode, int hiscore, string cell)
{

    string cmd, temp;

    do
    {
        cmd = handleInput(cell, height, width, game_mode, hiscore);
        temp = cmd;
        temp.resize(3);

        if (check_cell(temp, height, width) && temp.length() == 3)
        {
            cell = temp;
        }

        if (cmd == "q")
        {
            break;
        }

        if (cmd == "n")
        {
            if (newgame())
            {
                return "new";
            }

            continue;
            
        }
        
    } while (cmd.size() != 4 || (cmd[3] != 'o' && cmd[3] != 'f' && cmd[3] != 'u') || !check_cell(cmd, height, width));

    
    if (cmd == "q")
    {        
        
        if (save(height, width, thbd, 5, -1, game_mode, 1))
        {
            
            std::cout << "Your process is saved." << endl;
            return "y";
        }
        

        if (newgame())
        {
            return "new";
        }

        return "n";
        
    }

   
    int pos_x = ((int) cmd[0]) - 64;
    int pos_y = stoi(cmd.substr(1));


    if (cmd[3] == 'f')
    {
        boxes[pos_x][pos_y].flagged = true;
    }
    if (cmd[3] == 'u')
    {
        boxes[pos_x][pos_y].flagged = false;
    }
    if (cmd[3] == 'o')
    {
        open(pos_x, pos_y, height, width);
    }
    
    return cmd;
}


//CHECK VALID CELL

bool check_cell(string cmd, int height, int width)
{
    
    if ((cmd[0] > (char) (width + 64) || cmd[0] < 'A') && (((int) cmd[1] - 48) * 10 + (int) cmd[2] - 48 > height || ((int) cmd[1] - 48) * 10 + (int) cmd[2] - 48 < 1))
    {
        return false;
    }

    return true;
}



//HANDLE INPUT

string handleInput(string cell, int height, int width, int game_mode, int hiscore)
{
    string cmd, remained_cmd;

    std::cout << "Enter command (q to quit, n to remake): ";

    int ch;
    ch = _getch();

    if (ch != 224)
    {
        string s1 = string(1, ch);
        std::cout << s1;
        getline(std::cin, remained_cmd);
        cmd = s1 + remained_cmd;
        return cmd;
    }

    else
    {
        cmd = move_with_arrow(cell, height, width, game_mode, hiscore);
    }

    return cmd;
}


//MOVE WITH ARROW

string move_with_arrow(string cell, int height, int width, int game_mode, int hiscore)
{
    string new_cell, cmd, action;
    int ch;
    char c = cell[0];
    int row = stoi(cell.substr(1));
    ch = _getch();

    if (ch == 72)
    {
        string column = string(1, c);
        --row;
        if (row < 1)
        {
            row = height;
        }

        string str_row = to_string(row);
        string formatted_row = string(2 - str_row.length(), '0') + str_row;

        new_cell = column + formatted_row;

        std::cout << endl;
        system("cls");
        draw(height, width, game_mode, hiscore, new_cell);
        cout << "Move with arrow ( add o or f or u to finish command): " << new_cell;
        ch = _getch();
        cout << endl;
        action = string(1, ch);

        if (ch != 224)
        {
            cmd = new_cell + action;
            return cmd;
        }
        
        cmd = move_with_arrow(new_cell, height, width, game_mode, hiscore);

    }


    else if (ch == 80)
    {
        string column = string(1, c);
        ++row;
        if (row > height)
        {
            row = 1;
        }

        string str_row = to_string(row);
        string formatted_row = string(2 - str_row.length(), '0') + str_row;

        new_cell = column + formatted_row;

        std::cout << endl;
        system("cls");
        draw(height, width, game_mode, hiscore, new_cell);
        cout << "Move with arrow (add o or f or u to finish command): " << new_cell;
        ch = _getch();
        cout << endl;
        action = string(1, ch);

        if (ch != 224)
        {
            cmd = new_cell + action;
            return cmd;
        }
        
        cmd = move_with_arrow(new_cell, height, width, game_mode, hiscore);
    }


    else if (ch == 77)
    {
        
        ++c;

        if (c > (char) (width + 64))
        {
            c = 'A';
        }

        string column = string(1, c);

        string str_row = to_string(row);
        string formatted_row = string(2 - str_row.length(), '0') + str_row;

        new_cell = column + formatted_row;

        std::cout << endl;
        system("cls");
        draw(height, width, game_mode, hiscore, new_cell);
        cout << "Move with arrow (add o or f or u to finish command): " << new_cell;
        ch = _getch();
        cout << endl;
        action = string(1, ch);

        if (ch != 224)
        {
            cmd = new_cell + action;
            return cmd;
        }
        
        cmd = move_with_arrow(new_cell, height, width, game_mode, hiscore);

    }


    else if (ch == 75)
    {
        --c;

        if (c < 'A')
        {
            c = (char) (width + 64);
        }

        string column = string(1, c);

        string str_row = to_string(row);
        string formatted_row = string(2 - str_row.length(), '0') + str_row;

        new_cell = column + formatted_row;

        std::cout << endl;
        system("cls");
        draw(height, width, game_mode, hiscore, new_cell);
        cout << "Move with arrow (add o or f or u to finish command): " << new_cell;
        ch = _getch();
        cout << endl;
        action = string(1, ch);

        if (ch != 224)
        {
            cmd = new_cell + action;
            return cmd;
        }
        
        cmd = move_with_arrow(new_cell, height, width, game_mode, hiscore);
    }

    else
    {
        cmd = cell;
    }

    return cmd;

}



//OPEN NEIGHBOUR-CELLS IF CURRENT CELL SURROUNDED IS SET TO 0


void open(int pos_x, int pos_y, int height, int width)
{
  
    if (boxes[pos_x][pos_y].status == 1)
    {
        return;
    }

    if (boxes[pos_x][pos_y].surrounded != 0)
    {
        boxes[pos_x][pos_y].status = 1;
        
        return;
    }

    if (pos_x < 1 || pos_x > width)
    {
        
        return;
    }

    if (pos_y < 1 || pos_y > height)
    {
        
        return;
    }

    boxes[pos_x][pos_y].status = 1;

    
    for (int i = pos_y - 1; i < pos_y + 2; i++)
    {
        for (int j = pos_x - 1; j < pos_x + 2; j++)
        {
           
            open(j, i, height, width);
        }
    }

    return;

}


// FOR CALCULATE 3BD'S

void reverse_open(int pos_x, int pos_y, int height, int width)
{
    int start_y = pos_y - 1;
    int end_y = pos_y + 2;
    int start_x = pos_x - 1;
    int end_x = pos_x +2;

    
    for (int i = start_y; i < end_y; i++)
    {
        if (i < 1 || i > height)
        {
            continue;
        }

        for (int j = pos_x - 1; j < pos_x + 2; j++)
        {
            if (j < 1 || j > width)
            {
                continue;
            }
           
            if (boxes[j][i].attribute == 0)
            {
                if (boxes[j][i].status == 0)
                {
                    if (boxes[j][i].surrounded == 0)
                    {
                        open(j, i, height, width);
                        return;
                    }
                }
            }
        }
    }

    return;
}


//CHECK FOR GAME-OVER

bool game_over(string cmd)
{
    if (cmd[3] == 'o')
    {
        int pos_x = ((int) cmd[0]) - 64;
        int pos_y = stoi(cmd.substr(1));
        if (boxes[pos_x][pos_y].attribute == 1)
        {
            return true;
        }
        
    }

    return false;
}


//THIS FUNCTION IS CALLED WHENEVER I FEEL RIGHT (OR MAY BE NOT)

bool newgame()
{
    string confirm;

    std::cout << endl;
    
    std::cout << "New game ? (If your game has not finished, the current proccess will be lost)" << endl;

    do
    {
        std::cout << "Press y to confirm, if not press n: ";
        getline(cin, confirm);

    } while (confirm.size() != 1 || (confirm[0] != 'y' && confirm[0] != 'n'));


    if (confirm == "n")
    {
        return false;
    }

    return true;
}


// SAVING SYSTEM WORKS BASED ON SQLITE

bool save(int height, int width, int thbd, int time, int hiscore, int game_mode, int cases)
{
    
    if (cases == 1)
    {
        int upper_hi = height + 2;
        int upper_wi = width + 2;
        string is_save;

        std::cout << "Do you want to save the process ?" << endl;

        do
        {
            std::cout << "Enter y if yes, else enter n and the process will be lost: ";
            getline(std::cin, is_save);

        } while (is_save.size() != 1 || (is_save[0] != 'y' && is_save[0] != 'n'));


        if (is_save == "y")
        {
            sqlite3* DB; 
            sqlite3_stmt *st;
            int exit = 0; 
            exit = sqlite3_open("minesdata.db", &DB);

            std::cout << "....." << endl;
            std::cout << endl; 
            string sql3 = "UPDATE minesdata SET attribute = ?, status = ?, surrounded = ?, flagged = ?, height = ?, width = ?, thbd = ?, mode = ? WHERE column = ? AND row = ?;"; 

            for (int i = 0; i < upper_hi; i++)
            {
                for (int j = 0; j < upper_wi; j++)
                {
                    int rc = sqlite3_prepare_v2(DB, sql3.c_str(), -1, &st, NULL);
                    if (rc == SQLITE_OK)
                    {
                        
                        sqlite3_bind_int(st, 1, boxes[j][i].attribute);
                        sqlite3_bind_int(st, 2, boxes[j][i].status);
                        sqlite3_bind_int(st, 3, boxes[j][i].surrounded);
                        sqlite3_bind_int(st, 4, boxes[j][i].flagged);
                        sqlite3_bind_int(st, 5, height);
                        sqlite3_bind_int(st, 6, width);
                        sqlite3_bind_int(st, 7, thbd);
                        sqlite3_bind_int(st, 8, game_mode);
                        sqlite3_bind_int(st, 9, i);
                        sqlite3_bind_int(st, 10, j);
                        sqlite3_step(st);
                        sqlite3_finalize(st);
                    }
                }
            }
            
            sqlite3_close(DB); 
            return true;
        }
    }
    
    if (cases == 2)
    {
        sqlite3* DB; 
        sqlite3_stmt *st;
        int exit = 0; 
        exit = sqlite3_open("minesdata.db", &DB);

        string sql4 = "UPDATE minesdata SET time = ? WHERE column = ? AND row = ?;";

        int re = sqlite3_prepare_v2(DB, sql4.c_str(), -1, &st, NULL);

        if (re == SQLITE_OK)
        {
            sqlite3_bind_int(st, 1, time);
            sqlite3_bind_int(st, 2, 0);
            sqlite3_bind_int(st, 3, 0);
            sqlite3_step(st);
            sqlite3_finalize(st);
        }

        sqlite3_close(DB);
  
    }


    if (cases == 3)
    {
        sqlite3* DB; 
        sqlite3_stmt *st;
        int exit = 0; 
        exit = sqlite3_open("minesdata.db", &DB);

        string sql5 = "UPDATE minesdata SET hiscore = ? WHERE column = ? AND row = ?;";

        int re = sqlite3_prepare_v2(DB, sql5.c_str(), -1, &st, NULL);

        if (re == SQLITE_OK)
        {
            sqlite3_bind_int(st, 1, hiscore);
            sqlite3_bind_int(st, 2, 0);
            sqlite3_bind_int(st, 3, game_mode);
            sqlite3_step(st);
            sqlite3_finalize(st);
        }

        sqlite3_close(DB);

    }

    return false;
}



//TAKE HIGHSCORE IN THE DATABASE

int receive_highscore(int game_mode)
{
    sqlite3* db;
    sqlite3_stmt *st;
    int value = 0;
    int exit = 0;
    int hiscore;
    exit = sqlite3_open("minesdata.db", &db);

    
    string sql1 = "SELECT * FROM minesdata WHERE column = ? AND row = ?;";

    int rt = sqlite3_prepare_v2(db, sql1.c_str(), -1, &st, NULL);

    if (rt == SQLITE_OK)
    {
        sqlite3_bind_int(st, 1, value);
        sqlite3_bind_int(st, 2, game_mode);
        sqlite3_step(st);

        hiscore = sqlite3_column_int(st, 10);
        sqlite3_finalize(st);
    }

    return hiscore;
}



//MANAGE HIGHSCORE

bool manage_high_score(int score, int high_score, int game_mode)
{
    if (game_mode == 0)
    {
        if (score > high_score)
        {
            std::cout << "Best score (easy mode): " << score << endl;
            std::cout << endl;
            return true;
        }
        else
        {
            std::cout << "Best score (easy mode): " << high_score << endl;
            std::cout << endl;
        }
        return false;
        
    }

    if (game_mode == 1)
    {
        if (score > high_score)
        {
            std::cout << "Best score (medium mode): " << score << endl;
            std::cout << endl;
            return true;
        }
        else
        {
            std::cout << "Best score (medium mode): " << high_score << endl;
            std::cout << endl;
        }
        return false;
        
    }

    if (game_mode == 2)
    {
        if (score > high_score)
        {
            std::cout << "Best score (hard mode): " << score << endl;
            std::cout << endl;
            return true;
        }
        else
        {
            std::cout << "Best score (hard mode): " << high_score << endl;
            std::cout << endl;
        }
        return false;
        
    }

    return false;
}