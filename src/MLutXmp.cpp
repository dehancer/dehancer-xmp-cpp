//
// Created by denn nevera on 2019-07-18.
//

#include "dehancer/MLutXmp.h"
#include "dehancer/Blowfish.h"
#include "dehancer/Base64.h"
#include "dehancer/Utils.h"
#include "nlohmann/json.h"

#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <unistd.h>

namespace dehancer {

    static const std::string xmp_meta_prefix = "Xmp.Dehancer.mlutAttributes.undo[1]/rdf:";
    static const std::string xmp_clut_prefix = "Xmp.Dehancer.mlutClutList.undo[1]/rdf:";

    inline bool  has_prefix(const std::string& str, const std::string& prefix) {
      auto res = std::mismatch(prefix.begin(), prefix.end(), str.begin());
      return res.first == prefix.end();
    }

    template<typename T>
    std::vector<T> slice(std::vector<T> const &v, int m, int n)
    {
      auto first = v.cbegin() + m;
      auto last = v.cbegin() + n;// + 1;

      std::vector<T> vec(first, last);
      return vec;
    }


    MLutXmp::~MLutXmp() = default;

    dehancer::expected<MLutXmp,Error> MLutXmp::Create(const std::string &buffer) {
      return MLutXmp::Create(buffer,Blowfish::KeyType());
    }

    dehancer::expected<MLutXmp,Error> MLutXmp::Create(const std::string &buffer, const Blowfish::KeyType &key) {
      return parse(buffer,key,"","");
    }

    dehancer::expected<MLutXmp,Error> MLutXmp::Open(const std::string &path) {
      return MLutXmp::Open(path,Blowfish::KeyType());
    }

    dehancer::expected<MLutXmp,Error> MLutXmp::Open(
            const std::string& path,
            const Blowfish::KeyType& key
    ) {
      return Open(path,key,"");
    }

