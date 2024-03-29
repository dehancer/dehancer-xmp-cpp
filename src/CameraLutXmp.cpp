//
// Created by denn nevera on 2019-10-12.
//

#include "dehancer/CameraLutXmp.h"
#include "dehancer/Blowfish.h"
#include "dehancer/Base64.h"
#include "dehancer/Utils.h"
#include "dehancer/FileUtils.h"
#include "nlohmann/json.h"

namespace dehancer {
    
    static const std::string xmp_meta_prefix = "Xmp.Dehancer.clutAttributes.undo[1]/rdf:";
    static const std::string xmp_clut_prefix = "Xmp.Dehancer.mlutClutList.undo[1]/rdf:";
    
    inline bool  has_prefix(const std::string& str, const std::string& prefix) {
        try{
            if (prefix.end()==prefix.begin()) return false;
            if (str.end()==str.begin()) return false;
            auto res = std::mismatch(prefix.begin(), prefix.end(), str.begin());
            return res.first == prefix.end();
        }
        catch (const std::runtime_error &e) {
            return false;
        }
        catch (const std::exception &e) {
            return false;
        }
        
    }
    
    template<typename T>
    std::vector<T> slice(std::vector<T> const &v, int m, int n)
    {
        auto first = v.cbegin() + m;
        auto last = v.cbegin() + n;// + 1;
        
        std::vector<T> vec(first, last);
        return vec;
    }
    
    
    CameraLutXmp::~CameraLutXmp() = default;
    
    dehancer::expected<CameraLutXmp,Error> CameraLutXmp::parse(
            const std::string &metaBuffer,
            const Blowfish::KeyType &key,
            const std::string& path,
            const std::string& cache_dir) {
        
        Exiv2::XmpData xmpData;
        
        if (0 != Exiv2::XmpParser::decode(xmpData, metaBuffer)) {
            return dehancer::make_unexpected(Error(
                    CommonError::PARSE_ERROR,
                    error_string("clut xmp file %s could not be parsed", path.c_str())));
        }
        
        Exiv2::XmpParser::terminate();
        
        Blowfish fish(key.empty() ? Blowfish::KeyType({0,0,0,0,0,0,0,0}) : key);
        
        auto xmp = CameraLutXmp();
        xmp.path_ = path;
        xmp.cache_dir_ = cache_dir;
        
        for (auto & md : xmpData) {
            
            std::stringstream lic_matrix_prefix;
            lic_matrix_prefix << xmp_meta_prefix << "nslicenseMatrix[";
            
            if (has_prefix(md.key(), lic_matrix_prefix.str())) {
                xmp.license_matrix_.push_back(static_cast<dehancer::License::Type>(md.getValue()->toInt64()));
            }
            
            bool is_clut = false;
            
            if (!key.empty()) {
                
                for (int i = 0; i < 1; ++i) {
                    
                    std::stringstream iss;
                    
                    iss << xmp_clut_prefix << "clutList[" << i + 1 << "]";
                    
                    if (iss.str() == md.key()) {
                        
                        is_clut = true;
                        
                        CLutBuffer tmp;
                        tmp.resize(md.value().count());
                        
                        md.value().copy(tmp.data(), Exiv2::ByteOrder::littleEndian);
                        
                        CLutBuffer out;
                        CLutBuffer buffer;
                        
                        base64::decode(tmp, out);
                        
                        fish.decrypt(out, buffer);
                        
                        xmp.clut_ = buffer;
                        
                        break;
                    }
                }
            }
            
            if (is_clut) continue;
            
            xmp.meta_[md.key()] = md.value().clone();
        }
        
        if (!xmp.cache_dir_.empty()) {
            try {
                
                std::string meta_file =  xmp.get_cache_meta_path();
                
                nlohmann::json meta;
                
                meta["meta"] = "[]"_json;
                
                for (auto& m: xmp.meta_) {
                    meta["meta"].push_back({{"key", m.first},{"type_id",m.second->typeId()},{"value",m.second->toString()}});
                }
                
                meta["license_matrix"] = "[]"_json;
                for (auto& m: xmp.license_matrix_) {
                    meta["license_matrix"].push_back(m);
                }
                
                if (!meta.empty()) {
                    platform::ofstream file(meta_file);
                    file << meta;
                    
                    std::string lut_file = xmp.get_cache_clut_path();
                    platform::ofstream fout(lut_file, std::ios::out | std::ios::binary);
                    
                    Blowfish fish_next(key.empty() ? Blowfish::KeyType({0,0,0,0,0,0,0,0}) : key);
                    
                    CLutBuffer buffer;
                    fish_next.encrypt(xmp.clut_,buffer);
                    fout.write(reinterpret_cast<const char *>(buffer.data()),buffer.size());
                    fout.close();
                    
                }
            }
            catch (std::exception &e) {
                return dehancer::make_unexpected(Error(
                        CommonError::PARSE_ERROR,
                        error_string("clut xmp file %s could not be writen to cache: %s", path.c_str(), e.what())));
            }
        }
        
        return xmp;
    }
    
