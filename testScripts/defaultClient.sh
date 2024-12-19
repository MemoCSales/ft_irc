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

# pkill -P $$ nc
# Set the trap to catch SIGINT (Ctrl+C)
# trap cleanup SIGINT
# # Function to handle signal interruptions
# cleanup()
# {
# 	echo "Signal received. Cleaning up..."
# 	# exec 0</dev/null
# 	# pkill -P $$ nc 2>/dev/null
# 	pkill -P $NC_PID #2>/dev/null
# 	# kill $NC_PID #2>/dev/null
# 	wait $NC_PID 2>/dev/null
# 	exit 1 
# }

# # Function to read from the ser

# # Connect to the IRC server
# # Function to read from the server and send input
# # read_and_send() {
# # 	while true; do
# # 		if ! kill -0 $NC_PID 2>/dev/null; then
# # 			# echo "Connection to server lost." <&0 
# # 			break
# # 		fi
# # 		read -r input #<&0 2>/dev/null
# # 		echo "$input" <&0
# # 	done
# # }

# # Connect to the IRC server
# (
#	 sleep 1
#	 echo "PASS $PASSWORD"
#	 sleep 1
#	 echo "NICK $NICK"
#	 sleep 1
#	 echo "USER $USER 0 * :$REALNAME"
#	 sleep 1
# 	while true; do
# 		if ! read -r input <&0 ; then
# 			if [ $? -eq 1 ]; then 
# 				break
# 			fi
# 			break
# 			# break
# 		fi
# 		# read -r input <&0 >/dev/null;
# 		echo "$input"
# 		if [ "$input" == "QUIT" ]; then
# 			# echo "QUIT :Client exiting"
# 			break
# 		fi
# 	done
# 	# exit 0 >/dev/null;
# 	exec 1>&-
# ) | nc $SERVER $PORT &
# NC_PID=$!
# wait $NC_PID 2>/dev/null

		# if [ "$input" != "" ]; then
		# 	echo "$input"
		# else
		# 	break 
		# fi
		# read -t 1 -p ">$" input
		# if [ "$input" != "" ]; then
		# 	echo "$input"
		# else
		# 	exit 0
		# fi
		# echo "$input"
		# if [ "$input" == *"Server is shutting down."* ]; then
		# 	echo "*$input"
		# 	pkill -P $$ nc
		# 	break
		# fi
		# if [ "$input" == *"Quit"* ]; then
		# 	pkill -P $$ nc && exit 0
		# 	break
		# fi
		#  exec 1>&-


# (
# 	sleep 1
# 	echo "PASS $PASSWORD"
# 	sleep 1
# 	echo "NICK $NICK"
# 	sleep 1
# 	echo "USER $USER 
# 	while true; do
# 		# Read from server
# 		read -p ">$ " input #<&0 2>/dev/null
# 		if [ "$input" != "" ]; then
# 			echo "$input" <&0
# 		fi 
# 		if ! kill -0 $NC_PID 2>/dev/null; then 
# 			break
# 			# pkill -P $$ nc
# 		fi
# 		# Check if the connection is still alive 
# 		# read -r  input <&0 #2>/dev/null
# 		# echo "$input"
# 	done
# 	# exec 1>&-
# ) | nc $SERVER $PORT &
# NC_PID=$!
# wait $NC_PID  2>/dev/null