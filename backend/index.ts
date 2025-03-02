
import * as express from "express";
import * as cors from "cors";
import { spawn } from "child_process";
import * as bodyParser from "body-parser";

const app = express();
app.use(cors());
app.use(express.json());
app.use(bodyParser.json());

// frontend, Pythonプロセスとやりとりを行う
app.post("/api/solve/number_place", (req, res) => {
    const board = req.body.board;

    const pythonProcess = spawn("python", ["../number_place/solver.py", "--request"]);

    pythonProcess.stdin.write(board.join("\n"));
    pythonProcess.stdin.end();

    let output = "";
    let errorOutput = "";
    pythonProcess.stdout.on("data", (data) => {
        output += data.toString();
    });

    pythonProcess.stderr.on("data", (data) => {
        errorOutput += data.toString();
    });

    pythonProcess.stdout.on("close", (code) => {
        // 本来0が返るので===0だがなぜかcodeがfalseになっている
        if (code == 0){
            console.log(`code: ${code}\nError: ${errorOutput}\noutput: ${output}`);
            res.json({solution: JSON.parse(output).solution
                // , error: JSON.parse(errorOutput)
            });
        }
        else{
            // stderr出力された場合はHTML 500エラー(Internal Server Error)を返す
            console.error(`code: ${code}\nError: ${errorOutput}\noutput: ${output}`);
            res.status(500).send({ error: errorOutput || 'Internal server error' });
        }
    });
});

// 直接アクセスするには/のエンドポイントが必要
app.get("/", (req, res) => {
    res.send("Express server is running!");
});

// React, Next.jsでのデフォルトが3000であり、Expressでは5000が慣習的に使われる
// 5000はMacのAirPlay機能
const port = 5001;
// Node.jsの環境変数であるprocess.envを用いてprocess.env.PORT || 5000;のようにも書ける
app.listen(port, () => {
    console.log(`Backend running on http://localhost:${port}`);
});
