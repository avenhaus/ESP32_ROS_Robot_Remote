#ifndef _CONF_VAR_H_
#define _CONF_VAR_H_

#include <Arduino.h>
#include <vector>

// Disable warning about char being deprecated. Not sure why ot's needed...
#define ARDUINOJSON_DEPRECATED(msg)
#include <ArduinoJson.h>

#define CONFIG_MAGIC 0x19710914

#ifndef CONFIG_BUFFER_SIZE
#define CONFIG_BUFFER_SIZE 1024
#endif

typedef struct ConfigHeader {
    uint32_t magic;
    size_t size;
    uint32_t crc;
} ConfigHeader;

class ConfigVar;

class ConfigGroup {
public:
    ConfigGroup(const char* name, ConfigGroup* parent=nullptr, uint8_t hidden=0, bool persisted=true) 
      : name_(name), parent_(parent), hidden_(hidden), isPersisted_(persisted) {
        if (!mainGroup) {
            mainGroup = (ConfigGroup*)-1; // Dummy value. Prevent infinite recursion
            mainGroup = new ConfigGroup(FST("main"), (ConfigGroup*)-1);
        }
        if (parent_ == nullptr) { parent_ = mainGroup; }
        if (parent_ && parent_ != (ConfigGroup*)-1) { parent_->addChild(this); }
    }
    ~ConfigGroup() {
        if(parent_) {
            parent_->removeChild(this);
            parent_ = nullptr;
        }
    }
    void addChild(ConfigGroup* child) { children_.push_back(child); }
    void removeChild(ConfigGroup* child) { remove(children_.begin(), children_.end(), child); }
    ConfigGroup* findChild(const char* name);
    void addVar(ConfigVar* var) { vars_.push_back(var); }
    void removeVar(ConfigVar* var) { remove(vars_.begin(), vars_.end(), var); }
    ConfigVar* findVar(const char* name);
    size_t toJsonStr(char* buffer, size_t size, bool noName=false);
    bool fromJson(const JsonObject& obj);
    inline const char* name() { return name_; }

    static size_t jsonName(char* buffer, size_t size, const char* name);
    static bool matchesJsonName(const char* jName, const char* name);

    static ConfigGroup* mainGroup;

protected:
    const char* name_;
    ConfigGroup* parent_;
    std::vector<ConfigGroup*> children_;
    std::vector<ConfigVar*> vars_;
    uint8_t hidden_;
    bool isPersisted_;  // Save to EEPROM
};

class ConfigVar {
public:
  // typedef enum ConfigType {CT_BOOL, CT_INT8, CT_INT16, CT_INT32, CT_FLOAT, VT_STR, CT_IP} PType;
  ConfigVar(const char* name, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint8_t hidden=0, bool persisted=true)
    : name_(name), fmt_(fmt), info_(info), group_(group), hidden_(hidden), isPersisted_(persisted) {
        if (!group_) { group_ = ConfigGroup::mainGroup; }
        group_->addVar(this);
    }
    virtual ~ConfigVar() {
        if (group_) {
            group_->removeVar(this);
            group_ = nullptr;
        }

    }
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) = 0;
    virtual bool fromJson(const JsonVariant& jv) = 0;
    inline const char* name() { return name_; }


protected:
    const char* name_;
    const char* fmt_;
    const char* info_;
    ConfigGroup* group_;
    uint8_t hidden_;
    bool isPersisted_;  // Save to EEPROM
};



