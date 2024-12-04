import subprocess
import random
import os
import time

BLUE = '\033[94m'
CYAN = '\033[96m'
MAG = '\033[95m'
GREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'

def modify_hexchat_config():
	# Generate random user data with the same unique number
	UNIQUE_NUMBER = random.randint(1000, 9999)
	NICK = f"user{UNIQUE_NUMBER}"
	USERNAME = f"user{UNIQUE_NUMBER}"
	REALNAME = f"Real{UNIQUE_NUMBER}"
	SERVER = "localhost"
	PORT = 6667  # Non-SSL port
	# NETWORK_PREFIX = "network"
	# NETWORK = f"{NETWORK_PREFIX}{UNIQUE_NUMBER}"

	print(f"server: {MAG}irc{ENDC}")
	print(f"Nick: {BLUE}{NICK}{ENDC}")
	print(f"Username: {BLUE}{UNDERLINE}{USERNAME}{ENDC}")
	# print(f"Generated Realname: {GREEN}{UNDERLINE}{REALNAME}{ENDC}")
	#-------------------------------------------------------------------
	USERS= "testScripts/users_created.txt"
	with open(USERS, "a") as file:
		file.write(f"{NICK}\n")
	#-------------------------------------------------------------------

	servlist_path = os.path.expanduser("~/.var/app/io.github.Hexchat/config/hexchat/servlist.conf")


	print(f"Writing to servlist.conf: {servlist_path}")
	with open(servlist_path, "r") as servlist_file:
		lines = servlist_file.readlines()
	network_exists = any(line.startswith(f"N=irc\n") for line in lines)
	# Write the existing lines back to the file and append the new network configuration if it doesn't exist
	with open(servlist_path, "w") as servlist_file:
		for line in lines:
			servlist_file.write(line)
		if not network_exists:
			servlist_file.write(f"\nN=irc\n")
			servlist_file.write(f"E=UTF-8 (Unicode)\n")
			servlist_file.write(f"F=115\n")
			servlist_file.write(f"D=0\n")
			servlist_file.write(f"S={SERVER}/{PORT}\n")

	#-------------------------------------------------------------------
	hexchat_config_path = os.path.expanduser("~/.var/app/io.github.Hexchat/config/hexchat/hexchat.conf")
	print(f"Modifying hexchat.conf: {hexchat_config_path}")
	with open(hexchat_config_path, "r") as hexchat_config_file:
		lines = hexchat_config_file.readlines()
		
	with open(hexchat_config_path, "w") as hexchat_config_file:
		for line in lines:
			if line.startswith(f"irc_nick1 ="):
				hexchat_config_file.write(f"irc_nick1 = {NICK}\n")
			elif line.startswith(f"irc_nick2 ="):
				hexchat_config_file.write(f"irc_nick2 = {NICK}_\n")
			elif line.startswith(f"irc_nick3 ="):
				hexchat_config_file.write(f"irc_nick3 = {NICK}__\n")
			elif line.startswith(f"irc_user_name ="):
				hexchat_config_file.write(f"irc_user_name = {USERNAME}\n")
			else:
				hexchat_config_file.write(line)
		
	print(GREEN,BOLD, UNDERLINE, "Configuration modified successfully.", ENDC)
	time.sleep(1)
	return NICK, USERNAME, REALNAME

#-------------------------------------------------------------------
nick, username, realname = modify_hexchat_config()
# Define the command to open a new HexChat window and run a script
# command = f'flatpak run io.github.Hexchat --existing--command="server {network}"'
command = f'flatpak run io.github.Hexchat  --command="server irc"'
print(MAG,f"command = {command}", ENDC)

# Execute the command in Bash
result = subprocess.run(command, shell=True, capture_output=True, text=True)

# Print the output of the command
print("Output:", result.stdout)
print(FAIL,"Error:", result.stderr, ENDC)
print("Return Code:", result.returncode)

#-------------------------------------------------------------------





'''
# chmod +r $(PWD)/testScripts/testHexChat.py
# mkdir -p $(HOME)/.var/app/io.github.Hexchat/config/hexchat/addons/
# cp $(PWD)/testScripts/testHexChat.py $(HOME)/.var/app/io.github.Hexchat/config/hexchat/addons/
#	flatpak run io.github.Hexchat -c "localhost" -p "6667" -n "testUser" -e "join #testChannel" -e "msg #testChannel this is a test"
#	python3 testScripts/modifyUserInfo.py 
#	flatpak run io.github.Hexchat --existing --command="py load $(HOME)/.var/app/io.github.Hexchat/config/hexchat/addons/testHexChat.py"

'''