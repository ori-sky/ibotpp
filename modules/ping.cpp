MODULE(
	"ping", // name
	{       // handlers
		{"ping", [] (const bot &bot) {
			message msg("pong", {"test"});
			bot.send(msg);
		}}
	}
)
