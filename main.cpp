// has token and other things
#include "details.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <dpp/dpp.h>
#include <random>
#include <string>

bool isNumber(const std::string &s) {
  if (s.empty())
    return false;

  for (char c : s) {
    if (c < '0' || c > '9') {
      return false;
    }
  }

  return true;
}

bool isNotRepeated(std::string &s) {
  if (s.empty())
    return false;

  int i = 0;
  int j = 1;

  while (i < 4) {
    while (j < 4) {
      if (s[i] == s[j])
        return false;
      j++;
    }
    i++;
    j = i + 1;
  }

  return true;
}

std::string strAnswer(std::string &reply, std::string &s) {
  char e = 0;
  char p = 0;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {

      if (reply[j] == s[i]) {
        if (j == i) {
          p++;
        } else {
          e++;
        }
      }
    }
  }

  return std::to_string(e) + "e" + std::to_string(p) + "p";
}

int main() {
  // for grabbing the token from env
  //  const char *BOT_TOKEN = std::getenv("BOT_TOKEN");
  //  if (!BOT_TOKEN) {
  //    std::cerr << "BOT_TOKEN missing from .env file!\n";
  //    return 1;
  //  }
  dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);
  std::unordered_map<dpp::snowflake, bool> to_answer;
  char a[4];
  dpp::snowflake channel_id;
  dpp::snowflake user_id;
  int timer = 0;

  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([&](const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "info") {
      event.reply("3manuel's Bot!\nOnly \"/game\" for now — give it a try!");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 9);

    if (event.command.get_command_name() == "game") {
      if (to_answer.empty()) {
        user_id = event.command.get_issuing_user().id;
        for (int i = 0; i < 4; i++) {
          char temp = dist(gen);
          while (std::find(a, a + i + 1, temp) != a + i + 1) {
            temp = dist(gen);
          }
          a[i] = temp;
        }
        std::string s = "";
        for (char i : a)
          s += std::to_string(i);

        // event.reply("A 4 digit number was generated\ne: means existing
        // digit."
        //             "(but not in it's place).\np: means a digit is in it's "
        //             "place\ne and p are a total.\nyou have 120 seconds "
        //             "(2mins) to find the number.");
        if (event.command.channel_id == CHANNEL_ID) {
          event.reply(dpp::message(
              "<@" + std::to_string(user_id) +
              ">\nA 4 digit number was generated\ne: means existing "
              "digit."
              "(but not in it's place).\np: means a digit is in it's "
              "place\ne and p are a total.\nyou have 120 seconds "
              "(2mins) to find the number."));
        } else {
          event.reply("Check <#" + std::to_string(CHANNEL_ID) + ">");
          bot.message_create(dpp::message(
              CHANNEL_ID,
              "<@" + std::to_string(user_id) +
                  ">\nA 4 digit number was generated\ne: means existing "
                  "digit."
                  "(but not in it's place).\np: means a digit is in it's "
                  "place\ne and p are a total."));
        }
        to_answer[event.command.get_issuing_user().id] = true;

        std::thread([&]() {
          for (; timer < 120 * 4; timer++) {
            if (to_answer.empty()) {
              timer = 0;
              return;
            }
            std::cout << "sec: " << static_cast<float>(timer) / 4 << std::endl;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(250)); // wait one second
          }
          to_answer.clear();
          bot.message_create(dpp::message(
              CHANNEL_ID,
              "<@" + std::to_string(user_id) + ">" +
                  " Game Over; 2 mins have passed without any reply"));
          timer = 0;
        }).detach();

      } else {
        auto it = to_answer.begin();
        auto id = it->first;
        std::string s =
            "<@" + std::to_string(id) +
            "> is using the bot rn, waiting for the game to end or timout";
        event.reply(s);
      }
    }
  });

  bot.on_message_create([&](const dpp::message_create_t &event) {
    if (event.msg.channel_id == CHANNEL_ID) {
      if (to_answer.count(event.msg.author.id)) {
        timer = 0;
        std::string reply = event.msg.content;
        if (reply == "stop") {
          bot.message_create(
              dpp::message(CHANNEL_ID, "<@" + std::to_string(user_id) +
                                           ">"
                                           " Game over"));
          // event.reply("Game over");
          to_answer.erase(event.msg.author.id);
        } else if (reply.length() != 4 || !isNumber(reply)) {
          // event.reply("must be 4 digit number :rage:");
          timer = 0;
          bot.message_create(
              dpp::message(CHANNEL_ID, "<@" + std::to_string(user_id) +
                                           ">"
                                           " must be 4 digit number :rage:"));
        } else if (!isNotRepeated(reply)) {
          // event.reply("must not repeat numbers :rage:");
          bot.message_create(
              dpp::message(CHANNEL_ID, "<@" + std::to_string(user_id) +
                                           ">"
                                           " must not repeat numbers :rage:"));
        } else {
          std::string s = "";
          for (int i = 0; i < 4; i++)
            s += std::to_string(a[i]);
          if (s == reply) {
            to_answer.erase(event.msg.author.id);
            // event.reply("congrats you won");
            bot.message_create(
                dpp::message(CHANNEL_ID, "<@" + std::to_string(user_id) +
                                             "> congrats you won"));
          } else {
            std::string answer = strAnswer(reply, s);
            // event.reply(answer);
            bot.message_create(
                dpp::message(CHANNEL_ID, "<@" + std::to_string(user_id) +
                                             ">\n" + reply + " " + answer));
          }
        }
        // to_answer.erase(event.msg.author.id);
      }
    }
  });

  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      bot.guild_command_create(
          dpp::slashcommand("game", "number guessing game", bot.me.id),
          GUILD_ID);
      bot.guild_command_create(dpp::slashcommand("info", "info", bot.me.id),
                               GUILD_ID);
    }
  });

  bot.start(dpp::st_wait);
}