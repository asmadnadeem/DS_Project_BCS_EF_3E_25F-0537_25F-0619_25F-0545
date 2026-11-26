# MayDay-404: Real-Time Console Aviation Simulator
**Course:** CL2001 - Data Structures Lab (Fall 2026)  
**Institution:** FAST-NUCES Chiniot-Faisalabad Campus

## Team Members
- **Asmad Nadeem (`25F-0537`):** Module 1 (Registry + Linear Probing Hash Table), Module 2 (Landing Queue + Emergency Min-Heap), Module 3 (Command History DLL + Undo Stack), Data Files
- **Faisal Shahzad (`25F-0619`):** Module 4 (BST Manifest), Module 5 (AVL Index), Module 6 (Airspace Graph), Module 7 (BFS/DFS Radar), Module 8 (Dijkstra), Module 9 (Kruskal MST)
- **Umar Saleem (`25F-0545`):** Module 10 (Game Engine & 30-Tick Loop), Module 11 (Leaderboard Persistence)

## Build & Run (MS Visual Studio C++)
1. Create an **Empty C++ Console Project** in Visual Studio.
2. Add all `.h` and `.cpp` files from `src/` into the project.
3. Place all 9 `.txt` files in the same working directory as the `.vcxproj` / executable.
4. Build and run (`Ctrl + F5`).

## Known Limitations
- Console input expects underscores (`_`) instead of spaces for multi-word sector names during manual prompts.
- Single-player turn-based tick progression (each turn simulates 30 seconds).
- Fixed maximum capacity of 20 airspace sectors and 101 hash table slots.
