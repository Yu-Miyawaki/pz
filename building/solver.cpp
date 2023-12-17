#include <bits/stdc++.h>
using namespace std;
using ll = long long;
bool DEBUG = false;

class Timer{
public:
    Timer(){
        timer = clock();
    }
    void call(const std::string &s){
        clock_t now = clock();
        double dt = static_cast<double>(now - timer) / CLOCKS_PER_SEC * 1000.0;
        if(!s.empty()) cout << s << ": ";
        cout << dt << "[ms]" << endl;
        timer = clock();
    }
    void set(){
        timer = clock();
    }

private:
    clock_t timer;
};

class solver{
public:
    int n;
    int n1;
    // 0 ~ n, 下位が手前
    int bit_all;
    vector<vector<int>> board;
    // (4, n)
    vector<vector<int>> board_bit;
    vector<vector<int>> hint;
    // 0: 0あり 1:なし
    vector<vector<bool>> ok_bit;
    // 0ありのみ
    vector<vector<bool>> ok_bit_hint;

    vector<int> pn1{1};
    Timer timer;

    solver(vector<vector<int>> hint_){
        n = hint_[0].size();
        n1 = n+1;
        bit_all = 1;
        for(int _=0; _<n; _++){
            bit_all *= n1;
            int bc = pn1.back();
            pn1.push_back(n1 * bc);
            // overflow確認
            assert(bc < n1*bc);
        }
        hint = hint_;
        board.assign(n, vector<int>(n, 0));
        board_bit.assign(4, vector<int>(n, 0));
        ok_bit.resize(bit_all, vector<bool>(2));
        ok_bit_hint.assign(bit_all, vector<bool>(n1, false));
    }

    vector<vector<int>> solve(){
        timer.set();

        for(int bit=0; bit<bit_all; bit++){
            ok_bit[bit][0] = is_valid_bit(bit, false);
            ok_bit[bit][1] = is_valid_bit(bit, true);
        }

        timer.call("init_1");

        init_ok_bit_hint();

        if(DEBUG){
            if(n >= 6) cerr << "big size" << endl;
            else{
                cerr << "bit_all: " << bit_all << endl;
                cerr << "n, n1: " << n << " " << n1 << endl;
                cerr << "ok_bit:\n";
                // v2cerr(ok_bit);
                for(int bit=0; bit<bit_all; bit++) cerr << bit << ": " << ok_bit[bit][0] << " " << ok_bit[bit][1] << endl;
                cerr << "\nok_bit_hint:\n";
                // v2cerr(ok_bit_hint);
                for(int bit=0; bit<bit_all; bit++){
                    auto num = get_num(bit);
                    cerr << bit << ": ";
                    for(int i=0; i<4; i++) cerr << ok_bit_hint[bit][i] << " ";
                    cerr << "       ";
                    for(int i=0; i<n; i++) cerr << num[i] << " ";
                    cerr << endl;
                }
                // cerr << "\nlast_bit:\n";
                // for(const int bit : ok_bit_vec){
                //     auto num = get_num(bit);
                //     cerr << bit << ": ";
                //     REP(i, n) cerr << num[i] << " ";
                //     cerr << endl;
                // }
            }
        }

        timer.call("init_2");

        if(dfs(0)){
            if(DEBUG){
                cerr << "\nboard_bit:\n";
                for(int i=0; i<4; i++){
                    for(int j=0; j<n; j++){
                        cerr << board_bit[i][j] << " ";
                    }
                    cerr << endl;
                }
            }
            timer.call("solved");
            return board;
        }

        timer.call("not_solved");
        return board;

    }

    bool dfs(int row){
        if(row>=n){
            // REP(i, 4){
            //     REP(j, n){
            //         if(!ok_bit[board_bit[i][j]][1]) return false;
            //         if(hint[i][j] && !ok_bit_hint[board_bit[i][j]][hint[i][j]]) return false;
            //     }
            // }
            return true;
        }
        else{
            vector<int> index(n,0);
            for(int i=0; i<n; i++) index[i] = i+1;
            do{
                bool ok = true;
                for(int i=0; i<n; i++){
                    // REP(direction, 4){
                    ok &= set_bit_alldir(row, i, index[i]);
                    // }
                }
                if(ok){
                    // if(DEBUG){
                    //     cerr << row << endl;
                    // }
                    if(dfs(row+1)) return true;
                    else{
                        for(int i=0; i<n; i++) reset_bit_alldir(row, i);
                    }
                }
                else{
                    for(int i=0; i<n; i++) reset_bit_alldir(row, i);
                    // return false;
                }
            }while(next_permutation(index.begin(), index.end()));
            return false;
        }
    }

