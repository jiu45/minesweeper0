#include <iostream>
#include <string>
#include <cstdlib> 
#include <time.h> 
#include <windows.h>     //Insert color
#include <sqlite3.h>     //Save data

using namespace std;

// Define maximum boxes

const int N = 32;

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
int draw(int height, int width);
string take_command(int height, int width);
void open(int pos_x, int pos_y, int height, int width);
bool game_over(string cmd);
bool newgame();
bool save(int height, int width);
void showboard(int height, int width);



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
    
    std::cout << "Welcome to Minsweeper on terminal :)" << endl;
    std::cout << "Press enter to continue";
    std::cin.get();
    


    
    //Rules

    std::cout << std::endl;

    std::cout << "Rules:\n";

    std::cout << "To open: type in o and location with no spaces, row first(capital) and column after (eg. oA05)" << endl;
    std::cout << "To flag: type in f and location with no spaces, row first(capital) and column after (eg. fC16)" << endl;
    std::cout << "To unflag: type in u and location with no spaces, row first(capital) and column after (eg. uC06)" << endl;
    std::cout << "When you're ready, press enter to continue" << endl;
    std::cin.get();
    std::cin.clear();    
    

    //Execute at every time the program run and loop when is_new_game is true
    

    bool is_new_game = false;


    do
    {
        int board_height, board_width, bombs;
        
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

                sqlite3_finalize(st);
            }

            

            //This can be executed when the program first run or new the last user did not save the process

            if (s_height == 0)
            {
                is_new_game = true;
                std::cout << "No records of last game, press enter to continue: ";
                std::cin.get();
            } 

            //If the data is saved succesfully
            //Assign the last game's data in to boxes

            if (s_height != 0)
            {
                
                board_height = s_height;
                board_width = s_width;
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
            std::cout << "A - Easy\n";
            std::cout << "B - Medium\n";
            std::cout << "C - Hard\n";
            std::cout << "M - Customize your board\n";

            

            do
            {
                
                std::cout << "Enter A or B or C or M" << endl;
                std::cin >> str1;
                
                
            } while ((str1[0] != 'A' && str1[0] != 'B' && str1[0] != 'C' && str1[0] != 'M') || str1.size() > 1);

            std::cin.clear();

            std::cin.get();

            //Set up for level

            

            switch (str1[0])
            {
            case 'A':

                board_height = 9;
                board_width = 9;
                bombs = 10;
                break;
            
            case 'B':
                board_height = 16;
                board_width = 16;
                bombs = 40;
                break;
            case 'C':
                board_height = 30;
                board_width = 16;
                bombs = 99;
                break;

            case 'M':
                std::cout << "Input the board's height (< 30): " << endl;
                std::cin >> board_height;
                std::cout << "Input the board's width (< 26): " << endl;
                std::cin >> board_width;
                std::cout << "Input number of bombs < " << board_height * board_width << " : " << endl;
                std::cin >> bombs;

            }

            set_bombs(bombs, board_height, board_width);   //mines are at location, be carefull !

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
        
        }


        //PLAY-TIME

        int max = board_height * board_width - bombs;
        string cmd;
        bool show_board = true;
        

        //Loop through the entire game


        while (true)
        {
            //If their is no more box to open -> win

            if (draw(board_height, board_width) >= max)
            {
                std::cout << "You win !" << endl;
                std::cout << endl;
                std::cout << "Press enter to continue";
                std::cin.get();
                is_new_game = newgame();
                std::cout << "Press enter to show the board for last match: ";
                std::cin.get();
                break;
            }
        
            std::cout << endl;

            cmd = take_command(board_height, board_width);

            //If new game is selected
            if (cmd == "new")
            {
                is_new_game = true;
                break;
            }

            //If not save is selected
            if (cmd == "n")
            {
                is_new_game = false;
                break;
            }

            //If save is chosen
            if (cmd == "y")
            {
                is_new_game = false;
                show_board = false;        //Change the value to avoid show_board
                break;
            }

            //If clicked onto bomb
            if (game_over(cmd))
            {
                std::cout << "Clicked onto bomb. Exploded." << endl;
                std::cout << endl;
                std::cout << "Game over" << endl;
                std::cout << endl;
                is_new_game = newgame();    // a small remind to revenge :)

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

        //End game, show board

        if (show_board)
        {
            showboard(board_height, board_width);
        
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

void showboard(int height, int width)
{
    //Show board

    for (int i = 0; i <= height; i++)
    {
        for (int j = 0; j <= width; j++)
        {
            
            boxes[j][i].status = 1;
            
        }
    }


    draw(height, width);

    //Reset data

    int insert = 0;
 
    sqlite3* DB; 
    sqlite3_stmt *st;
    int exit = 0; 
    exit = sqlite3_open("minesdata.db", &DB);

    
    string sql3 = "UPDATE minesdata SET attribute = ?, status = ?, surrounded = ?, flagged = ?, height = ?, width = ? WHERE column = ? AND row = ?;"; 

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

                
                sqlite3_bind_int(st, 7, i);
                sqlite3_bind_int(st, 8, j);
                sqlite3_step(st);
                sqlite3_finalize(st);
            }

            if (rc != SQLITE_OK)
            {
                cout << "fail" << endl;
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

//DRAW BOARD

int draw(int height, int width)
{
    int opened = 0;

    int vert_bound = height * 2 + 2;
    int hor_bound = width + 2;

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
                std::cout << "|";
                continue;
            }

            if (i % 2 != 0)
            {
                std::cout << "___" << "|";
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
                        std::cout << " " << "|";                       
                        continue;
                    }
                    
                    if (boxes[j - 1][i / 2].surrounded == 0)
                    {
                        std::cout << " " << " " << " " << "|";
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 1)
                    {
                        
                        std::cout << " ";
                        setcolor(9);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " " << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 2)
                    {
                        
                        std::cout << " ";
                        setcolor(2);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " " << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 3)
                    {
                        
                        std::cout << " ";
                        setcolor(12);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " " << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 4)
                    {
                        
                        std::cout << " ";
                        setcolor(1);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " " << "|";
                        
                        continue;
                    }

                    if (boxes[j - 1][i / 2].surrounded == 5)
                    {
                        
                        std::cout << " ";
                        setcolor(13);
                        std::cout << boxes[j - 1][i / 2].surrounded;
                        setcolor(15);
                        std::cout << " " << "|";
                       
                        continue;
                    }

                    std::cout << " " << boxes[j - 1][i / 2].surrounded << " " << "|";
                    continue;
                }

                if (boxes[j - 1][i / 2].flagged)
                {
                    
                    std::cout << " ";
                    setcolor(4);
                    std::cout << "*";
                    setcolor(15);
                    std::cout << " " << "|";
                    
                    continue;
                }
                
                std::cout << " ";
                setcolor(6);
                std::cout << "?";
                setcolor(15);
                std::cout << " " << "|";
                
            }
        }

        std::cout << endl;
    }

    return opened; 
}


