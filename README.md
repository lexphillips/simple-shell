# Simple Shell Interpreter (SSI)

A minimal Unix-like shell implemented in C.  
Supports launching external programs, running jobs in the background, and a small set of built-in commands (`cd`, `pwd`, `exit`, `bg`, `bglist`).  

---

## Features

- **Command execution**  
  - Runs standard Unix commands in the foreground.  
  - Handles process creation with `fork` + `execvp`.  

- **Background jobs**  
  - Start background processes with `bg`.  
  - View active jobs with `bglist`.  
  - Automatic cleanup of terminated jobs.  

- **Built-ins**  
  - `cd [dir]` — change directory (supports `-` for previous and `~` for home).  
  - `pwd` — print current working directory.  
  - `exit` — quit the shell.  
  - `bg <command>` — run a program in the background.  
  - `bglist` — show all active background jobs.  

- **Prompt**  
  - Displays `username@hostname:cwd >`  
  - Updates on each command.  

---

## Build & Run

### Prerequisites
- GCC or Clang  
- GNU Readline library (`libreadline-dev` on Debian/Ubuntu)

### Build
```bash
make
```

### Run
```bash
./ssi
```

---

## Usage Examples

```bash
user@host:/home/user > pwd
/home/user

user@host:/home/user > cd /tmp
user@host:/tmp > bg sleep 10
Starting background process: PID 12345

user@host:/tmp > bglist
12345: [/tmp] sleep 10
Total Background jobs: 1

user@host:/tmp > exit
```

---

## Project Structure

```
.
├── ssi.c          # Main loop
├── builtins.c     # Built-in command implementations
├── background.c   # Background job handling
├── utils.c        # Prompt, tokenizing, helpers
├── ssi.h          # Header definitions
├── Makefile       # Build rules
└── README.md
```

---

## Acknowledgements

Documentation created with assistance from AI (ChatGPT).