    dehancer::expected<MLutXmp,Error> MLutXmp::Open(
            const std::string& path,
            const Blowfish::KeyType& key,
            const std::string& cache_dir,
            bool purge_cache
            ) {

      try {

        if (!purge_cache && !cache_dir.empty() && access(cache_dir.c_str(),W_OK)==0 ) {
          
          auto xmp = MLutXmp();
          xmp.path_ = path;
          xmp.cache_dir_ = cache_dir;
          auto meta_file_name = xmp.get_cache_meta_path();

          if (access(meta_file_name.c_str(),R_OK)==0) {
            nlohmann::json meta;
            std::ifstream  meta_file(meta_file_name);
            meta_file>>meta;

            size_t lut_count = meta.at("lut_count");

            for (int i=0; i<lut_count; ++i) {
              std::string lut_file = xmp.get_cache_clut_path(i);
              std::ifstream instream(lut_file, std::ios::in | std::ios::binary);
              Blowfish fish(key.empty() ? Blowfish::KeyType({0,0,0,0,0,0,0,0}) : key);

              CLutBuffer tmp((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
              CLutBuffer buffer;

              fish.decrypt(tmp, buffer);

              xmp.cluts_.push_back(buffer);
            }

            for (auto m: meta.at("meta")) {
              auto buf = m["value"].get<std::string>();
              auto value =  Exiv2::XmpTextValue(buf);
              xmp.meta_[m["key"]] = value.clone();
            }

            for (auto m: meta.at("license_matrix")) {
              xmp.license_matrix_.push_back(m);
            }

            if (xmp.cluts_.size()!=0)
              return xmp;
          }
        }

        std::string metaBuffer;
        std::ifstream inFile;

        inFile.open(path.c_str(),  std::fstream::in);

        if (!inFile.is_open()) {
          return dehancer::make_unexpected(Error(
                  CommonError::NOT_FOUND,
                  error_string("mlut xmp file %s could not be opened or found", path.c_str())));
        }

        inFile.seekg(0, std::ios::end);
        metaBuffer.reserve(inFile.tellg());
        inFile.seekg(0, std::ios::beg);

        metaBuffer.assign((std::istreambuf_iterator<char>(inFile)),
                          std::istreambuf_iterator<char>());

        inFile.close();

        return parse(metaBuffer, key, path, cache_dir);

      }
      catch (std::exception &err) {
        return dehancer::make_unexpected(Error(CommonError::NOT_FOUND, err.what()));
      }
    }

    dehancer::expected<MLutXmp,Error> MLutXmp::parse(
            const std::string &metaBuffer,
            const Blowfish::KeyType &key,
            const std::string &path,
            const std::string& cache_dir) {

      Exiv2::XmpData xmpData;

      if (0 != Exiv2::XmpParser::decode(xmpData, metaBuffer)) {
        return dehancer::make_unexpected(Error(
                CommonError::PARSE_ERROR,
                error_string("mlut xmp file %s could not be parsed", path.c_str())));
      }

      Exiv2::XmpParser::terminate();

      Blowfish fish(key.empty() ? Blowfish::KeyType({0,0,0,0,0,0,0,0}) : key);

      auto xmp = MLutXmp();
      xmp.path_ = path;
      xmp.cache_dir_ = cache_dir;

      for (auto md = xmpData.begin(); md != xmpData.end(); ++md) {

        bool is_clut = false;

        std::stringstream lic_matrix_prefix;
        lic_matrix_prefix << xmp_meta_prefix << "nslicenseMatrix[";

        if (has_prefix(md->key(), lic_matrix_prefix.str())) {
          xmp.license_matrix_.push_back(static_cast<dehancer::License::Type>(md->getValue()->toLong()));
        }

        if (!key.empty()) {

          for (int i = 0; i < 3; ++i) {

            std::stringstream iss;

            iss << xmp_clut_prefix << "clutList[" << i + 1 << "]";

            if (iss.str() == md->key()) {

              is_clut = true;

              CLutBuffer tmp;
              tmp.resize(md->value().count());

              md->value().copy(tmp.data(), Exiv2::ByteOrder::littleEndian);

              CLutBuffer out;
              CLutBuffer buffer;

              base64::decode(tmp, out);

              fish.decrypt(out, buffer);

              xmp.cluts_.push_back(buffer);

              break;
            }
          }
        }

        if (is_clut) continue;

        xmp.meta_[md->key()] = md->value().clone();
      }

      if (!xmp.cache_dir_.empty()) {
        try {

          std::string meta_file =  xmp.get_cache_meta_path();

          nlohmann::json meta;

          meta["meta"] = "[]"_json;
          meta["lut_count"] = xmp.cluts_.size();

          for (auto& m: xmp.meta_) {
            meta["meta"].push_back({{"key", m.first},{"type_id",m.second->typeId()},{"value",m.second->toString()}});
          }

          meta["license_matrix"] = "[]"_json;
          for (auto& m: xmp.license_matrix_) {
            meta["license_matrix"].push_back(m);
          }

          if (!meta.empty()) {
            std::ofstream file(meta_file);
            file << meta;

            for (int i = 0; i < xmp.cluts_.size(); ++i) {
              std::string lut_file = xmp.get_cache_clut_path(i);
              std::ofstream fout(lut_file, std::ios::out | std::ios::binary);

              Blowfish fish(key.empty() ? Blowfish::KeyType({0,0,0,0,0,0,0,0}) : key);

              CLutBuffer buffer;
              fish.encrypt(xmp.cluts_[i],buffer);
              fout.write(reinterpret_cast<const char *>(buffer.data()),buffer.size());
              fout.close();
            }
          }
        }
        catch (std::exception &e) {
          return dehancer::make_unexpected(Error(
                  CommonError::PARSE_ERROR,
                  error_string("mlut xmp file %s could not be writen to cache: %s", path.c_str(), e.what())));
        }
      }

      return xmp;
    }

    std::string MLutXmp::get_cache_meta_path() const {
      return get_cache_path().append(".meta");
    }

    std::string MLutXmp::get_cache_clut_path(int index) const {
      return get_cache_path().append(".clut").append(std::to_string(index));
    }

    std::string MLutXmp::get_cache_path() const {
      auto file_path = cache_dir_;
      auto h = std::hash<std::string>()(path_);

      if (file_path.back() != '/')
        file_path.append("/");

      file_path.append("mlut_");
      file_path.append(std::to_string(h));

      return file_path;
    }

    MLutXmp::MLutXmp(const MLutXmp &other):path_(other.path_){
      for (auto &v: other.meta_) {
        meta_[v.first] = std::move(v.second->clone());
      }
      for (auto &v: other.cluts_) {
        cluts_.push_back(v);
      }
      license_matrix_ = other.license_matrix_;
    };

    Exiv2::Value::UniquePtr MLutXmp::get_value(const std::string &name) const {
      try {
        std::string key = xmp_meta_prefix;
        key.append(name);
        if (auto& v = meta_.at(key.c_str()))
          return  std::move(v->clone());
        return nullptr;
      }
      catch (...) {
        return nullptr;
      }
    }

    static std::vector<std::string> keys = {
            "nsid",
            "nscaption",
            "nsdescription",
            "nsrevision",
            "nscolorType",
            "nsISOIndex",
            "nsexpandBlendingMode",
            "nsexpandImpact",
            "nsauthor",
            "nsmaintainer",
            "nslutSize",
            "nsfilmType",
            "nslutType",
            "nsisPhotoEnabled",
            "nsisVideoEnabled",
            "nslicenseMatrix",
            "nstags",
            "serial",
            "datetime"
    };

    const std::vector<std::string>& MLutXmp::get_key_list() const {
      return keys;
    }

    const std::vector<dehancer::License::Type>& MLutXmp::get_license_matrix() const {
      return license_matrix_;
    }

    MLutXmp::ColorType MLutXmp::get_color_type() const {
      if (get_value("nscolorType"))
        return static_cast< MLutXmp::ColorType>(get_value("nscolorType")->toLong());
      return MLutXmp::ColorType::color;
    }

    MLutXmp::FilmType MLutXmp::get_film_type() const {
      if (get_value("nsfilmType"))
        return static_cast< MLutXmp::FilmType>(get_value("nsfilmType")->toLong());
      return MLutXmp::FilmType::negative;
    }

    int MLutXmp::get_ISO_index() const {
      if (get_value("nsISOIndex"))
        return static_cast<int>(get_value("nsISOIndex")->toLong());
      return 100;
    }

    int MLutXmp::get_expand_mode() const {
      if (get_value("nsexpandBlendingMode"))
        return static_cast<int>(get_value("nsexpandBlendingMode")->toLong());
      return 0;
    }

    float MLutXmp::get_expand_impact() const {
      if (get_value("nsexpandImpact"))
        return get_value("nsexpandImpact")->toFloat();
      return 1;
    }

    time_t MLutXmp::get_datetime() const {
      if (get_value("datetime"))
        return time::get_iso8601_time(get_value("datetime")->toString());
      else
        return std::time(0);
    }

    int MLutXmp::get_revision() const {
      if (get_value("nsrevision"))
        return static_cast<int>(get_value("nsrevision")->toLong());
      return 0;
    }

    std::string MLutXmp::get_id() const {

      if (get_value("nsid"))
        return get_value("nsid")->toString();

      else if (!get_caption().empty())
        return get_caption();

      else if (!get_name().empty())
        return get_name();

      return "";
    }

    std::string MLutXmp::get_name() const {
      return file::deleting_extension(file::last_component(path_));
    }

    std::string MLutXmp::get_caption() const {
      if (get_value("nscaption"))
        return get_value("nscaption")->toString();
      return "";
    }

    std::string MLutXmp::get_description() const {
      if (get_value("nsdescription"))
        return get_value("nsdescription")->toString();
      return "";
    }

    std::string MLutXmp::get_tags() const {
      if (get_value("nstags"))
        return get_value("nstags")->toString();
      return "";
    }

    std::string MLutXmp::get_author() const {
      if (get_value("nsauthor"))
        return get_value("nsauthor")->toString();
      return "";
    }

    std::string MLutXmp::get_maintainer() const {
      if (get_value("nsmaintainer"))
        return get_value("nsmaintainer")->toString();
      return "";
    }

    const std::vector<MLutXmp::CLutBuffer>& MLutXmp::get_cluts() const {
      return cluts_;
    }

    bool MLutXmp::is_photo_enabled()  const {
      if (get_value("nsisPhotoEnabled")) return get_value("nsisPhotoEnabled")->toString() == "True";
      return false;
    }

    bool MLutXmp::is_video_enabled()  const {
      if (get_value("nsisVideoEnabled")) return get_value("nsisVideoEnabled")->toString() == "True";
      return false;
    }

    MLutXmp::MLutXmp() :path_(),cache_dir_(){}

}