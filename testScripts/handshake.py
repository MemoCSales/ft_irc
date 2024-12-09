import hexchat

WHITE = "\00300"
BLACK = "\00301"
BLUE = "\00302"
GREEN = "\00303"
RED = "\00304"
BROWN = "\00305"
PURPLE = "\00306"
ORANGE = "\00307"
YELLOW = "\00308"
LIGHT_GREEN = "\00309"
TEAL = "\00310"
LIGHT_CYAN = "\00311"
LIGHT_BLUE = "\00312"
PINK = "\00313"
GREY = "\00314"
LIGHT_GREY = "\00315"
ENDC = "\003"


__module_name__ = "On Handshake"
__module_version__ = "1.0"
__module_description__ = "Prints a message when the handshake is established"
connection_established = False
message = []
#--------------------------------------------------------------------------------
def send_message(nick):
	print()
	hexchat.command(f"msg {nick} {nick}test")
	return False
#--------------------------------------------------------------------------------
def hexChatMsg():
	context = hexchat.get_context()
	if context is not None:
		network = context.get_info("network")
		server = context.get_info("server")
		nick = context.get_info("nick")
		user = context.get_info("user")
		realname = context.get_info("realname")
		context_id = context.get_info("contextid")
		
		hexchat.prnt(f"\n{LIGHT_GREY}Connected to network: {network}{ENDC}")
		hexchat.prnt(f"{LIGHT_GREY}Server address: {server}{ENDC}")
		hexchat.prnt(f"{LIGHT_GREY}Nick: {nick}{ENDC}")
		hexchat.prnt(f"{LIGHT_GREY}User name: {user}{ENDC}")
		hexchat.prnt(f"{LIGHT_GREY}Real name: {realname}{ENDC}")
		hexchat.prnt(f"{LIGHT_GREY}Context ID: {context_id}{ENDC}")
		hexchat.hook_timer(1000, send_message, nick)
	else:
		hexchat.prnt(f"{RED}Failed to get current context{ENDC}")
	return False
#--------------------------------------------------------------------------------

def on_handshake(word, word_eol, userdata):
	global connection_established
	global message
	if not word_eol:
		message.append("\n")
	if word_eol:
		message.append(word_eol[0])
		if "NOTICE" in word_eol[0]:
			hexchat.prnt(f"{YELLOW}NOTICE{ENDC}")
		elif "ERROR" in word_eol[0]:
			hexchat.prnt(f"{RED}ERROR{ENDC}")
		elif "Welcome" in word:
			if not connection_established:
				hexchat.prnt(f"{LIGHT_GREY}------------- HANDSHAKE WITH SERVER -------------{ENDC}")
				connection_established = True
			for line in message:
				hexchat.emit_print("Server Text", f"\t{BLUE}{line}{ENDC}")
			hexChatMsg()
			# try:
			# 	message = message.encode('ascii', 'ignore').decode('ascii')
			# except UnicodeEncodeError:
			# 	message = message  # Fallback to original message if encoding fails
			# char_count = len(message)
			# print("{} (len:{})".format(message, char_count))
			# hexchat.prnt(f"{message} ({char_count} |eol: {len(word_eol)} | word: {len(word)} )")

	return hexchat.EAT_ALL
#--------------------------------------------------------------------------------
# Hook into server messages
hexchat.hook_server("RAW LINE", on_handshake)
# hexchat.hook_print("Server Text", on_handshake)
#--------------------------------------------------------------------------------
# print(__module_name__, "version", __module_version__, "loaded.")
# hexChatMsg()

