"use client";

// import Image from "next/image";
// import styles from "./page.module.css";

import { useState } from "react";

export default function Home(){
  const [board, setBoard] = useState<string>("");
  const [solution, setSolution] = useState<string>("");

  const solveNP = async () => {
    const port = 5001;
    const response = await fetch(`http://localhost:${port}/api/solve/number_place`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ board: board.split("\n") }),
    });
    const data = await response.json();
    setSolution(JSON.stringify(data.solution));
  };

  return (
    <div>
      <h1>number place solver</h1>
      <textarea
        value = {board}
        onChange = {(e) => setBoard(e.target.value)}
        placeholder = "input board (0 is blank)"
      />
      <button onClick = {solveNP}>Solve</button>
      <pre>{solution}</pre>
    </div>
  );
}

