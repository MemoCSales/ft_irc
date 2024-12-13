#!/bin/bash

# Configuration
SERVER="localhost"
PORT=6667
PASSWORD="42"
NICK="Vicki"
USER="Victoria"
REALNAME="VictoriaLizarraga"
CHANNEL="#YourChannel"
MESSAGE="Hello, IRC!"

# Function to handle signal interruptions
cleanup()
{
	echo "Signal received. Cleaning up..."
	pkill -P $$ nc
	exit 0
}

# Set the trap to catch SIGINT (Ctrl+C)
trap cleanup SIGINT

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
		if ! read -p "" input <&0; then
			if [ $? -eq 1 ]; then
				pkill -P $$ nc
			fi
			break
		fi
		echo "$input"
		# Break the loop if the input is "QUIT"
		if [ "$input" == "QUIT" ]; then
			echo "QUIT :Client exiting"
			pkill -P $$ nc
			break
		fi
	done
	exec 1>&-
) | nc $SERVER $PORT &
wait $!