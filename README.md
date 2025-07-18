# 🗺️ Treasure Hunt – UNIX C Game System

[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))  
📁 UNIX-based treasure hunt simulation developed in C across three phases, combining file handling, process management, signals, and inter-process communication.

---

## 📌 Overview

This project simulates a digital treasure hunt game. It allows users to manage hunt sessions and treasures via file operations, interact with a live monitoring process through signals, and compute player scores using external programs and pipes.

> 💡 Developed as part of the System Programming lab at Politehnica University of Timișoara.

---

## 🧱 Components

| Phase | Component          | Description                                                                 |
|-------|--------------------|-----------------------------------------------------------------------------|
| 1     | `treasure_manager` | CLI tool to create/manage hunts & treasures via system calls               |
| 2     | `treasure_hub`     | Interactive shell communicating with a monitor process using `sigaction()` |
| 3     | `score_calculator` | External tool computing per-user treasure values via pipe output           |

---

## 🔧 Technologies & Concepts

- `C (POSIX)`, `bash`, `make`
- `open()`, `read()`, `write()`, `lseek()`, `stat()`, `mkdir()`
- `fork()`, `exec()`, `sigaction()`, `kill()`, `waitpid()`
- `pipes`, `signals`, `process management`, `file I/O`, `binary data formats`

---

## 🎮 Features by Phase

<details>
  <summary><strong>📁 Phase 1 – File System</strong></summary>

- `add`, `list`, `view`, `remove_treasure`, `remove_hunt` commands
- Structured binary file format for storing treasures (ID, user, GPS, clue, value)
- Logs actions in `logged_hunt` files per hunt directory
- Creates symlinks like `logged_hunt-Hunt001` at root level
</details>

<details>
  <summary><strong>🧠 Phase 2 – Signals & Process Control</strong></summary>

- `treasure_hub` manages user input and controls a monitor background process
- Monitor handles commands: `list_hunts`, `list_treasures`, `view_treasure`
- Uses `SIGUSR1`, `SIGCHLD` for communication and termination sync
</details>

<details>
  <summary><strong>🔁 Phase 3 – Pipes & Score Calculation</strong></summary>

- Pipes redirect monitor output back to the main process
- New command: `calculate_score`
- Spawns a child process per hunt to run `score_calculator`
- Aggregates user scores based on treasure value
</details>

---
