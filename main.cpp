// has token and other things
#include "details.hpp"

#include "includes/sophia.h"
#include <string>
#include <vector>

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
      event.reply("3manuel's Bot!\nhas \"/game\" it's a number guessing game — "
                  "give it a try!\n\"/ascci\" creates an ascii art from image; "
                  "resize (width and height are devided by resize).");
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
              "place\ne and p are a total."));
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
            // std::cout << "sec: " << static_cast<float>(timer) / 4 <<
            // std::endl;
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

    if (event.command.get_command_name() == "ascii") {
      auto id = event.command.usr.id;
      auto opt = event.get_parameter("image");
      auto resize = event.get_parameter("resize");
      unsigned int value = 8;
      if (std::holds_alternative<dpp::snowflake>(opt)) {
        if (std::holds_alternative<int64_t>(resize)) {
          value = std::get<int64_t>(resize);
          event.reply("creating the ascii art ...");
        } else {
          event.reply("Invalid resize creating the ascii art ...");
        }
        dpp::snowflake attachment_id = std::get<dpp::snowflake>(opt);
        auto it = event.command.resolved.attachments.find(attachment_id);
        if (it != event.command.resolved.attachments.end()) {
          const dpp::attachment &att = it->second;
          std::string image_url = att.url;
          std::cout << "getting data \n";
          std::vector<unsigned char> data = to_ascii(image_url, value);
          std::string_view sv(reinterpret_cast<const char *>(data.data()),
                              data.size());
          std::cout << "sending data\n";
          bot.message_create(
              dpp::message(CHANNEL_ID,
                           "<@" + std::to_string(id) + ">here is the file")
                  .add_file("report.txt", sv, "text/plain"));
          return;
        }
      }
      event.reply("No valid image found.");
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
      dpp::slashcommand cmd("ascii", "Send me an image!", bot.me.id);
      cmd.add_option(dpp::command_option(dpp::co_attachment, "image",
                                         "Attach an image", true));
      cmd.add_option(dpp::command_option(dpp::co_integer, "resize",
                                         "size to scale down", true));
      bot.guild_command_create(cmd, GUILD_ID);
    }
  });

  bot.start(dpp::st_wait);
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t total = size * nmemb;
  auto *buffer = static_cast<std::vector<unsigned char> *>(userp);
  buffer->insert(buffer->end(), (unsigned char *)contents,
                 (unsigned char *)contents + total);
  return total;
}

stbi_uc *scale_down(const stbi_uc *src, int width, int height, int new_w,
                    int new_h) {

  stbi_uc *out = new stbi_uc[new_h * new_w];

  for (int y = 0; y < new_h; y++) {
    int src_y = y * height / new_h;
    for (int x = 0; x < new_w; x++) {
      int src_x = x * width / new_w;
      out[y * new_w + x] = src[src_y * width + src_x];
    }
  }
  return out;
}

std::vector<unsigned char> to_ascii(std::string url, unsigned int value) {
  std::vector<unsigned char> imageData;
  std::vector<unsigned char> out;
  CURL *curl = curl_easy_init();
  if (!curl)
    return out;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageData);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  if (value == 0)
    value = 1;
  if (value > 20)
    value = 20;
  CURLcode res = curl_easy_perform(curl);
  if (res == CURLE_OK) {

    Image image{};
    stbi_uc pix[] = " .:-=+*#%@";
    image.data = stbi_load_from_memory(imageData.data(), imageData.size(),
                                       &image.x, &image.y, &image.comp, 3);

    if (!image.data) {
      std::cout << "unable to load file\n";
      return out;
    }
    stbi_uc *greyscaled = new stbi_uc[image.x * image.y];

    for (int i = 0, j = 0; i < (image.x * image.y * 3) - 3; j++, i += 3) {
      greyscaled[j] =
          uint8_t(0.299f * image.data[i] + 0.587f * image.data[i + 1] +
                  0.114f * image.data[i + 2]);
      // greyscaled[j] = uint16_t((image.data[i] + image.data[i+1] +
      // image.data[i+2]) / 3);
    }
    int new_h = image.y / value;
    int new_w = image.x / value;
    stbi_uc *scaled = scale_down(greyscaled, image.x, image.y, new_w, new_h);
    int pix_long = 0;
    for (int i = 0; i < (new_w * new_h); i++) {
      if (pix_long == new_w - 1) {
        out.push_back('\n');
        pix_long = 0;
      } else {
        out.push_back(pix[scaled[i] * (sizeof(pix) - 2) / 255]);
      }
      pix_long++;
    }
    stbi_image_free(image.data);
    delete[] greyscaled;
    delete[] scaled;

  } else {

    std::cerr << "Error: " << curl_easy_strerror(res) << "\n";
  }

  curl_easy_cleanup(curl);
  return out;
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