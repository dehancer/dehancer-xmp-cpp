//
// Created by denn nevera on 2019-10-12.
//

#pragma once

#include <exiv2/exiv2.hpp>
#include <optional>
#include <string>
#include <vector>
#include <map>

#include "dehancer/Common.h"
#include "dehancer/Deferred.h"
#include "dehancer/Blowfish.h"
#include "dehancer/License.h"

namespace dehancer {

    class CameraLutXmp {

    public:

        using CLutBuffer = std::vector<std::uint8_t>;
        using exiv2_ptr_t = Exiv2::Value::UniquePtr;

    public:

        static dehancer::expected<CameraLutXmp,Error> Create(const std::string &buffer, const Blowfish::KeyType& key);
        static dehancer::expected<CameraLutXmp,Error> Create(const std::string &buffer);

        /**
        * Open xmp-mlut-file
        * @param path - file path
        * @param key - pass key, by default empty means properties read only
        * @return
        */
        static dehancer::expected<CameraLutXmp,Error> Open(
                const std::string &path,
                const Blowfish::KeyType& key
                );

        /**
       * Open xmp-mlut-file
       * @param path - file path
       * @param key - pass key, by default empty means properties read only
       * @param cache_dir
       * @return
       */
        static dehancer::expected<CameraLutXmp,Error> Open(
                const std::string &path,
                const Blowfish::KeyType& key,
                const std::string& cache_dir,
                bool purge_cache = false
        );
        static dehancer::expected<CameraLutXmp,Error> Open(const std::string &path);
    
        [[nodiscard]] exiv2_ptr_t get_value(const std::string &key) const ;
        [[nodiscard]] const std::vector<std::string> & get_key_list() const;
    
        [[nodiscard]] time_t get_datetime() const ;
        [[nodiscard]] int get_revision() const ;
        [[nodiscard]] int get_order() const ;
        [[nodiscard]] bool is_photo_enabled() const ;
        [[nodiscard]] bool is_video_enabled() const ;
        [[nodiscard]] std::string get_id() const ;
        [[nodiscard]] std::string get_vendor() const ;
        [[nodiscard]] std::string get_model() const ;
        [[nodiscard]] std::string get_format() const ;
        [[nodiscard]] std::string get_caption() const ;
        [[nodiscard]] std::string get_description() const ;
        [[nodiscard]] std::string get_tags() const ;
        [[nodiscard]] std::string get_author() const ;
        [[nodiscard]] std::string get_maintainer() const ;
        [[nodiscard]] const std::vector<dehancer::License::Type>& get_license_matrix() const ;
        [[nodiscard]] const CLutBuffer& get_clut() const ;

        CameraLutXmp(const CameraLutXmp& other);
        ~CameraLutXmp();

    private:
        CameraLutXmp():path_(),cache_dir_(){};
        std::string path_;
        std::string cache_dir_;
        std::map<std::string, exiv2_ptr_t> meta_;
        CLutBuffer clut_;
        std::vector<dehancer::License::Type> license_matrix_;
    
        [[nodiscard]] std::string get_cache_path() const ;
        [[nodiscard]] std::string get_cache_meta_path() const ;
        [[nodiscard]] std::string get_cache_clut_path() const ;
    private:
        static dehancer::expected<CameraLutXmp,Error> parse(
                const std::string &metaBuffer,
                const Blowfish::KeyType &key,
                const std::string& path,
                const std::string& cache_dir);
    };
}
