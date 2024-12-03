import hexchat
import random
import string

__module_name__ = "AutoConnect"
__module_version__ = "1.0"
__module_description__ = "Automatically connects to an IRC server and sends a message"

SERVER = "localhost"
PORT = 6667
CHANNEL = "#testChannel"
MESSAGE = "This is a test message"

# Generate a random nickname
NICK = "user" + ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))

def on_connect(word, word_eol, userdata):
    hexchat.command(f"nick {NICK}")
    hexchat.command(f"join {CHANNEL}")
    hexchat.command(f"msg {CHANNEL} {MESSAGE}")
    return hexchat.EAT_ALL

hexchat.hook_server("001", on_connect)

hexchat.command(f"server {SERVER} {PORT} -ssl")