//TAKE COMMANDS, EXECUTE COMMANDS

string take_command(int height, int width)
{

    string cmd;

    do
    {
        std::cout << "Enter command (q to quit, n to remake): ";
    
        getline(std::cin, cmd);

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
        
    } while (cmd.size() != 4 || (cmd[0] != 'o' && cmd[0] != 'f' && cmd[0] != 'u'));

    
    if (cmd == "q")
    {        
        
        if (save(height, width))
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

   
    while ((cmd[1] > (char) (width + 64) || cmd[1] < 'A') && (((int) cmd[2] - 48) * 10 + (int) cmd[3] - 48 > height || ((int) cmd[2] - 48) * 10 + (int) cmd[3] - 48 < 1))
    {
        std::cout << "Enter command (o or f or u): ";
        getline(std::cin, cmd);
    }

    
    int pos_x = ((int) cmd[1]) - 64;
    int pos_y = ((int) cmd[2] - 48) * 10 + (int) cmd[3] - 48;


    if (cmd[0] == 'f')
    {
        boxes[pos_x][pos_y].flagged = true;
    }
    if (cmd[0] == 'u')
    {
        boxes[pos_x][pos_y].flagged = false;
    }
    if (cmd[0] == 'o')
    {
        open(pos_x, pos_y, height, width);
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

//CHECK FOR GAME-OVER

bool game_over(string cmd)
{
    if (cmd[0] == 'o')
    {
        int pos_x = ((int) cmd[1]) - 64;
        int pos_y = ((int) cmd[2] - 48) * 10 + (int) cmd[3] - 48;
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
    
    cout << "New game ? (If your game has not finished, the current proccess will be lost)" << endl;

    do
    {
        cout << "Press y to confirm, if not press n: ";
        getline(cin, confirm);

    } while (confirm.size() != 1 || (confirm[0] != 'y' && confirm[0] != 'n'));


    if (confirm == "n")
    {
        return false;
    }

    return true;
}


// SAVING SYSTEM WORKS BASED ON SQLITE

bool save(int height, int width)
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

    sqlite3* DB; 
    sqlite3_stmt *st;
    int exit = 0; 
    exit = sqlite3_open("minesdata.db", &DB);

    if (is_save == "y")
    {
        cout << "....." << endl;
        cout << endl; 
        string sql3 = "UPDATE minesdata SET attribute = ?, status = ?, surrounded = ?, flagged = ?, height = ?, width = ? WHERE column = ? AND row = ?;"; 

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
                    sqlite3_bind_int(st, 7, i);
                    sqlite3_bind_int(st, 8, j);
                    sqlite3_step(st);
                    sqlite3_finalize(st);
                }
            }
        }
        
    
        sqlite3_close(DB); 

        return true;
    
    }

    return false;
}



