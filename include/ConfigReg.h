/*======================================================================*\
|* Configuration Variable Registry
|*
|* This framework provides a hierarchical registry for configuration 
|* variables. Each variable can register defaults, formatting, references,
|* getter/setter callbacks etc.
|*
|* The variables can be persisted in EEPROM (as JSON). The registry
|* can be used to control or read variables from CLI, Web Interfaces
|* or Display GUI interfaces.
\*======================================================================*/


#ifndef _CONFIG_VAR_H_
#define _CONFIG_VAR_H_

#include <Arduino.h>
#include <vector>
#include <iterator>
#include <cstddef>

#include "Helper.h"

// Disable warning about char being deprecated. Not sure why it's needed...
#define ARDUINOJSON_DEPRECATED(msg)
#include <ArduinoJson.h>

#define CONFIG_MAGIC 0x19710914

// Size of buffer for JSON written to EEPROM
#ifndef CONFIG_BUFFER_SIZE
#define CONFIG_BUFFER_SIZE 1024
#endif

/*
const char WUI_CATEGORY[] PROGMEM = "F";
const char WUI_PARAMETER[] PROGMEM = "P";
const char WUI_LABEL[] PROGMEM = "L";
const char WUI_HELP[] PROGMEM = "H";
const char WUI_TYPE[] PROGMEM = "T";
const char WUI_VALUE[] PROGMEM = "V";
const char WUI_MAX[] PROGMEM = "S";
const char WUI_MIN[] PROGMEM = "M";
const char WUI_OPTIONS[] PROGMEM = "O";
*/


/************************************************************************\
|* Flags
\************************************************************************/

static const uint8_t CVF_HIDDEN =        (uint8_t)(1<<0);
static const uint8_t CVF_NOT_PERSISTED = (uint8_t)(1<<1);
static const uint8_t CVF_READ_ONLY =     (uint8_t)(1<<2);
static const uint8_t CVF_PASSWORD =      (uint8_t)(1<<3);
static const uint8_t CVF_WIZARD =        (uint8_t)(1<<4);
static const uint8_t CVF_IMPORTANT =     (uint8_t)(1<<5);
static const uint8_t CVF_COLLAPSED =     (uint8_t)(1<<6);

static const uint16_t CVF_SHOW_PASSWORD = (uint16_t)(1<<8);


/************************************************************************\
|* Global Functions
\************************************************************************/
void saveConfig();
bool loadConfig();
void defaultConfig();
bool parseConfigJson(char* jsonStr);


/************************************************************************\
|* EEPROM Config Header
\************************************************************************/

typedef struct ConfigHeader {
    uint32_t magic;
    size_t size;
    uint32_t crc;
} ConfigHeader;


/************************************************************************\
|* Config Group organizes Config Variables in a hierarchical tree
\************************************************************************/

class ConfigVar;
class ConfigGroup {
public:

    /*------------------------------------------------------------------*\
     * Recursive iterator over all the (child) variables of this group.
    \*------------------------------------------------------------------*/
    class Iterator 
    {
        public: 
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ConfigVar&;
        using pointer           = ConfigVar*;  // or also value_type*
        using reference         = ConfigVar&;  // or also value_type&

        Iterator(ConfigGroup& group, size_t index) : group_(group), index_(index) {
            varP_ = index < group_.size() ?  group_.get(index_) : nullptr;
        }

        reference operator*() const { return *varP_; }
        pointer operator->() { return varP_; }

        // Prefix increment
        Iterator& operator++() { next_(); return *this; }  

        // Postfix increment
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.index_ == b.index_; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.index_ != b.index_; };     


        size_t getVarName(char* buffer, size_t size) {
            return group_.getVarName(buffer, size, index_);
        }

