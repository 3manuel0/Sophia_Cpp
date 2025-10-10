#include <algorithm>
#include <dpp/dpp.h>
#include <string>

int main() {
  const char *BOT_TOKEN = std::getenv("BOT_TOKEN");
  if (!BOT_TOKEN) {
    std::cerr << "BOT_TOKEN missing from .env file!\n";
    return 1;
  }
  dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);
  std::unordered_map<dpp::snowflake, bool> to_answer;
  char a[4];
  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([&to_answer, &a](const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "info") {
      event.reply("3manuel's Bot!");
    }
    if (event.command.get_command_name() == "game") {
      for (int i = 0; i < 4; i++) {
        char temp = rand() % 10;
        while (std::find(a, a + i + 1, temp) != a + i + 1) {
          temp = rand() % 10;
        }
        a[i] = temp;
      }
      std::string s = "";
      for (char i : a)
        s += std::to_string(i);
      event.reply(s);
      to_answer[event.command.get_issuing_user().id] = true;
    }
  });

  bot.on_message_create([&](const dpp::message_create_t &event) {
    if (to_answer.count(event.msg.author.id)) {
      std::string reply = event.msg.content;
      if (reply.length() != 4) {
        event.reply("must be 4 numbers");
      } else {
        event.reply(reply);
      }
      // to_answer.erase(event.msg.author.id);
    }
  });

  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      bot.guild_command_create(
          dpp::slashcommand("game", "number guessing game", bot.me.id),
          1368619088200601620);
      bot.guild_command_create(dpp::slashcommand("info", "info", bot.me.id),
                               1368619088200601620);
    }
  });

  bot.start(dpp::st_wait);
}