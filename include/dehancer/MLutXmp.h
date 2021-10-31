//
// Created by denn nevera on 2019-07-18.
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

    class MLutXmp {

    public:

        enum class ColorType:int  {
            color = 0,
            bw    = 1
        };

        enum class FilmType:int {
            negative  = 0,
            positive  = 1
        };

        using CLutBuffer = std::vector<std::uint8_t>;
        using exiv2_ptr_t = Exiv2::Value::UniquePtr;

    public:


        static dehancer::expected<MLutXmp,Error> Create(const std::string &buffer, const Blowfish::KeyType& key);
        static dehancer::expected<MLutXmp,Error> Create(const std::string &buffer);

        /**
         * Open xmp-mlut-file
         * @param path - file path
         * @param key - pass key, by default empty means properties read only
         * @return
         */
        static dehancer::expected<MLutXmp,Error> Open(
                const std::string &path,
                const Blowfish::KeyType& key);

        /**
        * Open xmp-mlut-file
        * @param path - file path
        * @param key - pass key, by default empty means properties read only
        * @param cache_path
        * @return
        */
        static dehancer::expected<MLutXmp,Error> Open(
                const std::string &path,
                const Blowfish::KeyType& key,
                const std::string& cache_dir,
                bool purge_cache = false
                );


        static dehancer::expected<MLutXmp,Error> Open(const std::string &path);
    
        [[nodiscard]] exiv2_ptr_t get_value(const std::string &key) const ;
        [[nodiscard]] const std::vector<std::string> & get_key_list() const;

        [[nodiscard]] time_t get_datetime() const ;
        [[nodiscard]] int get_revision() const ;
        [[nodiscard]] std::string get_id() const ;
        [[nodiscard]] std::string get_name() const ;
        [[nodiscard]] std::string get_caption() const ;
        [[nodiscard]] bool is_printed() const ;
        [[nodiscard]] bool is_print_media() const ;
        [[nodiscard]] bool is_photo_enabled() const ;
        [[nodiscard]] bool is_video_enabled() const ;
        [[nodiscard]] std::string get_description() const ;
        [[nodiscard]] std::string get_tags() const ;
        [[nodiscard]] std::string get_author() const ;
        [[nodiscard]] std::string get_maintainer() const ;
        [[nodiscard]] int   get_ISO_index() const ;
        [[nodiscard]] int   get_expand_mode() const;
        [[nodiscard]] float get_expand_impact() const;
    
        [[nodiscard]] FilmType  get_film_type() const ;
        [[nodiscard]] ColorType get_color_type() const;

        [[nodiscard]] const std::vector<dehancer::License::Type>& get_license_matrix() const ;
        [[nodiscard]] const std::vector<CLutBuffer>& get_cluts() const ;

        MLutXmp(const MLutXmp& other);
        ~MLutXmp();

    private:
        MLutXmp();
        std::string path_;
        std::string cache_dir_;
        std::map<std::string, exiv2_ptr_t> meta_;
        std::vector<CLutBuffer> cluts_;
        std::vector<dehancer::License::Type> license_matrix_;

        [[nodiscard]] std::string get_cache_path() const ;
        [[nodiscard]] std::string get_cache_meta_path() const ;
        [[nodiscard]] std::string get_cache_clut_path(int index) const ;

    private:
        static dehancer::expected<MLutXmp,Error> parse(const std::string &metaBuffer,
                                                       const Blowfish::KeyType &key,
                                                       const std::string& path,
                                                       const std::string& cache_dir
                                                       );

    };
}
