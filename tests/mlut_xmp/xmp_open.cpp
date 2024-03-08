//
// Created by denn nevera on 2019-07-18.
//

#include "dehancer/MLutXmp.h"
#include "dehancer/Utils.h"
#include "dehancer/FileUtils.h"

#include <string>
#include <iostream>
#include <fstream>

#include "../dotenv/dotenv_utils.h"


TEST(XMP, XMPOpenTest) {

  try {

    auto pass = get_key();

    std::cout << std::endl;

    std::string file_path = "../../../tests/mlut_xmp/mlut.mlut";
    std::string cache_dir = "./cache";

    auto ec = dehancer::platform::create_directories(cache_dir);
  
    GTEST_EXPECT_FALSE(ec);
    
    std::cout << "Open test: " << file_path << std::endl;

    /*
     * * read properties
     * */

    auto xmp = dehancer::MLutXmp::Open(file_path, pass, cache_dir, false);

    if (!xmp) {
      std::cerr << xmp.error().message() << std::endl;
    }

    EXPECT_TRUE(xmp);

    if (xmp) {

      std::cout << "               id: " << xmp->get_id() << std::endl;
      std::cout << "       is printed: " << xmp->is_printed() << std::endl;
      std::cout << "   is print media: " << xmp->is_print_media() << std::endl;
      std::cout << " is photo enabled: " << xmp->is_photo_enabled() << std::endl;
      std::cout << " is video enabled: " << xmp->is_video_enabled() << std::endl;
      std::cout << "       color type: " << (int) xmp->get_color_type() << std::endl;
      std::cout << "        film type: " << (int) xmp->get_color_type() << std::endl;

      auto lics = xmp->get_license_matrix();

      for (auto l: lics) {
        std::cout << " has lic: " << static_cast<int>(l) << std::endl;
      }

      auto list = xmp->get_key_list();
      std::cout << std::endl;


      std::for_each(list.begin(), list.end(), [&xmp](const std::string &key) {
          auto value = xmp->get_value(key);

          if (value) {
            if (value->typeId() == Exiv2::TypeId::string || value->typeId() == Exiv2::TypeId::xmpText)
              std::cout << " mlut_xmp key: " << key << " = " << value->toString() << std::endl;
            else if (value->typeId() == Exiv2::TypeId::signedLong || value->typeId() == Exiv2::TypeId::unsignedLong)
              std::cout << " mlut_xmp key: " << key << " = " << value->toInt64() << std::endl;
            else {
              std::cout << " mlut_xmp key: " << key << " type:  " << std::hex << value->typeId() << std::endl;
            }
          }
      });

      EXPECT_TRUE(xmp->get_cluts().size() == 3);

      for (int i = 0; i < 3; i++) {
        auto data = xmp->get_cluts()[i];

        std::cout << " mlut_xmp luts: " << data.size() << "  >> " << std::endl;
        
        std::ofstream outFile;

        std::string file = "./";
        auto name = xmp->get_name();
        file.append(name);
        file.append("[");
        file.append(std::to_string(i));
        file.append("].png");

        outFile.open(file, std::fstream::out | std::ofstream::binary);

        std::copy(data.begin(), data.end(), std::ostreambuf_iterator<char>(outFile));

        auto str = std::string(data.begin(), data.end());

        std::cout << " mlut_xmp luts: " << xmp->get_cluts().size() << "  >> " << std::endl;
        std::copy(data.begin(), data.end(), std::ostream_iterator<uint16_t>(std::cout, " "));

        std::cout << std::endl;
      }

    } else {
      std::cerr << "Error: " << xmp.error().message() << std::endl;
    }
  }
  catch (const std::runtime_error &e) {
    std::cerr << "Exception runtime: " << e.what() << std::endl;
  }
  catch (const std::exception &e) {
    std::cerr << "Exception exception: " << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Exception unknown " << std::endl;
  }
}