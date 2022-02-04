/*======================================================================*\
|* Status Variable Registry
|*
|* This framework provides a hierarchical registry for statuts
|* variables. Each variable can register change callbacks.
\*======================================================================*/

#ifndef _STATE_REG_H_
#define _STATE_REG_H_

#if ENABLE_STATE_REG
#include <Arduino.h>
#include <vector>
#include <iterator>
#include <cstddef>

#include "Config.h"
#include "Helper.h"
#include "RegGroup.h"

// Disable warning about char being deprecated. Not sure why it's needed...
#ifndef ARDUINOJSON_DEPRECATED
#define ARDUINOJSON_DEPRECATED(msg)
#endif
#include <ArduinoJson.h>


#ifndef STATE_REG_CHECK_MS
#define STATE_REG_CHECK_MS 100
#endif

/************************************************************************\
|* Flags
\************************************************************************/

static const uint8_t SVF_HIDDEN =        (uint8_t)(1<<0);


/************************************************************************\
|* Global Functions
\************************************************************************/

bool stateRegRun(uint32_t now=0);

/************************************************************************\
|* State Group organizes State Variables in a hierarchical tree
\************************************************************************/

class StateVar;
class StateGroup : public GroupRegT<StateVar> {
public:
  typedef void (*CallbackFunc)(StateGroup& group, void* data);
  typedef struct Callback {
      CallbackFunc callback;
      void* data;
  } Callback;

    StateGroup(const char* name, GroupRegT* parent=nullptr, const char* info=nullptr, uint8_t flags=0) 
        : GroupRegT(name, parent, info, flags=0) {
        checkMain();
        if (parent_ == nullptr) { parent_ = mainGroup; }
        if (parent_ && parent_ != (GroupRegT*)-1) { parent_->addChild(this); }
    }

  static void checkMain() {
      if (!mainGroup) {
          mainGroup = (StateGroup*)-1; // Dummy value. Prevent infinite recursion
          mainGroup = new StateGroup(FST("main"), (StateGroup*)-1);
      }
    }

  inline bool isHidden() { return flags_ & SVF_HIDDEN; } 
  inline bool isChanged() { return isChanged_; } 
  void addCallback(CallbackFunc cb, void* data=nullptr) { callbacks_.push_back({cb, data}); }
  void removeCallback(CallbackFunc cb, void* data) {
    callbacks_.erase(
        std::remove_if(callbacks_.begin(), callbacks_.end(), [&](Callback const & c) {
            return c.callback == cb && c.data == data;}), callbacks_.end());
  }
  bool checkChange();
  size_t printChangeJson(Print& out, char* namePrefix, size_t npSize, size_t npEnd=~0, size_t count=0);

  static StateGroup* mainGroup;

  protected:
    bool isChanged_;
    std::vector<Callback> callbacks_;
};

/************************************************************************\
|* State Variable Base Class
\************************************************************************/
class StateVar {
public:
  StateVar(const char* name, const char* typeHelp=nullptr, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, uint8_t flags=0)
    : name_(name), fmt_(fmt), typeHelp_(typeHelp), info_(info), group_(group), flags_(flags), id_(count_++) {
        if (!typeHelp_) { typeHelp_ = FST("value"); }
        if (!info_) { info_ = FST(""); }
        if (!group_) { 
            StateGroup::checkMain();
            group_ = StateGroup::mainGroup; 
        }
        group_->addVar(this);
    }
    virtual ~StateVar() {
        if (group_) {
            group_->removeVar(this);
            group_ = nullptr;
        }

    }
    virtual size_t print(Print& stream) = 0;
    virtual void printChangeJson(Print& out, char* namePrefix=nullptr, size_t npSize=0, size_t npEnd=0) = 0;
    virtual size_t toStr(char* buffer, size_t size) = 0;
    virtual size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) = 0;
    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) = 0;
    virtual bool checkChange() = 0;
    inline const char* name() { return name_; }
    inline const char* fmt() { return fmt_; }
    inline const char* typeHelp() { return typeHelp_; }
    inline const char* info() { return info_; }
    inline bool isChanged() { return isChanged_; } 
    inline uint8_t flags() { return flags_; }
    inline bool isHidden() { return flags_ & SVF_HIDDEN; } 
    inline size_t id() { return id_; }   

