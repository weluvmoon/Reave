#!/bin/bash
# A script to build the project and run the game if successful.

# Ensure we use ccache if available in the path
export PATH="/usr/lib/ccache:$PATH"

echo "--- Starting Build Process ---"

# Run the 'make' command.
# The '&&' operator means the next command will only run if 'make' exits with a 0 status (success).
make && {
    echo "--- Build Successful! Starting Game ---"
    ghostty -e gdb ./game
}

# The following line will only execute if 'make' failed (non-zero exit status)
if [ $? -ne 0 ]; then
    echo "--- Build Failed. Game not launched. ---"
fi
