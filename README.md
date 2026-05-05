# J.A.R.V.I.S.

A lightweight C-based interactive AI terminal assistant that bridges the local shell with the Google Gemini generative language API.

## Overview

J.A.R.V.I.S. is built as a command-line interface that sends user prompts to the Gemini model, receives structured JSON responses, and optionally executes generated shell commands after explicit user confirmation. It also feeds command output back into the AI to support multi-step automation.

## Key Features

- Interactive prompt loop for continuous user input
- Sends requests to the Google Gemini API via `libcurl`
- Builds structured JSON payloads using `cJSON`
- Parses AI responses to extract both a human-readable message and a JSON `command`
- Prompts the user before executing any generated shell command
- Captures command output and injects it back into the next AI request for iterative task completion
- Supports `clear` and `exit` commands locally

## Project Structure

- `src/main.c` - main application loop and AI orchestration
- `src/network.c` - network communication with the AI endpoint using `libcurl`
- `src/os_utils.c` - running shell commands safely and capturing output
- `src/payload_handler.c` - building request payloads and parsing AI JSON responses
- `include/network.h` - network API declarations
- `include/os_utils.h` - command execution definitions and size limits
- `include/payload_handler.h` - payload builder/parsing definitions
- `Makefile` - build rules for compiling the executable

## Dependencies

The project requires the following libraries:

- `gcc`
- `libcurl` development headers
- `libcjson` development headers

On Debian/Ubuntu, install dependencies with:

```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev libcjson-dev
```

## Build Instructions

From the project root:

```bash
make
```

This produces the executable at `bin/jarvis`.

## Running J.A.R.V.I.S.

Before launching, set your API key in the environment variable `JARVIS_API_KEY`:

```bash
export JARVIS_API_KEY="your_api_key_here"
```

Then start the program:

```bash
make run
```

If the environment variable is missing, the program prints a warning and exits.

## Usage

- Type any natural language request at the prompt
- The assistant will send the prompt to Gemini and display the returned message
- If Gemini returns a `command`, you will be asked to confirm execution
- Enter `y` to run the command or `n` to cancel
- Enter `clear` to clear the terminal output
- Enter `exit` to quit

## Notes

- The AI endpoint is configured to use `https://generativelanguage.googleapis.com/v1beta/models/gemini-3.1-pro-preview:generateContent`
- The app assumes the Gemini response follows a JSON-within-JSON structure and extracts `command` and `message` fields
- Command execution is performed via `popen()` with stderr merged into stdout

## Cleanup

To remove build artifacts:

```bash
make clean
```
