MODULE(
	"ping", // name
	{       // handlers
		{"ibotpp", "ping", [] (bot bot, std::string str) {
			bot.send("PONG", str);
		}}
	}
)
