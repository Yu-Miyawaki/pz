#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>

class Timer{
public:
    Timer(){
        timer = clock();
    }
    void call(const std::string &s){
        clock_t now = clock();
        double dt = static_cast<double>(now - timer) / CLOCKS_PER_SEC * 1000.0;
        if(!s.empty()) std::cout << s << ": ";
        std::cout << dt << "[ms]" << std::endl;
        timer = clock();
    }
    void set(){
        timer = clock();
    }

private:
    clock_t timer;
};


/// @brief 蟻本4-5 候補が少ない順に全探索する
class solver_np{
public:
    solver_np(std::vector<std::vector<int>> board_){
        board = board_;
        grid_size = board.size();
        board_bit.resize(grid_size, std::vector<int>(grid_size));
        for(int i=1; i<=MAX_GRID_SIZE; ++i){
            if(i*i == grid_size){
                squareroot_grid_size = i;
                break;
            }
        }
    }

    /// @brief 解を1つ返し、求解時間を出力する
    /// TODO: 複数解への対応
    std::vector<std::vector<int>> solve(){
        if(grid_size > MAX_GRID_SIZE){
            std::cout << "too big size!" << std::endl;
            std::vector<std::vector<int>> tmp;
            return tmp;
        }
        if(squareroot_grid_size == -1){
            std::cout << "invalid board!" << std::endl;
            std::vector<std::vector<int>> tmp;
            return tmp;
        }
        timer.set();
        init_board_bit();
        auto first_search = search_board();
        if(first_search.first == true){
            std::cout << "already solved!" << std::endl;
            return board;
        }

        if(dfs_fill(first_search.second.first, first_search.second.second)){
            timer.call("solved");
            return solution;
        }

        else{
            timer.call("No Answer!");
            return solution;
        }
    }

private:
    int MAX_GRID_SIZE = 16;
    int grid_size;
    int squareroot_grid_size = -1;
    std::vector<std::vector<int>> board, board_bit; // bitはありうる値の1-indexedのビット列
    std::vector<std::vector<int>> solution;
    Timer timer;

    /// @brief board, board_bitについて(row, col)にnumberを代入する
    void set_board(int row, int col, int number){
        for(int r=0; r<grid_size; ++r) board_bit[r][col] &= (~(1<<number));
        for(int c=0; c<grid_size; ++c) board_bit[row][c] &= (~(1<<number));
        int block_row = squareroot_grid_size * (row / squareroot_grid_size);
        int block_col = squareroot_grid_size * (col / squareroot_grid_size);
        for(int i=0; i<squareroot_grid_size; ++i){
            for(int j=0; j<squareroot_grid_size; ++j){
                int r = block_row + i;
                int c = block_col + j;
                // numberに対応するビットを0にする
                board_bit[r][c] &= (~(1<<number));
            }
        }
        board[row][col] = number;
        board_bit[row][col] = (1<<number);
    }

    /// @brief boardの情報をboard_bitにbit情報で転記
    void init_board_bit(){
        int bit_max = 0;
        for(int i=1; i<=grid_size; ++i) bit_max |= (1<<i);
        for(int row=0; row<grid_size; ++row){
            for(int col=0; col<grid_size; ++col){
                board_bit[row][col] = bit_max;
            }
        }

        // 初期値を代入
        for(int row=0; row<grid_size; ++row){
            for(int col=0; col<grid_size; ++col){
                if(board[row][col]){
                    set_board(row, col, board[row][col]);
                }
            }
        }
    }

    /// @brief board[row][col]にnumberを代入した場合に数字が入らなくなるマスがないかを返す 
    bool is_valid_fill(int row, int col, int number){
        assert(!board[row][col]);
        if(((board_bit[row][col]>>number) & 1) == 0) return false;

        for(int i=0; i<grid_size; ++i){
            if(i != col && !board[row][i]){
                if((board_bit[row][i] & (~(1<<number))) == 0) return false;
            }
            if(i != row && !board[i][col]){
                if((board_bit[i][col] & (~(1<<number))) == 0) return false;
            }
        }
        int block_row = squareroot_grid_size * (row / squareroot_grid_size);
        int block_col = squareroot_grid_size * (col / squareroot_grid_size);
        for(int i=0; i<squareroot_grid_size; ++i){
            for(int j=0; j<squareroot_grid_size; ++j){
                int r = block_row + i;
                int c = block_col + j;
                if(r == row && c == col) continue;
                if(!board[r][c]){
                    if((board_bit[r][c] & (~(1<<number))) == 0) return false;
                }
            }
        }

        return true;
    }

