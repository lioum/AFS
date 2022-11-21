# Distributed Algorithm File System
Project for the Distributed Algorithm class at EPITA.

Raft Consensus Algorithm with log replication and leader election.

Authors:
- Pierre Seguin
- Jiayi Hao
- Liliam Jean-Baptiste
- Arthur Le Bourg
- Antoine Delattre
- Thibaut Ambrosino

Supervised by:
- Etienne Renault

## 1. Requirements
To use this project, you should install OpenMPI.
To do so, you can install the `openmpi` package for Arch-based systems or  `libopenmpi-dev` for Debian-based systems.
You will also need a C++ compiler supporting the C++20 version.

## 2. Build
To build the program, you can run the following command:
```bash
make
```

In order to build the program in debug mode, you need to use the Debug Makefile:
```bash
make -f Debug
```


## 3. Run the program
To run the program, you can use the run.sh script:
```bash
chmod +x run.sh
./run.sh
```

If you want to build and run the program in one command, you can use the following command:
```bash
make exe
```

To debug the program, you can use the debug.sh script:
```bash
chmod +x debug.sh
./debug.sh
```
You will need to modify the `debug.sh` script to change the konsole with your favorite terminal.

same for debug mode:
```bash
make -f Debug exe
```

## 3.1 REPL controller
The REPL controller is a command line interface that allows you to interact with the program.
It is used to send messages to the processus and to display the current state of the program.
Here is the list and description of the commands:
- `CRASH <uid>`: crash the processus
- `SPEED <speed> <uid>`: change the speed of the processus
- `START <uid>`: start the processus (usage only on client since server are started by default)
- `RECOVER <uid>`: recover the processus from a crashed state
The command line will guide you through the parameter to pass to these commands.

If you use the `run.sh` script, you can redirect the output of the logs to another terminal.

```bash
# from the other terminal, execute this to get the tty to get its value
$ tty
/dev/pts/1

# On the first orignal terminal, execute this
$ ./run.sh > /dev/pts/1
```


## 3.2 Client Requests
All the client requests are stored in the `client_folder` folder each in a separate folder corresponding to their possible uid.
The client requests are stored in a file named `commands.txt` and are formatted as follow:
```bash
LOAD commands.txt
LIST
APPEND commands.txt MyBeautifulText
LIST
```
One command per line.
- `LOAD <filename>`: load a file containing a list of commands.
- `LIST`: display the current state of the client.
- `APPEND <filename> <some text>`: append a text to a given file.
- `DELETE <filename>`: delete a file.

## 4. Checking log consistancy
To check log consistancy you can run the script compare_logs.sh which will run a diff command between every logs.