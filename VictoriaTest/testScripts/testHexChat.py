import hexchat

__module_name__ = "WindowManager"
__module_version__ = "1.0"
__module_description__ = "Manages HexChat windows"

def get_current_context():
	context = hexchat.get_context()
	context_id = context.get_info("contextid")
	print(f"Current Context ID: {context_id}")
	hexchat.prnt(f"Current Context ID: {context_id}")
	return context_id

def my_command_callback(word, word_eol, userdata):
	hexchat.prnt("MYCOMMAND was called!")
	hexchat.prnt(f"word:{word}")
	hexchat.prnt(f":word_eol:{word_eol}")
	hexchat.prnt(f"user:{userdata}")
	return hexchat.EAT_ALL

def open_new_window():
	# hexchat.command("set irc_nick1 newuser")
	# hexchat.command("set irc_user_name newuser")
	# hexchat.command("set irc_real_name RealNewUser")
	hexchat.command("server irc")
	hexchat.hook_command("MYCOMMAND", my_command_callback)

	#-n newuser -u newuser -r RealNewUser
	hexchat.command("msg user text")


# Get the current context ID
current_context_id = get_current_context()

# Open a new window in cascade sequence
open_new_window()

'''
# Connect to the new network
# hexchat.command(f"server {SERVER} {PORT} -ssl=0")
# Connect to the configured network
# hexchat.command(f"server {NETWORK}")
# hexchat.command(f"server irc://{SERVER}:{PORT} -ssl=0")
# hexchat.command(f"server {SERVER} {PORT}")


# ----------------------------------------
import hexchat
import string

__module_name__ = "AutoConnect"
__module_version__ = "1.0"
__module_description__ = "Automatically creates a new network profile and connects to an IRC server with random user data"

# Generate a random network name and user data with the same number
# NETWORK_PREFIX = "network"

# NETWORK = f"{NETWORK_PREFIX}{UNIQUE_NUMBER}"
NETWORK = 'irc'

SERVER = "localhost"
PORT = 6667  # Non-SSL port
CHANNEL = "#testChannel"
MESSAGE = "This is a test message"


def on_connect(word, word_eol, userdata):
	hexchat.command(f"join {CHANNEL}")
	hexchat.command(f"msg {CHANNEL} {MESSAGE}")
	return hexchat.EAT_ALL

# Modify HexChat configuration to add a new network profile


# Introduce a delay to ensure the configuration changes are written
# hexchat.hook_timer(3000, lambda userdata: hexchat.command(f"server {NETWORK}"))

hexchat.hook_server("001", on_connect)
hexchat.command(f"server {NETWORK}")
'''