        protected:
        void next_() {
            ++index_;
            varP_ = index_ < group_.size() ?  group_.get(index_) : nullptr;
        }
        ConfigGroup& group_;
        size_t index_;
        ConfigVar* varP_;
    };


    /*------------------------------------------------------------------*\
     * Config Group
    \*------------------------------------------------------------------*/
    ConfigGroup(const char* name, ConfigGroup* parent=nullptr, const char* info=nullptr, uint8_t flags=0) 
      : name_(name), parent_(parent), info_(info), flags_(flags), varCount_(0) {
        checkMain();
        if (parent_ == nullptr) { parent_ = mainGroup; }
        if (parent_ && parent_ != (ConfigGroup*)-1) { parent_->addChild(this); }
    }
    ~ConfigGroup() {
        if(parent_) {
            parent_->removeChild(this);
            parent_ = nullptr;
        }
    }
    void addChild(ConfigGroup* child) { children_.push_back(child); updateVarCount_(child->size()); }
    void removeChild(ConfigGroup* child) { remove(children_.begin(), children_.end(), child);  updateVarCount_(-child->size()); }
    ConfigGroup* findChild(const char* name);
    void addVar(ConfigVar* var) { vars_.push_back(var); updateVarCount_(1); }
    void removeVar(ConfigVar* var) { size_t tmp=vars_.size(); remove(vars_.begin(), vars_.end(), var); updateVarCount_(vars_.size() - tmp); }
    ConfigVar* findVar(const char* name);
    ConfigVar* findVarByFullName(const char* name, bool matchCase=true);
    size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0);
    size_t toJson(Print* stream, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0);
    size_t getWebUi(Print* stream, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0);
    bool setFromJson(const JsonObject& obj);
    void setDefaults();
    inline const char* name() { return name_; }
    size_t getVarName(char* buffer, size_t size, size_t index);
    inline size_t size() { return varCount_; }
    inline size_t childrenSize() { return varCount_ - vars_.size(); }
    inline std::vector<ConfigGroup*>& children() { return children_; }
    inline std::vector<ConfigVar*>& vars() { return vars_; }
    inline const char* info() { return info_; }
    inline uint8_t flags() { return flags_; }
    inline bool isHidden() { return flags_ & CVF_HIDDEN; }   
    inline bool isNotPersisted() { return flags_ & CVF_NOT_PERSISTED; }   
    ConfigVar* get(size_t n);
    std::vector<ConfigVar*>::iterator getIt(size_t n);
    Iterator begin() { return Iterator(*this, 0); }
    Iterator end()   { return Iterator(*this, size()); } 

    static ConfigGroup* mainGroup;
    static void checkMain() {
        if (!mainGroup) {
        mainGroup = (ConfigGroup*)-1; // Dummy value. Prevent infinite recursion
        mainGroup = new ConfigGroup(FST("main"), (ConfigGroup*)-1);
    }

    }

protected:
    size_t updateVarCount_(size_t n) {
        varCount_ += n;
        if (parent_ && parent_ != (ConfigGroup*)-1) { parent_->updateVarCount_(n); }
        return varCount_;
    }

    const char* name_;
    ConfigGroup* parent_;
    const char* info_;
    std::vector<ConfigGroup*> children_;
    std::vector<ConfigVar*> vars_;
    uint8_t flags_;
    size_t varCount_;
};

/************************************************************************\
|* Config Variable Base Class
\************************************************************************/
class ConfigVar {
public:
  // typedef enum ConfigType {CT_BOOL, CT_INT8, CT_INT16, CT_INT32, CT_FLOAT, VT_STR, CT_IP} PType;
  ConfigVar(const char* name, const char* typeHelp=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint8_t flags=0)
    : name_(name), fmt_(fmt), typeHelp_(typeHelp), info_(info), group_(group), flags_(flags), id_(count_++) {
        if (!typeHelp_) { typeHelp_ = FST("value"); }
        if (!info_) { info_ = FST(""); }
        if (!group_) { 
            ConfigGroup::checkMain();
            group_ = ConfigGroup::mainGroup; 
        }
        group_->addVar(this);
    }
    virtual ~ConfigVar() {
        if (group_) {
            group_->removeVar(this);
            group_ = nullptr;
        }

    }
    virtual size_t print(Print& stream) = 0;
    virtual size_t toStr(char* buffer, size_t size) = 0;
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) = 0;
    virtual bool setFromJson(const JsonVariant& jv) = 0;
    virtual size_t setFromStr(const char* valStr, const char** errorStr=nullptr) = 0;
    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) = 0;
    virtual void setDefault() = 0;
    inline const char* name() { return name_; }
    inline const char* fmt() { return fmt_; }
    inline const char* typeHelp() { return typeHelp_; }
    inline const char* info() { return info_; }
    inline uint8_t flags() { return flags_; }
    inline bool isHidden() { return flags_ & CVF_HIDDEN; }   
    inline bool isNotPersisted() { return flags_ & CVF_NOT_PERSISTED; }   
    inline bool isReadOnly() { return flags_ & CVF_READ_ONLY; }   
    inline bool isPassword() { return flags_ & CVF_PASSWORD; }
    inline size_t id() { return id_; }   

