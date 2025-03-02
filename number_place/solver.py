
import time
from pathlib import Path
from typing import List, Tuple, Optional
import argparse
import os
import contextlib
import json
import sys

# transpiled by GPT

class Timer:
    """ インスタンスを呼び出すごとに前回の呼び出しからの経過時間を返す, また標準出力する """
    def __init__(self, s: str = ''):
        self._t = time.perf_counter()
        self._s = s # 時間と一緒にデフォルト出力する文字列

    def __call__(self, s: Optional[str] = None, is_print: bool = True) -> float:
        """ 前回の呼び出しからの時間を返す """
        if s is None:
            s = self._s
        t = time.perf_counter()
        diff = t - self._t
        if is_print:
            print(f'{s} time: {diff}')
        self._t = time.perf_counter()
        return diff


class SolverNP:
    def __init__(self, board: List[List[int]]):
        self.board = board
        self.grid_size = len(board)
        self.board_bit = [[0] * self.grid_size for _ in range(self.grid_size)]
        self.MAX_GRID_SIZE = 16
        self.sqrt_grid_size = -1
        self.solution = []
        self.timer = Timer()

        for i in range(1, self.MAX_GRID_SIZE + 1):
            if i * i == self.grid_size:
                self.sqrt_grid_size = i
                break

    def solve(self) -> List[List[int]]:
        if self.grid_size > self.MAX_GRID_SIZE:
            print("too big size!")
            return []

        if self.sqrt_grid_size == -1:
            print("invalid board!")
            return []

        self.timer(is_print=False)
        self.init_board_bit()

        first_search = self.search_board()
        if first_search[0]:
            print("already solved!")
            return self.board

        if self.dfs_fill(first_search[1][0], first_search[1][1]):
            self.timer("solved")
            return self.solution
        else:
            self.timer("No Answer!")
            return self.solution

    def set_board(self, row: int, col: int, number: int):
        for r in range(self.grid_size):
            self.board_bit[r][col] &= ~(1 << number)
        for c in range(self.grid_size):
            self.board_bit[row][c] &= ~(1 << number)

        block_row = self.sqrt_grid_size * (row // self.sqrt_grid_size)
        block_col = self.sqrt_grid_size * (col // self.sqrt_grid_size)
        for i in range(self.sqrt_grid_size):
            for j in range(self.sqrt_grid_size):
                r = block_row + i
                c = block_col + j
                self.board_bit[r][c] &= ~(1 << number)

        self.board[row][col] = number
        self.board_bit[row][col] = 1 << number

    def init_board_bit(self):
        bit_max = (1 << (self.grid_size + 1)) - 1
        for row in range(self.grid_size):
            for col in range(self.grid_size):
                self.board_bit[row][col] = bit_max

        for row in range(self.grid_size):
            for col in range(self.grid_size):
                if self.board[row][col]:
                    self.set_board(row, col, self.board[row][col])

    def is_valid_fill(self, row: int, col: int, number: int) -> bool:
        if self.board[row][col] != 0:
            return False
        if not (self.board_bit[row][col] >> number) & 1:
            return False

        for i in range(self.grid_size):
            if i != col and self.board[row][i] == 0:
                if self.board_bit[row][i] & ~(1 << number) == 0:
                    return False
            if i != row and self.board[i][col] == 0:
                if self.board_bit[i][col] & ~(1 << number) == 0:
                    return False

        block_row = self.sqrt_grid_size * (row // self.sqrt_grid_size)
        block_col = self.sqrt_grid_size * (col // self.sqrt_grid_size)
        for i in range(self.sqrt_grid_size):
            for j in range(self.sqrt_grid_size):
                r = block_row + i
                c = block_col + j
                if r == row and c == col:
                    continue
                if self.board[r][c] == 0:
                    if self.board_bit[r][c] & ~(1 << number) == 0:
                        return False

        return True

    def search_board(self) -> Tuple[bool, Tuple[int, int]]:
        is_solved = True
        min_square = (-1, -1)
        min_num_valid = self.grid_size + 1

        for row in range(self.grid_size):
            for col in range(self.grid_size):
                if self.board[row][col] == 0:
                    is_solved = False
                    num_valid = bin(self.board_bit[row][col]).count("1")
                    if num_valid < min_num_valid:
                        min_num_valid = num_valid
                        min_square = (row, col)

        return is_solved, min_square

    def dfs_fill(self, row: int, col: int) -> bool:
        tmp_row = self.board_bit[row][:]
        tmp_col = [self.board_bit[i][col] for i in range(self.grid_size)]
        block_row = self.sqrt_grid_size * (row // self.sqrt_grid_size)
        block_col = self.sqrt_grid_size * (col // self.sqrt_grid_size)
        tmp_square = [
            [
                self.board_bit[block_row + i][block_col + j]
                for j in range(self.sqrt_grid_size)
            ]
            for i in range(self.sqrt_grid_size)
        ]

        for number in range(1, self.grid_size + 1):
            if self.is_valid_fill(row, col, number):
                self.set_board(row, col, number)

                next_search = self.search_board()
                if next_search[0]:
                    self.solution = [row[:] for row in self.board]
                    return True

                next_row, next_col = next_search[1]
                if self.dfs_fill(next_row, next_col):
                    return True
                else:
                    self.board[row][col] = 0
                    self.board_bit[row] = tmp_row[:]
                    for i in range(self.grid_size):
                        self.board_bit[i][col] = tmp_col[i]
                    for i in range(self.sqrt_grid_size):
                        for j in range(self.sqrt_grid_size):
                            r = block_row + i
                            c = block_col + j
                            self.board_bit[r][c] = tmp_square[i][j]

        return False

def parse_board(lines: List[str]) -> List[List[int]]:
    grid_size = int(lines[0].strip())
    assert 0 < grid_size < len(lines), f"grid_size must be > 0, but get {grid_size}"

    board = [] # [[] for _ in range(grid_size)]
    lines_board = lines[1:1+grid_size]

    for line in lines_board:
        row = list(map(int, line.strip().split()))
        assert len(row) == grid_size
        board.append(row)
    
    return board

def read_board_from_file(path: Path) -> List[List[int]]:
    """
    入力ファイルは1行目にgrid_size, 2行目以降に盤面が記入されているものとする
    例 (https://gigazine.net/news/20100822_hardest_sudoku/):
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
    """
    with open(path, "r") as f:
        lines = f.readlines()
    return parse_board(lines)

def main(board: List[List[int]]) -> List[List[int]]:    
    sv = SolverNP(board)
    answer = sv.solve()
    
    if not answer:
        print("not solved")
    else:
        for row in answer:
            print(f'{" ".join(map(str, row))}')
    
    return answer

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser()
    parser.add_argument("--request", action='store_true')
    args = parser.parse_args()
    
    if args.request:
        response = {
            "solution": "",
            "message": "",
            "error": "",
        }

        try:
            lines = sys.stdin.read().strip().split("\n")
            lines = [str(len(lines)), *lines]
            # response["input"] = lines
            response["debug"] = 1
            board = parse_board(lines)
            response["debug"] = 2
            with open(os.devnull, 'w') as fnull:
                with contextlib.redirect_stdout(fnull):
                    response["debug"] = 3
                    # response["board"] = board
                    answer = main(board)
                    response["debug"] = 4
            response["solution"] = answer
            response["message"] = "Solved."
            response["debug"] = 5
        except Exception as e:
            response["error"] = str(e)
            response["message"] = "Not Solved."
            # print(str(e), file=sys.stderr)

        # print("1", file=sys.stderr)
        print(json.dumps(response))
        # print("2", file=sys.stderr)
        # print("123")
        # sys.exit()

    else:
        board = read_board_from_file(Path("input.txt"))
        main(board)




