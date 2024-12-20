#!/bin/bash

# Configuration
SERVER="localhost"
PORT=6667
PASSWORD="42"
# Lists of possible values
NICKS=("Vicki" "Memo" "Marian" "Test")
USERS=("Victoria" "Guillermo" "Marian" "Test")
REALNAMES=("VictoriaL" "GuillermoC" "MarianS" "TestR")

# Select random values
index=$((RANDOM % ${#NICKS[@]}))
NICK=${NICKS[$index]}
USER=${USERS[$index]}
REALNAME=${REALNAMES[$index]}
CHANNEL="#YourChannel"
MESSAGE="Hello, IRC!"

is_server_running()
{
	nc -z $SERVER $PORT
	return $?
}

# Set the trap to catch SIGINT (Ctrl+C)
trap cleanup SIGINT


cleanup()
{
	echo "Signal received. Cleaning up..."
	pkill -P $$ nc
	exit 1
	# wait $!
}


# Connect to the IRC server
(
	sleep 1
	echo "PASS $PASSWORD"
	sleep 1
	echo "NICK $NICK"
	sleep 1
	echo "USER $USER 0 * :$REALNAME"
	sleep 1
	# echo "PRIVMSG $CHANNEL :$MESSAGE"
	# echo "JOIN $CHANNEL"
	# sleep 1
	# sleep 1
	# echo "QUIT"
	# Connect to the IRC server
	while true; do
		if read -r input 2>/dev/null; then
			if [ $? -eq 1 ]; then 
				pkill -P $$ nc
				echo "Error: Server connection terminated" >&2
				exit 1
			fi
		fi
		if ! is_server_running; then
			echo "Error: Server is not running" 1>&2
			exit 1
		fi
		echo "$input"
		# Break the loop if the input is "QUIT"
		if [ "$input" == "QUIT" ]; then
			exit 0
		else
			if [ "$input" == *"Server is shutting down."* ]; then
				pkill -P $$ nc
				echo "*$input"
				exit 1
			fi
		fi
	done
) | nc $SERVER $PORT &
NC_PID=$!
wait $NC_PID && echo