protected:
    virtual size_t getWebUiCommon_(Print* stream) {
        size_t n = 0;
        char buffer[64];
        StrTool::toCleanName(buffer, sizeof(buffer), name_);
        n += stream->printf(FST("{\"P\":\"%s\""), buffer); // Parameter Name
        n += stream->printf(FST(",\"L\":\"%s\""), name_);  // Label
        n += stream->printf(FST(",\"T\":\"%s\""), typeHelp_);  // Type
        if (info_) {n += stream->printf(FST(",\"H\":\"%s\""), info_); } // Help
        if (flags_) {n += stream->printf(FST(",\"F\":%d"), flags_); }
        return n;
    }

    const char* name_;
    const char* fmt_;
    const char* typeHelp_;
    const char* info_;
    ConfigGroup* group_;
    uint8_t flags_;
    size_t id_;

    static size_t count_;
};


/************************************************************************\
|* Config Variable Template Class for individual values
\************************************************************************/
template <class T> class ConfigVarT : public ConfigVar {
public:
    ConfigVarT(const char* name, const T deflt=0, const char* typeHelp=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, T* ptr=nullptr, bool (*setCb)(T val, void* cbData)=nullptr, T (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVar(name, typeHelp, info, fmt, group, flags), default_(deflt), value_(deflt), ptr_(ptr), setCb_(setCb), getCb_(getCb), cbData_(cbData) {
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

    virtual size_t print(Print& stream) {
        return stream.printf(fmt_, get());
    }

    virtual size_t toStr(char* buffer, size_t size) {
        size_t n = 0;
        n += snprintf(buffer+n, size-n-1, fmt_, get());
        buffer[n] = '\0';
        return n;        
    }

    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
        return n + toStr(buffer+n, size-n);
    }

    virtual bool setFromJson(const JsonVariant& jv) {
        if (!jv.is<JsonInteger>()) {
            DEBUG_printf(FST("JSON config for \"%s\" is not an integer\n"), name_);
            return true;
        }
        set(jv);
        return false;
    }

    virtual size_t setFromStr(const char* valStr, const char** errorStr=nullptr) {
        int32_t value = 0;
        size_t n = StrTool::readInteger(valStr, &value, errorStr);
        if (n) { set(value); }
        return n;
    }

    virtual void setDefault() {
        set(default_);
    }

    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = getWebUiCommon_(stream);
        get();
        n += stream->printf(FST(",\"V\":%d"), value_);  // Value
        stream->write('}'); n++;
        return n;
    }

protected:
    T default_;
    T value_;
    T* ptr_;
    bool (*setCb_)(T val, void* cbData);
    T (*getCb_)(void* cbData);
    void* cbData_;
};

/*----------------------------------------------------------------------*\
 * Boolean
\*----------------------------------------------------------------------*/
class ConfigBool : public ConfigVarT<bool> {
public:
    ConfigBool(const char* name, bool deflt=0, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, bool* ptr=nullptr, bool (*setCb)(bool val, void* cbData)=nullptr, bool (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVarT(name, deflt, FST("bool"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {}

    virtual size_t setFromStr(const char* valStr, const char** errorStr=nullptr) {
        bool value = false;
        size_t n = StrTool::readBool(valStr, &value, errorStr);
        if (n) { set(value); }
        return n;
    }
};

/*----------------------------------------------------------------------*\
 * Int32
\*----------------------------------------------------------------------*/
class ConfigInt32 : public ConfigVarT<int32_t> {
public:
    ConfigInt32(const char* name, int32_t deflt=0, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, int32_t* ptr=nullptr, bool (*setCb)(int32_t val, void* cbData)=nullptr, int32_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVarT(name, deflt, FST("int32"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int32
\*----------------------------------------------------------------------*/
class ConfigUInt32 : public ConfigVarT<uint32_t> {
public:
    ConfigUInt32(const char* name, uint32_t deflt=0, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint32_t* ptr=nullptr, bool (*setCb)(uint32_t val, void* cbData)=nullptr, uint32_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVarT(name, deflt, FST("uint32"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Int16
\*----------------------------------------------------------------------*/
class ConfigInt16 : public ConfigVarT<int16_t> {
public:
    ConfigInt16(const char* name, int16_t deflt=0, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, int16_t* ptr=nullptr, bool (*setCb)(int16_t val, void* cbData)=nullptr, int16_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVarT(name, deflt, FST("int16"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int16
\*----------------------------------------------------------------------*/
class ConfigUInt16 : public ConfigVarT<uint16_t> {
public:
    ConfigUInt16(const char* name, uint16_t deflt=0, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint16_t* ptr=nullptr, bool (*setCb)(uint16_t val, void* cbData)=nullptr, uint16_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVarT(name, deflt, FST("uint16"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Int8
\*----------------------------------------------------------------------*/
class ConfigInt8 : public ConfigVarT<int8_t> {
public:
    ConfigInt8(const char* name, int8_t deflt=0, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, int8_t* ptr=nullptr, bool (*setCb)(int8_t val, void* cbData)=nullptr, int8_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVarT(name, deflt, FST("int8"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int16
\*----------------------------------------------------------------------*/
class ConfigUInt8 : public ConfigVarT<uint8_t> {
public:
    ConfigUInt8(const char* name, uint8_t deflt=0, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t val, void* cbData)=nullptr, uint8_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVarT(name, deflt, FST("uint8"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {}
};


/************************************************************************\
|* Config Variable Template Class for arrays
\************************************************************************/
template <class T> class ConfigArrayT : public ConfigVar {
public:
    ConfigArrayT(const char* name, size_t size, const T* deflt=nullptr, const char* typeHelp=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, T* ptr=nullptr, bool (*setCb)(T* val, void* cbData)=nullptr, void (*getCb)(T* val,void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigVar(name, typeHelp, info, fmt, group, flags), size_(size), default_(deflt), ptr_(ptr), setCb_(setCb), getCb_(getCb), cbData_(cbData) {
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

    virtual size_t print(Print& stream) {
        size_t n = 0;
        stream.write('['); n++; 
        get();
        for (size_t i=0; i<size_; i++) {
            if (i) { stream.write(','); n++; }
            n += stream.printf(fmt_, value_[i]);
        }
        stream.write(']'); n++; 
        return n;
    }

    virtual size_t toStr(char* buffer, size_t size) {
        size_t n = 0;
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

    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
        return n + toStr(buffer+n, size-n);
    }

    virtual bool setFromJson(const JsonVariant& jv) {
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

    virtual size_t setFromStr(const char* valStr, const char** errorStr=nullptr) {
        size_t n = 0;
        const char* eStr=nullptr;
        if (valStr[n++] != '[') {
            *errorStr = FST("array does not start with [");
            return 0;
        }
        for (size_t i=0; i<size_; i++) {
            if (i) {
                if(valStr[n] == ']') {
                    *errorStr = FST("not enough array values");
                    return 0;
                }
                if(valStr[n++] != ',') {
                    *errorStr = FST("invalid array");
                    return 0;
                }
            }
            while (valStr[n] == ' ') { n++; }
            int32_t value = 0;
            n += StrTool::readInteger(valStr+n, &value, &eStr, true);
            if (eStr) {
                *errorStr = eStr;
                return 0;
            }
            value_[i] = value;
            while (valStr[n] == ' ') { n++; }
        }
        if (valStr[n++] != ']') {
            *errorStr = FST("array does not end with ]");
            return 0;
        }
        if (n) { set(value_); }
        return n;
    }

    virtual void setDefault() {
        if (default_) {
            for (size_t n=0; n<size_; n++) {
                value_[n] = default_[n];
            }
        } else {
            for (size_t n=0; n<size_; n++) { value_[n] = 0; }
        }
        set(value_);
    }

    inline size_t size() { return size_; }
    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        char buffer[32];
        size_t n = getWebUiCommon_(stream);
        n += stream->print(FST(",\"V\":["));  // Value
        get();
        for (size_t i=0; i<size_; i++) {
            if (i) { stream->write(','); n++; }
             size_t m = snprintf(buffer, sizeof(buffer)-1, fmt_, value_[i]);
             buffer[m] = '\0';
             stream->write(buffer, m);
             n += m;
        }
        stream->write(']'); n++;
        stream->write('}'); n++;
        return n;
    }

protected:
    size_t size_;
    const T* default_;
    T* value_;
    T* ptr_;
    bool (*setCb_)(T* val, void* cbData);
    void (*getCb_)(T* val, void* cbData);
    void* cbData_;
};

/*----------------------------------------------------------------------*\
 * String
\*----------------------------------------------------------------------*/
class ConfigStr : public ConfigArrayT<char> {
public:
    ConfigStr(const char* name, size_t size, const char* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, char* ptr=nullptr, bool (*setCb)(char* val, void* cbData)=nullptr, void (*getCb)(char* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, size, deflt, FST("str"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {
            if (!fmt) { fmt_ = FST("%s"); }
        }

    virtual size_t print(Print& stream) {
        get();
        return stream.printf(fmt_, value_);
    }
    virtual size_t toStr(char* buffer, size_t size) {
        size_t n = 0;
        get();
        n += snprintf(buffer+n, size-n-1, fmt_, value_);
        buffer[n] = '\0';
        return n;
    }
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
        if (n < size-2) { buffer[n++] = '"'; }
        if (isPassword() && !(flags & CVF_SHOW_PASSWORD)) {
            for (size_t i=0; i < 8; i++) { if (n < size-2) buffer[n++] = '*'; }
        } else {
            n += toStr(buffer+n, size-n - 1);
        }
        if (n < size-2) { buffer[n++] = '"'; }
        buffer[n]= '\0';
        return n;
    }

    virtual bool setFromJson(const JsonVariant& jv) {
        if (!jv.is<const char*>()) {
            DEBUG_printf(FST("JSON config for \"%s\" is not string\n"), name_);
            return true;
        }
        set((const char*)jv);
        return false;
    }

    virtual size_t setFromStr(const char* valStr, const char** errorStr=nullptr) {
        size_t n = 0;
        while (valStr[n] && n < size_-1) { value_[n] = valStr[n]; n++; }
        value_[n] = '\0';
        set(value_);
        return n;
    }

    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = getWebUiCommon_(stream);
        get();
        if (isPassword() && !(flags & CVF_SHOW_PASSWORD)) {
            n += stream->print(FST(",\"V\":\"********\""));
        } else { n += stream->printf(FST(",\"V\":\"%s\""), value_); } 
        n += stream->printf(FST(",\"S\":%d"), size_-1);  // Max
        n += stream->printf(FST(",\"M\":%d"), 0);  // Min
        stream->write('}'); n++;        
        return n;
    }
};


/*----------------------------------------------------------------------*\
 * Int32 Array
\*----------------------------------------------------------------------*/
class ConfigInt32Array : public ConfigArrayT<int32_t> {
public:
    ConfigInt32Array(const char* name, size_t size, const int32_t* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, int32_t* ptr=nullptr, bool (*setCb)(int32_t* val, void* cbData)=nullptr, void (*getCb)(int32_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, size, deflt, nullptr, info, fmt, group, ptr, setCb, getCb, cbData, flags) {
            sprintf(ths, FST("int32[%d]"), size);
            typeHelp_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Unsigned Int32 Array
\*----------------------------------------------------------------------*/
class ConfigUInt32Array : public ConfigArrayT<uint32_t> {
public:
    ConfigUInt32Array(const char* name, size_t size, const uint32_t* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint32_t* ptr=nullptr, bool (*setCb)(uint32_t* val, void* cbData)=nullptr, void (*getCb)(uint32_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, size, deflt, nullptr, info, fmt, group, ptr, setCb, getCb, cbData, flags) {
            sprintf(ths, FST("uint32[%d]"), size);
            typeHelp_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Int16 Array
\*----------------------------------------------------------------------*/
class ConfigInt16Array : public ConfigArrayT<int16_t> {
public:
    ConfigInt16Array(const char* name, size_t size, const int16_t* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, int16_t* ptr=nullptr, bool (*setCb)(int16_t* val, void* cbData)=nullptr, void (*getCb)(int16_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, size, deflt, nullptr, info, fmt, group, ptr, setCb, getCb, cbData, flags) {
            sprintf(ths, FST("int16[%d]"), size);
            typeHelp_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Unsigned Int16 Array
\*----------------------------------------------------------------------*/
class ConfigUInt16Array : public ConfigArrayT<uint16_t> {
public:
    ConfigUInt16Array(const char* name, size_t size, const uint16_t* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint16_t* ptr=nullptr, bool (*setCb)(uint16_t* val, void* cbData)=nullptr, void (*getCb)(uint16_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, size, deflt, nullptr, info, fmt, group, ptr, setCb, getCb, cbData, flags) {
            sprintf(ths, FST("uint16[%d]"), size);
            typeHelp_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Int8 Array
\*----------------------------------------------------------------------*/
class ConfigInt8Array : public ConfigArrayT<int8_t> {
public:
    ConfigInt8Array(const char* name, size_t size, const int8_t* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, int8_t* ptr=nullptr, bool (*setCb)(int8_t* val, void* cbData)=nullptr, void (*getCb)(int8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, size, deflt, nullptr, info, fmt, group, ptr, setCb, getCb, cbData, flags) {
            sprintf(ths, FST("int8[%d]"), size);
            typeHelp_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Unsigned Int8 Array
\*----------------------------------------------------------------------*/
class ConfigUInt8Array : public ConfigArrayT<uint8_t> {
public:
    ConfigUInt8Array(const char* name, size_t size, const uint8_t* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t* val, void* cbData)=nullptr, void (*getCb)(uint8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, size, deflt, nullptr, fmt, info, group, ptr, setCb, getCb, cbData, flags) {
            sprintf(ths, FST("uint8[%d]"), size);
            typeHelp_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * IP Address
\*----------------------------------------------------------------------*/
class ConfigIpAddr : public ConfigArrayT<uint8_t> {
public:
    ConfigIpAddr(const char* name, const uint8_t* deflt=nullptr, const char* info=nullptr, const char* fmt=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t* val, void* cbData)=nullptr, void (*getCb)(uint8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : ConfigArrayT(name, 4, deflt, FST("IP_ADR"), info, fmt, group, ptr, setCb, getCb, cbData, flags) {
            if (!fmt) { fmt_ = FST("%d.%d.%d.%d"); }
        }

    virtual size_t print(Print& stream) {
        get();
        return stream.printf(fmt_, value_[0], value_[1], value_[2], value_[3]);
    }

    virtual size_t toStr(char* buffer, size_t size) {
        size_t n = 0;
        get();
        n += snprintf(buffer+n, size-n-1, fmt_, value_[0], value_[1], value_[2], value_[3]);
        buffer[n] = '\0';
        return n;
    }
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
        if (n < size-2) { buffer[n++]= '"'; }
        n += toStr(buffer+n, size-n - 1);
        if (n < size-2) { buffer[n++]= '"'; }
        buffer[n]= '\0';
        return n;
    }
    virtual size_t setFromStr(const char* valStr, const char** errorStr=nullptr) {
        uint8_t ip[4] = {0};
        size_t n = StrTool::readIpAddr(valStr, ip, errorStr);
        if (n) { set(ip); }
        return n;
    }
    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = getWebUiCommon_(stream);
        get();
        n += stream->printf(FST(",\"V\":\"%d.%d.%d.%d\""), value_[0], value_[1], value_[2], value_[3]);  // Value
        stream->write('}'); n++;        
        return n;
    }
};



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

#endif // _CONFIG_VAR_H_

