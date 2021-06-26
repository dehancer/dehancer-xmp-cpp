//
// Created by denn nevera on 2019-07-18.
//

#include "dehancer/CameraLutXmp.h"
#include "dehancer/FileUtils.h"

#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <fstream>

#include "../dotenv/dotenv_utils.h"
#include "dehancer/Utils.h"

TEST(XMP, XMPOpenTest) {
  
  dotenv::dotenv::instance().config();
  
  auto pass = get_key();
  
  for (auto p: pass) std::cout<<(int)p<<",";
  std::cout<<std::endl;
  
  std::cout << std::endl;
  
  std::string file_path = "../../../tests/cameralut_xmp/clut.clut";
  std::string cache_dir = "./cache/";
  
  auto ec = dehancer::platform::create_directories(cache_dir);
  
  GTEST_EXPECT_FALSE(ec);
  
  std::cout << "Open test: " << file_path << std::endl;
  
  /*
   * * read properties
   * */
  
  auto xmp = dehancer::CameraLutXmp::Open(file_path, pass, cache_dir, true);
  
  EXPECT_TRUE(xmp);
  
  if (xmp) {
    
    for (auto lic: xmp->get_license_matrix())
      std::cout << "License, matrix: " << static_cast<int>(lic) << std::endl;
    
    std::cout << "               id " << xmp->get_id() << std::endl;
    std::cout << "           vendor " << xmp->get_vendor() << std::endl;
    std::cout << "            model " << xmp->get_model() << std::endl;
    std::cout << "           format " << xmp->get_format() << std::endl;
    std::cout << " is photo enabled " << xmp->is_photo_enabled() << std::endl;
    std::cout << " is video enabled " << xmp->is_video_enabled() << std::endl;
    
    auto list = xmp->get_key_list();
    std::cout << std::endl;
    
    std::for_each(list.begin(), list.end(), [&xmp](const std::string &key) {
        auto value = xmp->get_value(key);
        
        if (value) {
          if (value->typeId() == Exiv2::TypeId::string || value->typeId() == Exiv2::TypeId::xmpText)
            std::cout << " clut_xmp[txt] key: " << key << " = " << value->toString() << std::endl;
          else if (value->typeId() == Exiv2::TypeId::signedLong || value->typeId() == Exiv2::TypeId::unsignedLong)
            std::cout << " clut_xmp[number] key: " << key << " = " << value->toLong() << std::endl;
          else {
            std::cout << " clut_xmp key: " << key << " type:  " << std::hex << value->typeId() << std::endl;
          }
        }
    });
    
    EXPECT_TRUE(xmp->get_clut().size() > 0);
    
    auto data = xmp->get_clut();
    
    std::ofstream outFile;
    
    std::string file = "./";
    auto xmp_id = xmp->get_id();
    file.append(xmp_id);
    file.append(".png");
    
    outFile.open(file, std::fstream::out | std::ofstream::binary);
    
    std::copy(data.begin(), data.end(), std::ostreambuf_iterator<char>(outFile));
    
    auto str = std::string(data.begin(), data.end());
    
    std::cout << " mlut_xmp luts: " << xmp->get_clut().size() << "  >> " << std::endl;
    std::copy(data.begin(), data.end(), std::ostream_iterator<uint16_t>(std::cout, " "));
    
    std::cout << std::endl;
    
  } else {
    std::cerr << "Error: " << xmp.error().message() << std::endl;
  }
}