    /// @brief 全マスを走査し現在のboardが解となっているか, そうでない場合候補数が少ないマス目を返す 
    std::pair<bool, std::pair<int, int>> search_board(){
        bool is_solved = true;
        std::pair<int, int> min_square = std::make_pair(-1, -1);
        int min_num_valid = grid_size+1;
        for(int row=0; row<grid_size; ++row){
            for(int col=0; col<grid_size; ++col){
                if(!board[row][col]){
                    is_solved = false;
                    if(num_valid(board_bit[row][col]) < min_num_valid){
                        min_num_valid = num_valid(board_bit[row][col]);
                        min_square = std::make_pair(row, col);
                    }
                }
            }
        }

        return std::make_pair(is_solved, min_square);
    }

    int num_valid(int bit){
        // ビットの1の数を返す
        return __builtin_popcount(bit);
    }

    /// @brief (row, col)に入る数字を順に代入し、解が見つかればtrueを返す 
    bool dfs_fill(int row, int col){
        // 現在の値の退避
        std::vector<int> tmp_row = board_bit[row];
        std::vector<int> tmp_col(grid_size);
        std::vector<std::vector<int>> tmp_square(squareroot_grid_size, std::vector<int>(squareroot_grid_size));
        for(int i=0; i<grid_size; ++i) tmp_col[i] = board_bit[i][col];
        int block_row = squareroot_grid_size * (row / squareroot_grid_size);
        int block_col = squareroot_grid_size * (col / squareroot_grid_size);
        for(int i=0; i<squareroot_grid_size; ++i){
            for(int j=0; j<squareroot_grid_size; ++j){
                int r = block_row + i;
                int c = block_col + j;
                tmp_square[i][j] = board_bit[r][c];
            }
        }

        // 順番に代入し、解が見つかれば終了する
        for(int number=1; number<=grid_size; ++number){
            if(is_valid_fill(row, col, number)){
                set_board(row, col, number);
                
                auto next_search = search_board();
                bool is_solved = next_search.first;
                if(is_solved){
                    solution = board;
                    return true;
                }

                int next_row = next_search.second.first;
                int next_col = next_search.second.second;
                if(dfs_fill(next_row, next_col)){
                    return true;
                }
                // numberで解が見つからなかった場合は退避した値を戻す
                else{
                    board[row][col] = 0;
                    for(int i=0; i<grid_size; ++i) board_bit[i][col] = tmp_col[i];
                    for(int i=0; i<grid_size; ++i) board_bit[row][i] = tmp_row[i];
                    for(int i=0; i<squareroot_grid_size; ++i){
                        for(int j=0; j<squareroot_grid_size; ++j){
                            int r = block_row + i;
                            int c = block_col + j;
                            board_bit[r][c] = tmp_square[i][j];
                        }
                    }
                }
            }
        }

        // 全てのnumberで解が見つからなかった場合
        return false;
    }
};

int main(){
    std::ifstream INPUT_TXT("input.txt");
    std::cin.rdbuf(INPUT_TXT.rdbuf());
    int grid_size;
    std::cin >> grid_size;
    std::vector<std::vector<int>> board(grid_size, std::vector<int>(grid_size));
    for(int i=0; i<grid_size; ++i){
        for(int j=0; j<grid_size; ++j){
            std::cin >> board[i][j];
        }
    }
    
    solver_np sv(board);
    auto answer = sv.solve();
    
    if(answer.empty()){
        std::cout << "Not solved\n";
    }
    else{
        for(const auto &rows : answer){
            for(const int num : rows){
                std::cout << num << " ";
            }
            std::cout << std::endl;
        }
    }
    return 0;
}

/*

https://gigazine.net/news/20100822_hardest_sudoku/
9
0 0 5 3 0 0 0 0 0
8 0 0 0 0 0 0 2 0
0 7 0 0 1 0 5 0 0
4 0 0 0 0 5 3 0 0
0 1 0 0 7 0 0 0 6
0 0 3 2 0 0 0 8 0
0 6 0 5 0 0 0 0 9
0 0 4 0 0 0 0 3 0
0 0 0 0 0 9 7 0 0

*/