    void init_ok_bit_hint(){
        ok_bit_hint_set.resize(bit_all);
        set<int> now, next_search;
        // 完成形
        for(int bit=0; bit<bit_all; bit++) if(ok_bit[bit][1]){
            ok_bit_vec.push_back(bit);

            ok_bit_hint_set[bit].insert(get_hint(bit));
            next_search.insert(bit);
        }

        // TODO: 内部マスも初期設定可能にする

        // popcount(?)が大きい順に行う
        while(!next_search.empty()){
            assert(now.empty());
            if(*next_search.begin() == 0) break;
            swap(now, next_search);
            while(!now.empty()){
                int bit = *now.begin();
                now.erase(now.begin());
                for(int i=0; i<n; i++){
                    int k = (bit / pn1[i]) % n1;
                    if(k){
                        int ne = reset_bit(bit, i, k);
                        next_search.insert(ne);
                        for(const int x : ok_bit_hint_set[bit]){
                            // if(DEBUG){
                            //     cerr << "init: " << bit << " " << ne << " " << x << endl;
                            // }
                            ok_bit_hint_set[ne].insert(x);
                        }
                    }
                }
            }
        }

        for(int bit=0; bit<bit_all; bit++){
            if(ok_bit[bit][0]){
                ok_bit_hint[bit][0] = true;
                for(const int x : ok_bit_hint_set[bit]){
                    ok_bit_hint[bit][x] = true;
                }
            }
        }
    }
    bool is_valid_bit(int bit, bool zero_ng){
        vector<int> num(n1, 0);
        for(int i=0; i<n; i++){
            int k = bit % n1;
            if(k && num[k]) return false;
            if(zero_ng && !k) return false;
            num[k]++;
            bit /= n1;
        }
        return true;
    }
    // 変更は必ず行う
    bool set_bit_alldir(int y, int x, int k){
        bool ok = true;
        assert(!board[y][x]);
        board[y][x] = k;
        // // 0: up
        // board_bit[0][x] = set_bit(board_bit[0][x], y, k);
        // if(hint[0][x]) ok &= ok_bit_hint[board_bit[0][x]][hint[0][x]];
        // // 1: down
        // board_bit[1][x] = set_bit(board_bit[1][x], n-1-y, k);
        // if(hint[1][x]) ok &= ok_bit_hint[board_bit[1][x]][hint[1][x]];
        // // 2: left
        // board_bit[2][y] = set_bit(board_bit[2][y], x, k);
        // if(hint[2][y]) ok &= ok_bit_hint[board_bit[2][y]][hint[2][y]];
        // // 3: right
        // board_bit[3][y] = set_bit(board_bit[3][y], n-1-x, k);
        // if(hint[3][y]) ok &= ok_bit_hint[board_bit[3][y]][hint[3][y]];
        for(int i=0; i<4; i++){
            pair<int,int> p = get_target(y, x, i);
            board_bit[i][p.first] = set_bit(board_bit[i][p.first], p.second, k);
            if(true || hint[i][y]){
                ok &= ok_bit_hint[board_bit[i][p.first]][hint[i][p.first]];
            }
        }

        return ok;
    }
    void reset_bit_alldir(int y, int x){
        int k = board[y][x];
        assert(k);
        board[y][x] = 0;
        for(int i=0; i<4; i++){
            pair<int, int> p = get_target(y, x, i);
            board_bit[i][p.first] = reset_bit(board_bit[i][p.first], p.second, k);
        }
    }
    int set_bit(int bit, int idx, int k){
        return bit + (pn1[idx] * k);
    }
    int reset_bit(int bit, int idx, int k){
        return bit - (pn1[idx] * k);
    }
    int reset_bit_(int bit, int idx){
        int k = (bit / pn1[idx]) % n1;
        return set_bit(bit, idx, -k);
    }

    int get_hint(int bit){
        assert(ok_bit[bit][1]);
        int MAX = 0;
        int answer = 0;
        for(int i=0; i<n; i++){
            int k = bit % n1;
            answer += (MAX < k);
            MAX = max(MAX, k);
            bit /= n1;
        }
        return answer;
    }

    // board[i][target_1], target_2=idx
    pair<int, int> get_target(int y, int x, int direction){
        int target_1 = y;
        if(direction < 2) target_1 = x;
        int target_2 = -1;
        if(direction == 0) target_2 = y;
        else if(direction == 1) target_2 = n-1-y;
        else if(direction == 2) target_2 = x;
        else if(direction == 3) target_2 = n-1-x;
        return make_pair(target_1, target_2);
    }
    vector<int> get_num(int bit){
        vector<int> num(n);
        for(int i=0; i<n; i++){
            int k = (bit / pn1[i]) % n1;
            num[i] = k;
        }
        return num;
    }

    void display(){
        cout << "answer: " << endl;
        cout << "  ";
        for(int i=0; i<n; i++) cout << hint[0][i] << " ";
        cout << endl;
        for(int i=0; i<n; i++){
            cout << hint[2][i] << " ";
            for(int j=0; j<n; j++) cout << board[i][j] << " ";
            cout << hint[3][i] << endl;
        }
        cout << "  ";
        for(int i=0; i<n; i++) cout << hint[1][i] << " ";
    }

private:
    // init_hintを行うための準備
    vector<int> ok_bit_vec;
    vector<set<int>> ok_bit_hint_set;

};

int main(){
    // DEBUG = true;
    ifstream LOCAL_INPUT("input.txt"); cin.rdbuf(LOCAL_INPUT.rdbuf());
    int n;
    cin >> n;
    // 上,下,左,右
    vector<vector<int>> hint(4, vector<int>(n));
    for(int i=0; i<4; i++){
        for(int j=0; j<n; j++) cin >> hint[i][j];
    }
    solver sv(hint);
    sv.solve();
    sv.display();
    return 0;
}

// TODO: makerの作成

/*

https://puzsq.logicpuzzle.app/puzzle/101439
4
1 0 4 0
0 0 0 0
0 3 0 2
0 1 0 0


*/