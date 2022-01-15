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

// Disable warning about char being deprecated. Not sure why ot's needed...
#define ARDUINOJSON_DEPRECATED(msg)
#include <ArduinoJson.h>

#define CONFIG_MAGIC 0x19710914

// Size of buffer for JSON written to EEPROM
#ifndef CONFIG_BUFFER_SIZE
#define CONFIG_BUFFER_SIZE 1024
#endif


/************************************************************************\
|* Global Functions
\************************************************************************/
void saveConfig();
bool loadConfig();
void defaultConfig();
bool parseConfigJson(char* jsonStr);


/************************************************************************\
|* String Helper Functions
\************************************************************************/

namespace StrTool {
  inline bool isWhiteSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
  inline bool isWhiteSpaceOrEnd(char c) { return c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
  size_t readWord(const char* buffer, char* result, size_t size);
  size_t readBool(const char* buffer, bool* result, const char** errorStr=nullptr, bool arrayMode=false);
  size_t readInteger(const char* buffer, int* result, const char** errorStr=nullptr, bool arrayMode=false);
  size_t readFloat(const char* buffer, float* result, const char** errorStr=nullptr, bool arrayMode=false);
  size_t readBinary(const char* buffer, int* result, bool isNegative=false, const char** errorStr=nullptr, bool arrayMode=false);
  size_t readHex(const char* buffer, int* result, bool isNegative=false, const char** errorStr=nullptr, bool arrayMode=false);
  size_t readIpAddr(const char* buffer, uint8_t ip[4], const char** errorStr=nullptr);
  size_t tryRead(const char* str, const char* buffer);
  size_t toJsonName(char* buffer, size_t size, const char* name);
  size_t toVarName(char* buffer, size_t size, const char* name);
  bool matchesVarName(const char* jName, const char* name);
  size_t matchesNamePart(const char* pName, const char* name, bool matchCase=true);
  uint32_t calculateCrc(const char* data, size_t size);
};

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
    ConfigGroup(const char* name, ConfigGroup* parent=nullptr, uint8_t hidden=0, bool persisted=true) 
      : name_(name), parent_(parent), hidden_(hidden), isPersisted_(persisted), varCount_(0) {
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
    void addChild(ConfigGroup* child) { children_.push_back(child); updateVarCount_(child->size()); }
    void removeChild(ConfigGroup* child) { remove(children_.begin(), children_.end(), child);  updateVarCount_(-child->size()); }
    ConfigGroup* findChild(const char* name);
    void addVar(ConfigVar* var) { vars_.push_back(var); updateVarCount_(1); }
    void removeVar(ConfigVar* var) { size_t tmp=vars_.size(); remove(vars_.begin(), vars_.end(), var); updateVarCount_(vars_.size() - tmp); }
    ConfigVar* findVar(const char* name);
    ConfigVar* findVarByFullName(const char* name, bool matchCase=true);
    size_t toJsonStr(char* buffer, size_t size, bool noName=false);
    bool setFromJson(const JsonObject& obj);
    void setDefaults();
    inline const char* name() { return name_; }
    size_t getVarName(char* buffer, size_t size, size_t index);
    inline size_t size() { return varCount_; }
    inline size_t childrenSize() { return varCount_ - vars_.size(); }
    inline std::vector<ConfigGroup*>& children() { return children_; }
    inline std::vector<ConfigVar*>& vars() { return vars_; }
    inline uint8_t hidden() { return hidden_; }   
    inline bool isPersisted() { return isPersisted_; }   
    ConfigVar* get(size_t n);
    std::vector<ConfigVar*>::iterator getIt(size_t n);
    Iterator begin() { return Iterator(*this, 0); }
    Iterator end()   { return Iterator(*this, size()); } 

    static ConfigGroup* mainGroup;

protected:
    size_t updateVarCount_(size_t n) {
        varCount_ += n;
        if (parent_ && parent_ != (ConfigGroup*)-1) { parent_->updateVarCount_(n); }
        return varCount_;
    }

    const char* name_;
    ConfigGroup* parent_;
    std::vector<ConfigGroup*> children_;
    std::vector<ConfigVar*> vars_;
    uint8_t hidden_;
    bool isPersisted_;  // Save to EEPROM
    size_t varCount_;
};

/************************************************************************\
|* Config Variable Base Class
\************************************************************************/
class ConfigVar {
public:
  // typedef enum ConfigType {CT_BOOL, CT_INT8, CT_INT16, CT_INT32, CT_FLOAT, VT_STR, CT_IP} PType;
  ConfigVar(const char* name, const char* fmt=nullptr, const char* type_help=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint8_t hidden=0, bool persisted=true)
    : name_(name), fmt_(fmt), type_help_(type_help), info_(info), group_(group), hidden_(hidden), isPersisted_(persisted) {
        if (!type_help_) { type_help_ = FST("value"); }
        if (!group_) { group_ = ConfigGroup::mainGroup; }
        group_->addVar(this);
    }
    virtual ~ConfigVar() {
        if (group_) {
            group_->removeVar(this);
            group_ = nullptr;
        }

    }
    virtual size_t toStr(char* buffer, size_t size) = 0;
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) = 0;
    virtual bool setFromJson(const JsonVariant& jv) = 0;
    virtual size_t setFromStr(const char* valStr, const char** errorStr=nullptr) = 0;
    virtual void setDefault() = 0;
    inline const char* name() { return name_; }
    inline const char* fmt() { return fmt_; }
    inline const char* type_help() { return type_help_; }
    inline const char* info() { return info_; }
    inline uint8_t hidden() { return hidden_; }   
    inline bool isPersisted() { return isPersisted_; }   


protected:
    const char* name_;
    const char* fmt_;
    const char* type_help_;
    const char* info_;
    ConfigGroup* group_;
    uint8_t hidden_;
    bool isPersisted_;  // Save to EEPROM
};


