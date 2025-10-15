#ifndef SOFIA_H
#define SOFIA_H
#include <algorithm>
#include <cctype>
#include <chrono>
#include <curl/curl.h>
#include <dpp/dpp.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

struct Image {
  stbi_uc *data;
  int x;
  int y;
  int comp;
};
bool isNumber(const std::string &s);
bool isNotRepeated(std::string &s);
std::vector<unsigned char> to_ascii(std::string url, unsigned int value);
std::string strAnswer(std::string &reply, std::string &s);
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
stbi_uc *scale_down(const stbi_uc *src, int width, int height, int new_w,
                    int new_h);
#endif