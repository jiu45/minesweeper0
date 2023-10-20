#include <iostream>
#include <string>
#include <cstdlib> 
#include <time.h> 

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


void set_bombs(int bombs, int height, int width);
void update_surrounding(int y_located, int x_located);
int draw(int height, int width);
string take_command(int height, int width);
void open(int pos_x, int pos_y, int height, int width);
bool game_over(string cmd);


int main()
{
    // Welcome user
    
    std::cout << "Welcome to Minsweeper on terminal :)" << endl;
    std::cout << "Press enter to continue";
    cin.get();
    

    //Set default 

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

    //Rules

    std::cout << std::endl;

    std::cout << "Rules:\n";

    std::cout << "To open: type in o and location with no spaces, row first(capital) and column after (eg. oA5)" << endl;
    std::cout << "To flag: type in f and location with no spaces, row first(capital) and column after (eg. fC6)" << endl;
    std::cout << "To unflag: type in u and location with no spaces, row first(capital) and column after (eg. uC6)" << endl;
    std::cout << "When you're ready, press enter to continue" << endl;
    cin.get();
    cin.clear();    
    
    // Choose level
    
    string str1;
    std::cout << "Please choose difficulty level:\n";
    std::cout << "A - Easy\n";
    std::cout << "B - Medium\n";
    std::cout << "C - Hard\n";

    do
    {
        
        std::cout << "Enter A or B or C" << endl;
        cin >> str1;
        
        
    } while ((str1[0] != 'A' && str1[0] != 'B' && str1[0] != 'C') || str1.size() > 1);

    cin.clear();

    cin.get();

    //Set up for level

    int board_height, board_width, bombs;

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


    //PLAY-TIME

    int max = board_height * board_width - bombs;


    //Loop through the entire game


    while (true)
    {
        if (draw(board_height, board_width) >= max)
        {
            std::cout << "You win !" << endl;
            std::cout << endl;
            std::cout << "Press enter to continue";
            cin.get();
            break;
        }
       
        std::cout << endl;

        if (game_over(take_command(board_height, board_width)))
        {
            std::cout << "Clicked onto bomb. Exploded." << endl;
            std::cout << endl;
            std::cout << "Game over" << endl;
            std::cout << "Press enter to show board";
            cin.get();
            break;
        }
   
    }

    //End game, show board

    for (int i = 0; i <= board_height; i++)
    {
        for (int j = 0; j <= board_width; j++)
        {
              
            boxes[j][i].status = 1;
            boxes[j][i].surrounded = 0;
            
        }
    }


    draw(board_height, board_width);

    //Goodbye

    std::cout << "Press enter to continue";
    cin.get();

    std::cout << endl;

    std::cout << "Have you enjoyed :) ? \n";
    std::cout << endl;
    std::cout << "Follow ShiinaDidn'tBlush on ButtBook !" << endl;
    std::cout << endl;
    std::cout << "Restart the program to play again \n";
    std::cout << endl;

    return 0;

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
                    std::cout << " ";
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
                    std::cout << " ";
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
                    cout.width(2);
                    std::cout << i / 2 << " ";
                    continue;
                }
                else
                {
                    std::cout << "  "; 
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
                        std::cout << " " << "#" << " " << "|";
                        continue;
                    }
                    
                    if (boxes[j - 1][i / 2].surrounded == 0)
                    {
                        std::cout << " " << " " << " " << "|";
                        continue;
                    }
                    std::cout << " " << boxes[j - 1][i / 2].surrounded << " " << "|";
                    continue;
                }

                if (boxes[j - 1][i / 2].flagged == true)
                {
                    std::cout << " " << "*" << " " << "|";
                    continue;
                }
                std::cout << " " << "?" << " " << "|";
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
        std::cout << "Enter command (o or f or u): ";
    
        getline(cin, cmd);
        
    } while (cmd.size() != 4 || (cmd[0] != 'o' && cmd[0] != 'f' && cmd[0] != 'u'));
    
    
    
    

    
    while ((cmd[1] > (char) (width + 64) || cmd[1] < 'A') && (((int) cmd[2] - 48) * 10 + (int) cmd[3] - 48 > height || ((int) cmd[2] - 48) * 10 + (int) cmd[3] - 48 < 1))
    {
        std::cout << "Enter command (o or f or u): ";
        getline(cin, cmd);
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
        int pos_y = (int) cmd[2] - 48;
        if (boxes[pos_x][pos_y].attribute == 1)
        {
            return true;
        }
        
    }

    return false;
}





