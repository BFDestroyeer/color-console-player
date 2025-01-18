#pragma once

#include <opencv2/opencv.hpp>

constexpr char SPACE[] = {0x1, 0x1, 0x20, 0x0};

std::pair<cv::Vec3b, cv::Vec3b> getColors(
     const cv::Vec3s &ul,
     const cv::Vec3s &ur,
     const cv::Vec3s &bl,
     const cv::Vec3s &br
) {
     double max = -1.0;
     std::pair<cv::Vec3b, cv::Vec3b> result;
     if (cv::norm(ul - ur) > max) {
          max = cv::norm(ul - ur);
          result = std::make_pair(ul, ur);
     }
     if (cv::norm(ul - bl) > max) {
          max = cv::norm(ul - bl);
          result = std::make_pair(ul, bl);
     }
     if (cv::norm(ul - br) > max) {
          max = cv::norm(ul - br);
          result = std::make_pair(ul, br);
     }
     if (cv::norm(ur - bl) > max) {
          max = cv::norm(ur - bl);
          result = std::make_pair(ur, bl);
     }
     if (cv::norm(ur - br) > max) {
          max = cv::norm(ur - br);
          result = std::make_pair(ur, br);
     }
     if (cv::norm(bl - br) > max) {
          result = std::make_pair(bl, br);
     }
     return result;
}

const char* symbolByConvolution(
    const uint8_t convolution
) {
     switch (convolution) {
          case 0b0000:
               return SPACE;
          case 0b1000:
               return "▘";
          case 0b0100:
               return "▝";
          case 0b0010:
               return "▖";
          case 0b0001:
               return "▗";
          case 0b1100:
               return "▀";
          case 0b0011:
               return "▄";
          case 0b1010:
               return "▌";
          case 0b0101:
               return "▐";
          case 0b1001:
               return "▚";
          case 0b0110:
               return "▞";
          case 0b0111:
               return "▟";
          case 0b1011:
               return "▙";
          case 0b1101:
               return "▜";
          case 0b1110:
               return "▛";
          case 0b1111:
               return "█";
          default:
               return " ";
     }
}