    std::string CameraLutXmp::get_cache_meta_path() const {
        return get_cache_path().append(".meta");
    }
    
    std::string CameraLutXmp::get_cache_clut_path() const {
        return get_cache_path().append(".clut");
    }
    
    std::string CameraLutXmp::get_cache_path() const {
        auto file_path = cache_dir_;
        auto h = std::hash<std::string>()(path_);
        
        if (file_path.back() != '/')
            file_path.append("/");
        
        file_path.append("clut_");
        file_path.append(std::to_string(h));
        
        return file_path;
    }
    
    dehancer::expected<CameraLutXmp,Error> CameraLutXmp::Create(const std::string &metaBuffer) {
        return CameraLutXmp::Create(metaBuffer,Blowfish::KeyType());
    }
    
    dehancer::expected<CameraLutXmp,Error> CameraLutXmp::Create(const std::string &metaBuffer,
                                                                const Blowfish::KeyType &key) {
        return parse(metaBuffer,key,"","");
    }
    
    dehancer::expected<CameraLutXmp,Error> CameraLutXmp::Open(const std::string &path) {
        return CameraLutXmp::Open(path,Blowfish::KeyType());
    }
    
    dehancer::expected<CameraLutXmp,Error> CameraLutXmp::Open(
            const std::string& path,
            const Blowfish::KeyType& key) {
        return Open(path,key,"");
    }
    
