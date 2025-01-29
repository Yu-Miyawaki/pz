
from typing import List, Optional

import streamlit as st
from streamlit.runtime.uploaded_file_manager import UploadedFile
import numpy as np

from solver import SolverNP, parse_board

# GPT利用

def sqrt(num: int) -> int:
    return int(num ** 0.5 + 0.1)

def is_square(num: int) -> bool:
    return sqrt(num) ** 2 == num

def is_valid_board(board: List[List[int]]):
    """ 各マス目が空白もしくは可能な数値か判定 """
    grid_size = len(board)
    if grid_size <= 1 or (not is_square(grid_size)):
        return False
    valid_numbers = set(range(0, grid_size + 1))
    for row in board:
        for num in row:
            if num not in valid_numbers:
                return False
    return True

def read_board_from_buffer(file: UploadedFile) -> List[List[int]]:
    lines = file.getvalue().decode("utf-8").splitlines()
    return parse_board(lines)

def visualize_board(board: List[List[int]]) -> None:
    grid_size = len(board)
    border_num = sqrt(grid_size)
    for i in range(grid_size):
        cols = st.columns(grid_size)
        for j in range(grid_size):
            # 出力時にも3×3の枠線をつける
            border_style = ""
            if (j+1) % border_num == 0 and j != grid_size - 1:
                border_style = "border-right: 2px solid black;"
            if (i+1) % border_num == 0 and i != grid_size - 1:
                border_style += "border-bottom: 2px solid black;"

            cols[j].markdown(f"<div style='text-align: center; {border_style} font-size: 20px;'>{board[i][j]}</div>", unsafe_allow_html=True)

def main():

    st.title("number place solver")

    # 入力方法の選択
    input_method = st.radio("select method", ["manual", "file"])

    # ファイルアップロード
    uploaded_file = None
    if input_method == "file":
        uploaded_file = st.file_uploader("upload file（.txt）", type=["txt"])

    # グリッドサイズの設定
    if uploaded_file:
        try:
            board = read_board_from_buffer(uploaded_file)
            if not is_valid_board(board):
                st.error(f"read board is invalid.")
                st.stop()
            grid_size = len(board)
            # visualize_board(board)
        except Exception as e:
            st.error(f"fail to read file: {e}")
            st.stop()
    else:
        grid_size = st.number_input("grid_size (e.g. 9)", min_value=4, max_value=36, value=9)
        if not is_square(grid_size):
            st.error(f"grid size must be square")
            st.stop()
        board = [[0 for _ in range(grid_size)] for _ in range(grid_size)]

    st.write("input board (0 is blank)")
    input_board = []
    border_num = sqrt(grid_size)

    for i in range(grid_size):
        cols = st.columns(grid_size)
        row_values = []
        for j in range(grid_size):
            # 視覚的な区切りを入れる（3×3ブロック）
            border_style = "padding: 5px; text-align: center; display: flex; justify-content: center; align-items: center;"
            if (j+1) % border_num == 0 and j != grid_size - 1:
                border_style += "border-right: 2px solid black;"
            if i % border_num == 0 and i != 0:
                border_style += "border-bottom: 2px solid black;"

            with cols[j]:
                st.markdown(f"<div style='{border_style}'>", unsafe_allow_html=True)
                cell_value = st.number_input("", min_value=0, max_value=grid_size, value=int(board[i][j]), step=1, key=f"cell_{i}_{j}")
                st.markdown("</div>", unsafe_allow_html=True)
            
            row_values.append(cell_value)
        input_board.append(row_values)

    # 解答ボタン
    if st.button("Solve"):

        if is_valid_board(input_board):
            sv = SolverNP(input_board)
            answer = sv.solve()
            if answer:
                st.write("Solved:")
                visualize_board(answer)
            else:
                st.error("No Answer")
        
        else:
            st.error(f"each input must be 0 ~ {grid_size**2}")

if __name__ == "__main__":
    main()