template <class T> class ConfigVarT : public ConfigVar {
public:
    ConfigVarT(const char* name, const T deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, T* ptr=nullptr, bool (*setCb)(T val, void* cbData)=nullptr, T (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVar(name, fmt, info, group, hidden, persisted), default_(deflt), value_(deflt), ptr_(ptr), setCb_(setCb), getCb_(getCb), cbData_(cbData) {
        if (!fmt) { fmt_ = FST("%d"); }
    }
    virtual ~ConfigVarT() {}
    virtual T get() {
        if (ptr_) { value_ = *ptr_; }
        if (getCb_) { value_ = getCb_(cbData_); }
        return value_;
    }
    virtual bool set(T val) {
        value_ = val;
        if (ptr_) { *ptr_ = value_; }
        if (setCb_) { return setCb_(value_, cbData_); }
        return false;
    }
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
        size_t n = 0;
        if (!noName) { n += ConfigGroup::jsonName(buffer+n, size-n, name_); }
        n += snprintf(buffer+n, size-n-1, fmt_, get());
        buffer[n] = '\0';
        return n;
    }
    virtual bool fromJson(const JsonVariant& jv) {
        if (!jv.is<JsonInteger>()) {
            DEBUG_printf(FST("JSON config for \"%s\" is not an integer\n"), name_);
            return true;
        }
        set(jv);
        return false;
    };


protected:
    T default_;
    T value_;
    T* ptr_;
    bool (*setCb_)(T val, void* cbData);
    T (*getCb_)(void* cbData);
    void* cbData_;
};



class ConfigBool : public ConfigVarT<bool> {
public:
    ConfigBool(const char* name, bool deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, bool* ptr=nullptr, bool (*setCb)(bool val, void* cbData)=nullptr, bool (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};

class ConfigInt32 : public ConfigVarT<bool> {
public:
    ConfigInt32(const char* name, bool deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, bool* ptr=nullptr, bool (*setCb)(bool val, void* cbData)=nullptr, bool (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};

class ConfigUInt32 : public ConfigVarT<uint32_t> {
public:
    ConfigUInt32(const char* name, uint32_t deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint32_t* ptr=nullptr, bool (*setCb)(uint32_t val, void* cbData)=nullptr, uint32_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, info, group, ptr, setCb, getCb, cbData,  hidden, persisted) {}
};



template <class T> class ConfigArrayT : public ConfigVar {
public:
    ConfigArrayT(const char* name, size_t size, const T* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, T* ptr=nullptr, bool (*setCb)(T* val, void* cbData)=nullptr, void (*getCb)(T* val,void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVar(name, fmt, info, group, hidden, persisted), size_(size), default_(deflt), ptr_(ptr), setCb_(setCb), getCb_(getCb), cbData_(cbData) {
        if (!fmt) { fmt_ = FST("%d"); }
        value_ = new T[size_];
        if (default_) { for (size_t n=0; n<size_; n++) { value_[n] = default_[n]; } }
        else { for (size_t n=0; n<size_; n++) { value_[n] = 0; } }        
    }
    virtual ~ConfigArrayT() { delete [] value_; }
    virtual T* get() {
        if (ptr_) { for (size_t n=0; n<size_; n++) { value_[n] = ptr_[n]; } }
        if (getCb_) { getCb_(value_, cbData_); }
        return value_;
    }
    virtual bool set(const T* val) {
        for (size_t n=0; n<size_; n++) { value_[n] = val[n]; }
        if (ptr_) { for (size_t n=0; n<size_; n++) { ptr_[n] = val[n]; } }
        if (setCb_) { return setCb_(value_, cbData_); }
        return false;
    }
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
        size_t n = 0;
        if (!noName) { n += ConfigGroup::jsonName(buffer+n, size-n, name_); }
        if (n < size-1) { buffer[n++] = '['; }
        get();
        for (size_t i=0; i<size_; i++) {
            if (i && (n < size-1)) { buffer[n++] = ','; }
            n += snprintf(buffer+n, size-n-1, fmt_, value_[i]);
        }
        if (n < size-1) { buffer[n++] = ']'; }
        buffer[n] = '\0';
        return n;
    }
    virtual bool fromJson(const JsonVariant& jv) {
        if (!jv.is<JsonArray>()) {
            DEBUG_printf(FST("JSON config for \"%s\" is not an array\n"), name_);
            return true;
        }
        JsonArray arr = jv;
        if ( arr.size() > size_ ) {
            DEBUG_printf(FST("JSON config array for \"%s\" is too large:%d space:%d\n"), name_, arr.size(), size_);
            return true;
        }
        size_t n = 0;
        for (JsonVariant v : arr) {
            if (!v.is<JsonInteger>()) {
                DEBUG_printf(FST("JSON config array element for \"%s\" is not an integer\n"), name_);
                return true;
            }
            value_[n++] = v;
       }
       set(value_);
       return false;
    };
    inline size_t size() { return size_; }

protected:
    size_t size_;
    const T* default_;
    T* value_;
    T* ptr_;
    bool (*setCb_)(T* val, void* cbData);
    void (*getCb_)(T* val, void* cbData);
    void* cbData_;
};

#if 1
class ConfigStr : public ConfigArrayT<char> {
public:
    ConfigStr(const char* name, size_t size, const char* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, char* ptr=nullptr, bool (*setCb)(char* val, void* cbData)=nullptr, void (*getCb)(char* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}

    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
        size_t n = 0;
        if (!noName) { n += ConfigGroup::jsonName(buffer+n, size-n, name_); }
        get();
        n += snprintf(buffer+n, size-n-1, FST("\"%s\""), value_);
        buffer[n] = '\0';
        return n;
    }

    virtual bool fromJson(const JsonVariant& jv) {
        if (!jv.is<const char*>()) {
            DEBUG_printf(FST("JSON config for \"%s\" is not string\n"), name_);
            return true;
        }
        set((const char*)jv);
        return false;
    }

};
#else
class ConfigStr : public ConfigVarT<char*> {
public:
    ConfigStr(const char* name, size_t size, const char* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, char* ptr=nullptr, bool (*setCb)(char* val)=nullptr, char* (*getCb)()=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, info, group, ptr, setCb, getCb, hidden, persisted), size_(size) {
        if (!fmt) { fmt_ = FST("%d"); }
        value_ = new char[size_];
        if (default_) { for (size_t n=0; n<size_; n++) { value_[n] = default_[n]; } }
        else { for (size_t n=0; n<size_; n++) { value_[n] = 0; } }        
    }
    ~ConfigStr() {
        if (value_) {
            delete [] value_;
            value_ = 0;
        }
    }

    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
        size_t n = 0;
        if (!noName) { n += ConfigGroup::jsonName(buffer+n, size-n, name_); }
        get();
        n += snprintf(buffer+n, size-n-1, FST("\"%s\""), value_);
        buffer[n] = '\0';
        return n;
    }
    /*
    virtual bool fromJson(const JsonVariant& jv) {
        if (!jv.is<JsonString>()) {
            DEBUG_printf(FST("JSON config for \"%s\" is not string\n"), name_);
            return true;
        }
        //JsonString js = jv;
        //set(js.c_str());
        return false;
    };*/
    protected:
        size_t size_;
};
#endif


class ConfigUInt8Array : public ConfigArrayT<uint8_t> {
public:
    ConfigUInt8Array(const char* name, size_t size, const uint8_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t* val, void* cbData)=nullptr, void (*getCb)(uint8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};

class ConfigUInt16Array : public ConfigArrayT<uint16_t> {
public:
    ConfigUInt16Array(const char* name, size_t size, const uint16_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint16_t* ptr=nullptr, bool (*setCb)(uint16_t* val, void* cbData)=nullptr, void (*getCb)(uint16_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};


class ConfigIpAddr : public ConfigArrayT<uint8_t> {
public:
    ConfigIpAddr(const char* name, const uint8_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t* val, void* cbData)=nullptr, void (*getCb)(uint8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, 4, deflt, fmt, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};



/* ============================================== *\
 * EEPROM Config Data
\* ============================================== */
bool loadConfig();
void saveConfig();
void setConfigData(const char* field, const char* data);

/*
struct WifiInfo {
  char ssid[36];
  char password[64];
  char host_name[32];
  uint8_t ip[4];
  uint8_t dns[4];
  uint8_t gateway[4];
  uint8_t subnet[4];
  bool isDisabled;
};

struct ConfigData {
  uint32_t magic;
  uint32_t configSize;
  uint16_t displayBrightness;
  uint16_t touchCalibartionData[8];
  char timeFormat[16];
  char dateFormat[16];
  char timeZone[48];
  uint32_t adcVref;
  WifiInfo wifi;
  uint32_t crc;  // Must be at end of struct
};

extern ConfigData config;
*/

#endif // _CONF_VAR_H_