/************************************************************************\
|* Config Variable Template Class for individual values
\************************************************************************/
template <class T> class ConfigVarT : public ConfigVar {
public:
    ConfigVarT(const char* name, const T deflt=0, const char* fmt=nullptr, const char* type_help=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, T* ptr=nullptr, bool (*setCb)(T val, void* cbData)=nullptr, T (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVar(name, fmt, type_help, info, group, hidden, persisted), default_(deflt), value_(deflt), ptr_(ptr), setCb_(setCb), getCb_(getCb), cbData_(cbData) {
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

    virtual size_t toStr(char* buffer, size_t size) {
        size_t n = 0;
        n += snprintf(buffer+n, size-n-1, fmt_, get());
        buffer[n] = '\0';
        return n;        
    }

    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
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
    ConfigBool(const char* name, bool deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, bool* ptr=nullptr, bool (*setCb)(bool val, void* cbData)=nullptr, bool (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, FST("bool"), info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}

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
    ConfigInt32(const char* name, int32_t deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, int32_t* ptr=nullptr, bool (*setCb)(int32_t val, void* cbData)=nullptr, int32_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, FST("int32"), info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int32
\*----------------------------------------------------------------------*/
class ConfigUInt32 : public ConfigVarT<uint32_t> {
public:
    ConfigUInt32(const char* name, uint32_t deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint32_t* ptr=nullptr, bool (*setCb)(uint32_t val, void* cbData)=nullptr, uint32_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, FST("uint32"), info, group, ptr, setCb, getCb, cbData,  hidden, persisted) {}
};

/*----------------------------------------------------------------------*\
 * Int16
\*----------------------------------------------------------------------*/
class ConfigInt16 : public ConfigVarT<int16_t> {
public:
    ConfigInt16(const char* name, int16_t deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, int16_t* ptr=nullptr, bool (*setCb)(int16_t val, void* cbData)=nullptr, int16_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, FST("int16"), info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int16
\*----------------------------------------------------------------------*/
class ConfigUInt16 : public ConfigVarT<uint16_t> {
public:
    ConfigUInt16(const char* name, uint16_t deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint16_t* ptr=nullptr, bool (*setCb)(uint16_t val, void* cbData)=nullptr, uint16_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, FST("uint16"), info, group, ptr, setCb, getCb, cbData,  hidden, persisted) {}
};

/*----------------------------------------------------------------------*\
 * Int8
\*----------------------------------------------------------------------*/
class ConfigInt8 : public ConfigVarT<int8_t> {
public:
    ConfigInt8(const char* name, int8_t deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, int8_t* ptr=nullptr, bool (*setCb)(int8_t val, void* cbData)=nullptr, int8_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, FST("int8"), info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int16
\*----------------------------------------------------------------------*/
class ConfigUInt8 : public ConfigVarT<uint8_t> {
public:
    ConfigUInt8(const char* name, uint8_t deflt=0, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t val, void* cbData)=nullptr, uint8_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVarT(name, deflt, fmt, FST("uint8"), info, group, ptr, setCb, getCb, cbData,  hidden, persisted) {}
};


/************************************************************************\
|* Config Variable Template Class for arrays
\************************************************************************/
template <class T> class ConfigArrayT : public ConfigVar {
public:
    ConfigArrayT(const char* name, size_t size, const T* deflt=nullptr, const char* fmt=nullptr, const char* type_help=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, T* ptr=nullptr, bool (*setCb)(T* val, void* cbData)=nullptr, void (*getCb)(T* val,void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigVar(name, fmt, type_help, info, group, hidden, persisted), size_(size), default_(deflt), ptr_(ptr), setCb_(setCb), getCb_(getCb), cbData_(cbData) {
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

    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
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
    ConfigStr(const char* name, size_t size, const char* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, char* ptr=nullptr, bool (*setCb)(char* val, void* cbData)=nullptr, void (*getCb)(char* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, FST("str"), info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}

    virtual size_t toStr(char* buffer, size_t size) {
        size_t n = 0;
        get();
        n += snprintf(buffer+n, size-n-1, FST("%s"), value_);
        buffer[n] = '\0';
        return n;
    }
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
        if (n < size-2) { buffer[n++]= '"'; }
        n += toStr(buffer+n, size-n - 1);
        if (n < size-2) { buffer[n++]= '"'; }
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
};


/*----------------------------------------------------------------------*\
 * Int32 Array
\*----------------------------------------------------------------------*/
class ConfigInt32Array : public ConfigArrayT<int32_t> {
public:
    ConfigInt32Array(const char* name, size_t size, const int32_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, int32_t* ptr=nullptr, bool (*setCb)(int32_t* val, void* cbData)=nullptr, void (*getCb)(int32_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, nullptr, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {
            sprintf(ths, FST("int32[%d]"), size);
            type_help_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Unsigned Int32 Array
\*----------------------------------------------------------------------*/
class ConfigUInt32Array : public ConfigArrayT<uint32_t> {
public:
    ConfigUInt32Array(const char* name, size_t size, const uint32_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint32_t* ptr=nullptr, bool (*setCb)(uint32_t* val, void* cbData)=nullptr, void (*getCb)(uint32_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, nullptr, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {
            sprintf(ths, FST("uint32[%d]"), size);
            type_help_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Int16 Array
\*----------------------------------------------------------------------*/
class ConfigInt16Array : public ConfigArrayT<int16_t> {
public:
    ConfigInt16Array(const char* name, size_t size, const int16_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, int16_t* ptr=nullptr, bool (*setCb)(int16_t* val, void* cbData)=nullptr, void (*getCb)(int16_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, nullptr, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {
            sprintf(ths, FST("int16[%d]"), size);
            type_help_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Unsigned Int16 Array
\*----------------------------------------------------------------------*/
class ConfigUInt16Array : public ConfigArrayT<uint16_t> {
public:
    ConfigUInt16Array(const char* name, size_t size, const uint16_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint16_t* ptr=nullptr, bool (*setCb)(uint16_t* val, void* cbData)=nullptr, void (*getCb)(uint16_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, nullptr, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {
            sprintf(ths, FST("uint16[%d]"), size);
            type_help_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Int8 Array
\*----------------------------------------------------------------------*/
class ConfigInt8Array : public ConfigArrayT<int8_t> {
public:
    ConfigInt8Array(const char* name, size_t size, const int8_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, int8_t* ptr=nullptr, bool (*setCb)(int8_t* val, void* cbData)=nullptr, void (*getCb)(int8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, nullptr, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {
            sprintf(ths, FST("int8[%d]"), size);
            type_help_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * Unsigned Int8 Array
\*----------------------------------------------------------------------*/
class ConfigUInt8Array : public ConfigArrayT<uint8_t> {
public:
    ConfigUInt8Array(const char* name, size_t size, const uint8_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t* val, void* cbData)=nullptr, void (*getCb)(uint8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, size, deflt, fmt, nullptr, info, group, ptr, setCb, getCb, cbData, hidden, persisted) {
            sprintf(ths, FST("uint8[%d]"), size);
            type_help_ = ths;
        }
protected:
    char ths[16];
};

/*----------------------------------------------------------------------*\
 * IP Address
\*----------------------------------------------------------------------*/
class ConfigIpAddr : public ConfigArrayT<uint8_t> {
public:
    ConfigIpAddr(const char* name, const uint8_t* deflt=nullptr, const char* fmt=nullptr, const char* info=nullptr, ConfigGroup* group=nullptr, uint8_t* ptr=nullptr, bool (*setCb)(uint8_t* val, void* cbData)=nullptr, void (*getCb)(uint8_t* val, void* cbData)=nullptr, void* cbData=nullptr, uint8_t hidden=0, bool persisted=true)
        : ConfigArrayT(name, 4, deflt, fmt, FST("IP_ADR"), info, group, ptr, setCb, getCb, cbData, hidden, persisted) {}

    virtual size_t toStr(char* buffer, size_t size) {
        size_t n = 0;
        get();
        n += snprintf(buffer+n, size-n-1, FST("%d.%d.%d.%d"), value_[0], value_[1], value_[2], value_[3]);
        buffer[n] = '\0';
        return n;
    }
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false) {
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

