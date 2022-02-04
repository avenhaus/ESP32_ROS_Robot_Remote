/*======================================================================*\
|* Status Variable Registry
|*
|* This framework provides a hierarchical registry for statuts
|* variables. Each variable can register change callbacks.
\*======================================================================*/

#ifndef _REG_GROUP_H_
#define _REG_GROUP_H_

#include <Arduino.h>
#include <vector>
#include <iterator>
#include <cstddef>

#include "Config.h"
#include "Helper.h"

// Disable warning about char being deprecated. Not sure why it's needed...
#ifndef ARDUINOJSON_DEPRECATED
#define ARDUINOJSON_DEPRECATED(msg)
#endif
#include <ArduinoJson.h>

/************************************************************************\
|* Global Functions
\************************************************************************/

/************************************************************************\
|* State Group organizes State Variables in a hierarchical tree
\************************************************************************/

template <class T> class GroupRegT {
public:

    /*------------------------------------------------------------------*\
     * Recursive iterator over all the (child) variables of this group.
    \*------------------------------------------------------------------*/
    class Iterator 
    {
        public: 
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T&;
        using pointer           = T*;  // or also value_type*
        using reference         = T&;  // or also value_type&

        Iterator(GroupRegT& group, size_t index) : group_(group), index_(index) {
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
        GroupRegT& group_;
        size_t index_;
        T* varP_;
    };


    /*------------------------------------------------------------------*\
     * State Group
    \*------------------------------------------------------------------*/
    GroupRegT(const char* name, GroupRegT* parent=nullptr, const char* info=nullptr, uint8_t flags=0) 
      : name_(name), parent_(parent), info_(info), flags_(flags), varCount_(0) {}
    virtual ~GroupRegT() {
        if(parent_) {
            parent_->removeChild(this);
            parent_ = nullptr;
        }
    }
    void addChild(GroupRegT* child) { children_.push_back(child); updateVarCount_(child->size()); }
    void removeChild(GroupRegT* child) { children_.erase(remove(children_.begin(), children_.end(), child), children_.end());  updateVarCount_(-child->size()); }
    void addVar(T* var) { vars_.push_back(var); updateVarCount_(1); }
    void removeVar(T* var) { size_t tmp=vars_.size(); vars_.erase(remove(vars_.begin(), vars_.end(), var), vars_.end()); updateVarCount_(vars_.size() - tmp); }
    inline const char* name() { return name_; }
    inline size_t size() { return varCount_; }
    inline size_t childrenSize() { return varCount_ - vars_.size(); }
    inline std::vector<GroupRegT*>& children() { return children_; }
    inline std::vector<T*>& vars() { return vars_; }
    inline const char* info() { return info_; }
    inline uint8_t flags() { return flags_; }
    Iterator begin() { return Iterator(*this, 0); }
    Iterator end()   { return Iterator(*this, size()); } 

    size_t toJsonStr(char* buffer, size_t size, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) {
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
        if (n < size-1) { buffer[n++] = '{'; }
        bool first = true;
        for(auto v: vars_) {
        if (!first && (n < size-1)) { buffer[n++] = ','; }
        first = false; 
        n += v->toJsonStr(buffer+n, size-n, false, flags, flagsMask);
        }
        for(auto g: children_) {
        if (!first && (n < size-1)) { buffer[n++] = ','; }
        first = false; 
        n += g->toJsonStr(buffer+n, size-n, false, flags, flagsMask);
        }
        if (n < size-1) { buffer[n++] = '}'; }
        buffer[n] = '\0';
        return n;
    }

    size_t toJson(Print* stream, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) {
        char buffer[128];
        size_t size = sizeof(buffer);
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer, size, name_); }
        if (n < size-1) { buffer[n++] = '{'; }
        stream->write(buffer, n);
        bool first = true;
        for(auto v: vars_) {
        if (!first) { stream->write(','); n++; }
        first = false; 
        size_t m = v->toJsonStr(buffer, size, false, flags, flagsMask);
        stream->write(buffer, m);
        n += m;
        }
        for(auto g: children_) {
        if (!first) { stream->write(','); n++; }
        first = false; 
        n += g->toJson(stream, false, flags, flagsMask);
        }
        stream->write('}'); n++;
        return n;
    }

    size_t getWebUi(Print* stream, bool noName=false, uint16_t flags=0, uint8_t flagsMask=0) {
        char buffer[128];
        size_t size = sizeof(buffer);
        size_t n = 0;
        if (!noName) { n += StrTool::toJsonName(buffer, size, name_); }
        stream->write(buffer, n);
        n += stream->print(FST("{\"_INFO_\":{"));
        n += stream->printf(FST("\"L\":\"%s\""), name_);
        if (info_) {n += stream->printf(FST(",\"H\":\"%s\""), info_); }
        if (flags_) { n += stream->printf(FST(",\"F\":%d"), flags_); }
        bool first = true;
        n += stream->print(FST("},\"_VARS_\":["));
        for(auto v: vars_) {
        if (!v->isHidden()) {
            if (!first) { stream->write(','); n++; }
            first = false; 
            size_t m = v->getWebUi(stream, flags, flagsMask);
            n += m;
        }
        }
        stream->write(']'); n++;
        for(auto g: children_) {
        if (!first) { stream->write(','); n++; }
        first = false; 
        n += g->getWebUi(stream, false, flags, flagsMask);
        }
        stream->write('}'); n++;
        return n;
    }

    GroupRegT* findChild(const char* name) {
        for(auto g: children_) {
        if (StrTool::matchesCleanName(name, g->name())) { return g; }
        }
        return nullptr;
    }

    T* findVar(const char* name) {
        for(auto v: vars_) {
        if (StrTool::matchesCleanName(name, v->name())) { return v; }
        }
        return nullptr;
    }

    T* findVarByFullName(const char* name, bool matchCase=true) {
        size_t n = 0;
        for(auto v: vars_) {
        n = StrTool::matchesNamePart(name, v->name(), matchCase);
        if (n && name[n] != '.') { return v; }
        }
        for(auto g: children_) {
        n = StrTool::matchesNamePart(name, g->name(), matchCase);
        if (n && name[n] == '.') { return g->findVarByFullName(name + n + 1, matchCase); }
        }
        return nullptr;
    }

    T* get(size_t n) {
        if (n >= varCount_) { return nullptr; }
        if (n < vars_.size()) { return vars_.at(n); }
        n -= vars_.size();
        for(auto g: children_) {
        if (n < g->size()) { return g->get(n); }
        n -= g->size();
        }
        return nullptr;
    }

    size_t getVarName(char* buffer, size_t size, size_t index) {

        if (index >= varCount_) { return 0; }
        if (index < vars_.size()) { 
        return StrTool::toCleanName(buffer, size, vars_.at(index)->name()); 
        }
        index -= vars_.size();
        for(auto g: children_) {
        if (index < g->size()) { 
            size_t n = StrTool::toCleanName(buffer, size, g->name_);
            if (n < size -1) { buffer[n++] = '.'; }
            buffer[n] = '\0';
            return g->getVarName(buffer+n, size-n, index); 
        }
        index -= g->size();
        }
        return 0;
    }


    typename std::vector<T*>::iterator getIt(size_t n) {
        if (n >= varCount_) { return vars_.end(); }
        if (n <= vars_.size()) { return vars_.begin() + n; }
        n -= vars_.size();
        for(auto g: children_) {
        if (n <= g->size()) { return g->getIt(n); }
        n -= g->size();
        }
        return vars_.end();
    }


protected:
    size_t updateVarCount_(size_t n) {
        varCount_ += n;
        if (parent_ && parent_ != (GroupRegT*)-1) { parent_->updateVarCount_(n); }
        return varCount_;
    }

    const char* name_;
    GroupRegT<T>* parent_;
    const char* info_;
    std::vector<GroupRegT<T>*> children_;
    std::vector<T*> vars_;
    uint8_t flags_;
    size_t varCount_;
};

#endif // _GROUP_REG_H_