    dehancer::expected<CameraLutXmp,Error> CameraLutXmp::Open(
            const std::string& path,
            const Blowfish::KeyType& key,
            const std::string& cache_dir,
            bool purge_cache
    ) {
        
        
        try {
            
            
            if (!purge_cache && !cache_dir.empty() && platform::access(cache_dir, W_OK) == Error(CommonError::OK)) {
                
                auto xmp = CameraLutXmp();
                xmp.path_ = path;
                xmp.cache_dir_ = cache_dir;
                auto meta_file_name = xmp.get_cache_meta_path();
                
                if (platform::access(meta_file_name, R_OK) == Error(CommonError::OK)) {
                    nlohmann::json meta;
                    
                    platform::ifstream  meta_file(meta_file_name);
                    meta_file>>meta;
                    
                    std::string lut_file = xmp.get_cache_clut_path();
                    platform::ifstream instream(lut_file, std::ios::in | std::ios::binary);
                    Blowfish fish(key.empty() ? Blowfish::KeyType({0,0,0,0,0,0,0,0}) : key);
                    
                    CLutBuffer tmp((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
                    
                    fish.decrypt(tmp, xmp.clut_);
                    
                    for (auto m: meta.at("meta")) {
                        auto buf = m["value"].get<std::string>();
                        auto value =  Exiv2::XmpTextValue(buf);
                        xmp.meta_[m["key"]] = value.clone();
                    }
                    
                    for (auto m: meta.at("license_matrix")) {
                        xmp.license_matrix_.push_back(m);
                    }
                    
                    if (!xmp.clut_.empty())
                        return xmp;
                    
                }
            }
            
            
            std::string metaBuffer;
            platform::ifstream inFile;
            
            inFile.open(path.c_str(), std::fstream::in);
            
            if (!inFile.is_open()) {
                return dehancer::make_unexpected(Error(
                        CommonError::NOT_FOUND,
                        error_string("clut xmp file %s could not be opened or found", path.c_str())));
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
    
    CameraLutXmp::CameraLutXmp(const CameraLutXmp &other):path_(other.path_){
        for (auto &v: other.meta_) {
            meta_[v.first] = std::move(v.second->clone());
        }
        clut_ = other.clut_;
        license_matrix_ = other.license_matrix_;
    }
    
    CameraLutXmp::exiv2_ptr_t CameraLutXmp::get_value(const std::string &name) const {
        try {
            std::string key = xmp_meta_prefix;
            key.append(name);
            
            if (auto& v = meta_.at(key))
                return  std::move(v->clone());
            
            return nullptr;
        }
        catch (...) {
            return nullptr;
        }
    }
    
    static std::vector<std::string> keys = {
            "nsrevision",
            "nsauthor",
            "nscaption",
            "nsdescription",
            "nsmaintainer",
            "nslutSize",
            "nsfilmType",
            "nslutType",
            "nstags",
            "serial",
            "datetime",
            "nsorder",
            "nsisPhotoEnabled",
            "nsisVideoEnabled",
            "nslicenseMatrix",
            "nsid",
            "nsvendor",
            "nsmodel",
            "nsformat",
    };
    
    const std::vector<std::string>& CameraLutXmp::get_key_list() const {
        return keys;
    }
    
    const std::vector<dehancer::License::Type>& CameraLutXmp::get_license_matrix() const {
        return license_matrix_;
    }
    
    time_t CameraLutXmp::get_datetime() const {
        if (get_value("datetime"))
            return time::get_iso8601_time(get_value("datetime")->toString());
        else
            return std::time(0);
    }
    
    int CameraLutXmp::get_revision() const {
        if (get_value("nsrevision")) return get_value("nsrevision")->toInt64();
        return 0;
    }
    
    std::string CameraLutXmp::get_id() const {
        if (get_value("nsid"))
            return get_value("nsid")->toString();
        return get_vendor()+"-"+get_model()+"-"+get_format();
    }
    
    std::string CameraLutXmp::get_caption() const {
        if (get_value("nscaption"))
            return get_value("nscaption")->toString();
        return "";
    }
    
    std::string CameraLutXmp::get_description() const {
        if (get_value("nsdescription"))
            return get_value("nsdescription")->toString();
        return "";
    }
    
    std::string CameraLutXmp::get_tags() const {
        if (get_value("nstags"))
            return get_value("nstags")->toString();
        return "";
    }
    
    std::string CameraLutXmp::get_author() const {
        if (get_value("nsauthor"))
            return get_value("nsauthor")->toString();
        return "";
    }
    
    std::string CameraLutXmp::get_maintainer() const {
        if (get_value("nsmaintainer"))
            return get_value("nsmaintainer")->toString();
        return "";
    }
    
    const CameraLutXmp::CLutBuffer& CameraLutXmp::get_clut() const {
        return clut_;
    }
    
    std::string CameraLutXmp::get_format() const {
        if (get_value("nsformat")) return get_value("nsformat")->toString();
        return "";
    }
    
    std::string CameraLutXmp::get_vendor() const {
        if (get_value("nsvendor")) return get_value("nsvendor")->toString();
        return "";
    }
    
    std::string CameraLutXmp::get_model() const {
        if (get_value("nsmodel")) return get_value("nsmodel")->toString();
        return "";
    }
    
    int CameraLutXmp::get_order() const {
        if (get_value("nsorder")) return get_value("nsorder")->toInt64();
        return 0;
    }
    
    bool CameraLutXmp::is_photo_enabled()  const {
        if (get_value("nsisPhotoEnabled")) return get_value("nsisPhotoEnabled")->toString() == "True";
        return false;
    }
    
    bool CameraLutXmp::is_video_enabled()  const {
        if (get_value("nsisVideoEnabled")) return get_value("nsisVideoEnabled")->toString() == "True";
        return false;
    }
    
}