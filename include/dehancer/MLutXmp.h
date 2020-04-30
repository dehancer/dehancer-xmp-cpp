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

        Exiv2::Value::UniquePtr get_value(const std::string &key) const ;
        const std::vector<std::string> & get_key_list() const;

        time_t get_datetime() const ;
        int get_revision() const ;
        std::string get_id() const ;
        std::string get_name() const ;
        std::string get_caption() const ;
        bool is_photo_enabled() const ;
        bool is_video_enabled() const ;
        std::string get_description() const ;
        std::string get_tags() const ;
        std::string get_author() const ;
        std::string get_maintainer() const ;
        int   get_ISO_index() const ;
        int   get_expand_mode() const;
        float get_expand_impact() const;

        FilmType  get_film_type() const ;
        ColorType get_color_type() const;

        const std::vector<dehancer::License::Type>& get_license_matrix() const ;
        const std::vector<CLutBuffer>& get_cluts() const ;

        MLutXmp(const MLutXmp& other);
        ~MLutXmp();

    private:
        MLutXmp();
        std::string path_;
        std::string cache_dir_;
        std::map<std::string, Exiv2::Value::UniquePtr> meta_;
        std::vector<CLutBuffer> cluts_;
        std::vector<dehancer::License::Type> license_matrix_;

        std::string get_cache_path() const ;
        std::string get_cache_meta_path() const ;
        std::string get_cache_clut_path(int index) const ;

    private:
        static dehancer::expected<MLutXmp,Error> parse(const std::string &metaBuffer,
                                                       const Blowfish::KeyType &key,
                                                       const std::string& path,
                                                       const std::string& cache_dir
                                                       );

    };
}
