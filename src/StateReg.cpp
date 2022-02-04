/*======================================================================*\
|* Status Variable Registry
|*
|* This framework provides a hierarchical registry for statuts
|* variables. Each variable can register change callbacks.
\*======================================================================*/

#include <Arduino.h>
#include "Config.h"
#include "StateReg.h"
#include "Helper.h"
#include "Command.h"

#if ENABLE_STATE_REG

StateGroup* StateGroup::mainGroup = nullptr;
size_t StateVar::count_ = 0;

/************************************************************************\
|* Config Commands
\************************************************************************/


CommandRegistry cmdRegState(FST("state"));
const char NO_STATE_VARS[] PROGMEM = "No state variables defined";

Command cmdGetState(FST("get"), 
[] (const char* args, Print* stream) {
    if (!StateGroup::mainGroup) { stream->println(NO_STATE_VARS); return EC_ERROR; }
    StateGroup::mainGroup->toJson(stream, true);
    stream->println();
    return EC_OK;
},
FST("Get state as JSON"), &cmdRegState,
nullptr, CT_APP_JSON
);

Command cmdStateVar(FST("var"), 
[] (const char* args, Print* stream) {
    if (!StateGroup::mainGroup) { stream->println(NO_STATE_VARS); return EC_ERROR; }
    StateVar* var = StateGroup::mainGroup->findVarByFullName(args, false);
    if (var) {
        if (stream) { var->print(*stream); }
    } else {
         if (stream) { stream->print(FST("unknown variable")); }
         return EC_NOT_FOUND;
    }
    if (stream) { stream->println(); }
    return EC_OK;
},
FST("Get state variable"), &cmdRegState, FST("<name>"), CT_APP_JSON
);

Command cmdGetStategUi(FST("ui"), 
[] (const char* args, Print* stream) {
    if (!StateGroup::mainGroup) { stream->println(NO_STATE_VARS); return EC_ERROR; }
    StateGroup::mainGroup->getWebUi(stream, true);
    stream->println();
    return EC_OK;
},
FST("Get state UI as JSON"), &cmdRegState,
nullptr, CT_APP_JSON
);


/************************************************************************\
|* Global Functions
\************************************************************************/

bool stateRegRun(uint32_t now) {
    static uint32_t stateRegNextTs = 0;
    if (!now) { now = millis(); }
    if (now < stateRegNextTs || !StateGroup::mainGroup) { return false; }
    stateRegNextTs = now + STATE_REG_CHECK_MS;
    if (!StateGroup::mainGroup->checkChange()) {
        return false;
    }
    return true;
}

/************************************************************************\
|* State Group
\************************************************************************/

bool StateGroup::checkChange() {
    isChanged_ = false;
    for(auto v: vars_) { isChanged_ |= v->checkChange(); }
    for(auto g: children_) { isChanged_ |= ((StateGroup*) g)->checkChange(); }
    if (isChanged_) {
        for(auto cb: callbacks_) { cb.callback(*this, cb.data); }
    }
    return isChanged_;
}

size_t StateGroup::printChangeJson(Print& out, char* namePrefix, size_t npSize, size_t npEnd/*=~0*/, size_t count/*=0*/) {
    if (!isChanged_) { return 0; }
    size_t n = 0;
    bool isOuter = false;
    if (npEnd == ~0) { 
        npEnd = 0; 
        isOuter = true;
        out.write('{'); 
    } 
    else {
        while (name_[n] && npEnd < npSize -2) { namePrefix[npEnd++] = name_[n++]; }
        if (npEnd < npSize -2) { namePrefix[npEnd++] = '.'; }
    }
    namePrefix[npEnd] = '\0';
    for(auto v: vars_) { 
        if(v->isChanged()) { 
            if (count) { out.write(','); }
            v->printChangeJson(out, namePrefix, npSize, npEnd);
            count++;
        }
    }
    for(auto g: children_) { 
        count += ((StateGroup*)g)->printChangeJson(out, namePrefix, npSize, npEnd, count); 
    }
    if (isOuter) { out.write('}'); }
    return count;
}
#endif // ENABLE_STATE_REG

#if 0
size_t StateGroup::toJsonStr(char* buffer, size_t size, bool noName, uint16_t flags/*=0*/, uint8_t flagsMask/*=0*/) {
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

size_t StateGroup::toJson(Print* stream, bool noName, uint16_t flags/*=0*/, uint8_t flagsMask/*=0*/) {
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

size_t StateGroup::getWebUi(Print* stream, bool noName, uint16_t flags/*=0*/, uint8_t flagsMask/*=0*/) {
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

StateGroup* StateGroup::findChild(const char* name) {
    for(auto g: children_) {
      if (StrTool::matchesCleanName(name, g->name())) { return g; }
    }
    return nullptr;
}

StateVar* StateGroup::findVar(const char* name) {
    for(auto v: vars_) {
      if (StrTool::matchesCleanName(name, v->name())) { return v; }
    }
    return nullptr;
}

StateVar* StateGroup::findVarByFullName(const char* name, bool matchCase/*=true*/) {
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

StateVar* StateGroup::get(size_t n) {
    if (n >= varCount_) { return nullptr; }
    if (n < vars_.size()) { return vars_.at(n); }
    n -= vars_.size();
    for(auto g: children_) {
      if (n < g->size()) { return g->get(n); }
      n -= g->size();
    }
    return nullptr;
}

size_t StateGroup::getVarName(char* buffer, size_t size, size_t index) {

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


std::vector<StateVar*>::iterator StateGroup::getIt(size_t n) {
    if (n >= varCount_) { return vars_.end(); }
    if (n <= vars_.size()) { return vars_.begin() + n; }
    n -= vars_.size();
    for(auto g: children_) {
      if (n <= g->size()) { return g->getIt(n); }
      n -= g->size();
    }
    return vars_.end();
}
#endif