# SurStromming

- Summer 2023 Systems Software project. Implementing a toolchain and an emulator for an abstract, RISC-like computer system.

## Key Features

- **A custom assembler:** Compiling individual assembly language source files
- **A linker:** Links multiple files into a single executable FISH32 format file
- **An emulator:** Capable of running the executable FISH32 files
- **Herring:** A tool, similar to readelf, designed for viewing the contents of FISH32 files.

## Dependencies

- bison
- flex
- a g++ compiler

## Build

To build the project simply run the following from your terminal:
```bash
make all
```

## Usage

Each tool comes with its unique set of options, to view those options run the following for each tool:
```bash
./assembler --help
./linker --help
./emulator --help
./herring --help
```

## Testing

Navigate to the ```test``` directory to find the level A tests. To initiate the test, execute the following command:
```bash
cd test/nivo-a && ./start.sh
```