protected:
    virtual size_t getWebUiCommon_(Print* stream) {
        size_t n = 0;
        char buffer[64];
        StrTool::toCleanName(buffer, sizeof(buffer), name_);
        n += stream->printf(FST("{\"E\":1")); // Entry Type: State
        n += stream->printf(FST(",\"P\":\"%s\""), buffer); // Parameter Name
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
    StateGroup* group_;
    bool isChanged_;
    uint8_t flags_;
    size_t id_;

    static size_t count_;
};


/************************************************************************\
|* State Variable Template Class for individual values
\************************************************************************/
template <class T> class StateVarT : public StateVar {
public:
    typedef void (*CallbackFunc)(StateVar& var, T value, void* data);
    typedef struct Callback {
        CallbackFunc callback;
        void* data;
    } Callback;
    StateVarT(const char* name, const T value=0, const char* typeHelp=nullptr, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, T* ptr=nullptr, T (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVar(name, typeHelp, info, fmt, group, flags), value_(value), oldValue_(value), ptr_(ptr), getCb_(getCb), cbData_(cbData), callbacks_(nullptr) {
        if (!fmt) { fmt_ = FST("%d"); }
    }
    virtual ~StateVarT() {
        if (callbacks_) {
            delete callbacks_; callbacks_ = nullptr;
        }
    }

    virtual inline T* getValueRef() { return &value_; }

    virtual T get() {
        if (ptr_) { value_ = *ptr_; }
        if (getCb_) { value_ = getCb_(cbData_); }
        return value_;
    }

    virtual size_t print(Print& stream) {
        return stream.printf(fmt_, get());
    }

    virtual void printChangeJson(Print& out, char* namePrefix=nullptr, size_t npSize=0, size_t npEnd=0) {
        size_t n = 0;
        while (name_[n] && npEnd < npSize -2) { namePrefix[npEnd++] = name_[n++]; }
        namePrefix[npEnd] = '\0';
        char buffer[128];
        n = StrTool::toJsonName(buffer, sizeof(buffer), namePrefix);
        out.write(buffer, n); 
        out.print(oldValue_);
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

    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = getWebUiCommon_(stream);
        get();
        n += stream->print(FST(",\"V\":"));
        n += stream->print(value_);
        stream->write('}'); n++;
        return n;
    }

    virtual void addCallback(CallbackFunc cb, void* data=nullptr) {
        if (!callbacks_) {
            callbacks_ = new std::vector<Callback>;
        }
        callbacks_->push_back({cb, data});
    }

    virtual void removeCallback(CallbackFunc cb, void* data) {
        if (!callbacks_) { return; }
        callbacks_->erase(
            std::remove_if(callbacks_->begin(), callbacks_->end(), [&](Callback const & c) {
            return c.callback == cb && c.data == data;
            }),
        callbacks_->end());
    }

    virtual bool checkChange() {
        get();
        if (oldValue_ == value_) {
            isChanged_ = false;
            return false;
        }
        oldValue_ = value_;
        isChanged_ = true;
        if (callbacks_) {
            for(auto cb: *callbacks_) { cb.callback(*this, value_, cb.data); }
        }
        return true;
    }

protected:
    T value_;
    T oldValue_;
    T* ptr_;
    T (*getCb_)(void* cbData);
    void* cbData_;
    std::vector<Callback>* callbacks_;
};

/*----------------------------------------------------------------------*\
 * Boolean
\*----------------------------------------------------------------------*/
class StateBool : public StateVarT<bool> {
public:
    StateBool(const char* name, bool value=0, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, bool* ptr=nullptr, bool (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, value, FST("bool"), info, fmt, group, ptr, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Int32
\*----------------------------------------------------------------------*/
class StateInt32 : public StateVarT<int32_t> {
public:
    StateInt32(const char* name, int32_t value=0, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, int32_t* ptr=nullptr, int32_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, value, FST("int32"), info, fmt, group, ptr, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int32
\*----------------------------------------------------------------------*/
class StateUInt32 : public StateVarT<uint32_t> {
public:
    StateUInt32(const char* name, uint32_t value=0, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, uint32_t* ptr=nullptr, uint32_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, value, FST("uint32"), info, fmt, group, ptr, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Int16
\*----------------------------------------------------------------------*/
class StateInt16 : public StateVarT<int16_t> {
public:
    StateInt16(const char* name, int16_t value=0, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, int16_t* ptr=nullptr, int16_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, value, FST("int16"), info, fmt, group, ptr, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int16
\*----------------------------------------------------------------------*/
class StateUInt16 : public StateVarT<uint16_t> {
public:
    StateUInt16(const char* name, uint16_t value=0, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, uint16_t* ptr=nullptr, uint16_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, value, FST("uint16"), info, fmt, group, ptr, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Int8
\*----------------------------------------------------------------------*/
class StateInt8 : public StateVarT<int8_t> {
public:
    StateInt8(const char* name, int8_t value=0, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, int8_t* ptr=nullptr, int8_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, value, FST("int8"), info, fmt, group, ptr, getCb, cbData, flags) {}
};

/*----------------------------------------------------------------------*\
 * Unsigned Int8
\*----------------------------------------------------------------------*/
class StateUInt8 : public StateVarT<uint8_t> {
public:
    StateUInt8(const char* name, uint8_t value=0, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, uint8_t* ptr=nullptr, uint8_t (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, value, FST("uint8"), info, fmt, group, ptr, getCb, cbData, flags) {}
};


/*----------------------------------------------------------------------*\
 * String
\*----------------------------------------------------------------------*/
class StateStr : public StateVarT<char*> {
public:
    StateStr(const char* name, const char* value=nullptr, const char* info=nullptr, const char* fmt=nullptr, StateGroup* group=nullptr, char** ptr=nullptr, char* (*getCb)(void* cbData)=nullptr, void* cbData=nullptr, uint8_t flags=0)
        : StateVarT(name, (char*)value, FST("str"), info, fmt, group, ptr, getCb, cbData, flags) {
            if (!fmt) { fmt_ = FST("%s"); }
            //get();
            crc_ = StrTool::calculateStrCrc(value_);
        }

    virtual size_t print(Print& stream) {
        get();
        return stream.printf(fmt_, value_);
    }

    virtual void printChangeJson(Print& out, char* namePrefix=nullptr, size_t npSize=0, size_t npEnd=0) {
        size_t n = 0;
        while (name_[n] && npEnd < npSize -2) { namePrefix[npEnd++] = name_[n++]; }
        namePrefix[npEnd] = '\0';
        char buffer[128];
        n = StrTool::toJsonName(buffer, sizeof(buffer), namePrefix);
        out.write(buffer, n); 
        out.write('"');
        out.print(oldValue_);
        out.write('"');
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
        n += toStr(buffer+n, size-n - 1);
        if (n < size-2) { buffer[n++] = '"'; }
        buffer[n]= '\0';
        return n;
    }

    virtual size_t getWebUi(Print* stream, uint16_t flags=0, uint8_t flagsMask=0) {
        if ((flags_ ^ flags) & flagsMask) { return 0; }
        size_t n = getWebUiCommon_(stream);
        get();
        n += stream->printf(FST(",\"V\":\"%s\""), value_);
        // n += stream->printf(FST(",\"S\":%d"), size_-1);  // Max
        stream->write('}'); n++;        
        return n;
    }

    virtual bool checkChange() {
        get();
        uint32_t crc = StrTool::calculateStrCrc(value_);
        if (crc == crc_ && oldValue_ == value_) {
            isChanged_ = false;
            return false;
        }
        oldValue_ = value_;
        crc_ = crc;
        isChanged_ = true;
        if (callbacks_) {
            for(auto cb: *callbacks_) { cb.callback(*this, value_, cb.data); }
        }
        return true;
    }

protected: 
    uint32_t crc_;    
};

#endif // ENABLE_STATE_REG
#endif // _STATE_REG_H_

