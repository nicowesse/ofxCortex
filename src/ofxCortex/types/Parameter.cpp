#include "Parameter.h"

using std::string;
using std::endl;
using std::setw;
using std::weak_ptr;
using std::shared_ptr;
using std::vector;
using std::map;
using std::ostream;
using std::stringstream;

namespace ofxCortex {

string AbstractParameter::getEscapedName() const {
  return escape(getName());
}


string AbstractParameter::escape(const string& _str) const{
  
  std::string str(_str);
  
  ofStringReplace(str, " ", "_");
  ofStringReplace(str, "<", "_");
  ofStringReplace(str, ">", "_");
  ofStringReplace(str, "{", "_");
  ofStringReplace(str, "}", "_");
  ofStringReplace(str, "[", "_");
  ofStringReplace(str, "]", "_");
  ofStringReplace(str, ",", "_");
  ofStringReplace(str, "(", "_");
  ofStringReplace(str, ")", "_");
  ofStringReplace(str, "/", "_");
  ofStringReplace(str, "\\", "_");
  ofStringReplace(str, ".", "_");
  ofStringReplace(str, "#", "_");
  
  return str;
}


string AbstractParameter::type() const{
  return typeid(*this).name();
}

std::vector<string> AbstractParameter::getGroupHierarchyNames() const{
  std::vector<string> hierarchy;
  auto p = getFirstParent();
  if(p){
    hierarchy = p.getGroupHierarchyNames();
  }
  hierarchy.push_back(getEscapedName());
  return hierarchy;
}

bool AbstractParameter::isReferenceTo(const AbstractParameter &other) const{
  return getInternalObject() == other.getInternalObject();
}

std::ostream& operator<<(std::ostream& os, const AbstractParameter& p){
  os << p.toString();
  return os;
}

std::istream& operator>>(std::istream& is, AbstractParameter& p){
  string str;
  is >> str;
  p.fromString(str);
  return is;
}

Group & AbstractParameter::castGroup(){
  return static_cast<Group &>(*this);
}

const Group & AbstractParameter::castGroup() const{
  return static_cast<const Group &>(*this);
}



Group::Group()
:obj(new Value)
{

}

void Group::add(AbstractParameter & parameter){
  shared_ptr<AbstractParameter> param = parameter.newReference();
  const std::string name = param->getEscapedName();
  if(obj->parametersIndex.find(name) != obj->parametersIndex.end()){
    ofLogWarning() << "Adding another parameter with same name '" << param->getName() << "' to group '" << getName() << "'";
  }
  obj->parameters.push_back(param);
  obj->parametersIndex[name] = obj->parameters.size()-1;
  param->setParent(*this);
}

void Group::remove(AbstractParameter &param){
  for(auto & p: obj->parameters){
    if(p->isReferenceTo(param)){
      remove(param.getName());
      return;
    }
  }
}

void Group::remove(size_t index){
  if(index>obj->parameters.size()){
    return;
  }
  remove(obj->parameters[index]->getName());
}

void Group::remove(const string &name){
  auto escaped = escape(name);
  if(!contains(escaped)){
    return;
  }
  size_t paramIndex = obj->parametersIndex[escaped];
  obj->parameters.erase(obj->parameters.begin() + paramIndex);
  obj->parametersIndex.erase(escaped);
  std::for_each(obj->parameters.begin() + paramIndex, obj->parameters.end(), [&](shared_ptr<AbstractParameter>& p){
    obj->parametersIndex[p->getEscapedName()] -= 1;
  });
}

void Group::clear(){
  auto name = this->getName();
  obj.reset(new Value);
  setName(name);
}

string Group::valueType() const{
  return typeid(*this).name();
}

const Group & Group::getGroup(const string& name) const{
  return static_cast<const Group&>(get(name));
}

const Group & Group::getGroup(std::size_t pos) const{
  if(pos>=size()){
    throw std::out_of_range(("get(): " + ofToString(pos) + "out of bounds").c_str());
  }else{
    if(getType(pos)==typeid(Group).name()){
      return *static_cast<Group* >(obj->parameters[pos].get());
    }else{
      throw std::runtime_error(("get():  bad type for pos " + ofToString(pos) + ", returning empty group").c_str());
    }
  }
}

Group & Group::getGroup(const string& name){
  return static_cast<Group& >(get(name));
}

Group & Group::getGroup(std::size_t pos){
  if(pos>=size()){
    throw std::out_of_range(("get(): " + ofToString(pos) + "out of bounds").c_str());
  }else{
    if(getType(pos)==typeid(Group).name()){
      return *static_cast<Group* >(obj->parameters[pos].get());
    }else{
      throw std::runtime_error(("get():  bad type for pos " + ofToString(pos) + ", returning empty group").c_str());
    }
  }
}


std::size_t Group::size() const{
  return obj->parameters.size();
}

string Group::getName(std::size_t position) const{
  if(position>=size()){
    return "";
  }else{
    return obj->parameters[position]->getName();
  }
}

string Group::getType(std::size_t position) const{
  if(position>=size()) return "";
  else return obj->parameters[position]->type();
}


int Group::getPosition(const string& name) const{
  if(obj->parametersIndex.find(escape(name))!=obj->parametersIndex.end())
    return obj->parametersIndex.find(escape(name))->second;
  return -1;
}

string Group::getName() const{
  return obj->name;
}

void Group::setName(const string & name){
  obj->name = name;
}

string Group::getEscapedName() const{
  if(getName()==""){
    return "group";
  }else{
    return AbstractParameter::getEscapedName();
  }
}

string Group::toString() const{
  stringstream out;
  out << *this;
  return out.str();
}

void Group::fromString(const string & name){
  ofLogWarning() << "Group doesn't implement fromString yet";
}


const AbstractParameter & Group::get(const string& name) const{
  map<string,std::size_t>::const_iterator it = obj->parametersIndex.find(escape(name));
  std::size_t index = it->second;
  return get(index);
}

const AbstractParameter & Group::get(std::size_t pos) const{
  return *obj->parameters[pos];
}


const AbstractParameter & Group::operator[](const string& name) const{
  return get(name);
}

const AbstractParameter & Group::operator[](std::size_t pos) const{
  return get(pos);
}

AbstractParameter & Group::get(const string& name){
  map<string,std::size_t>::const_iterator it = obj->parametersIndex.find(escape(name));
  std::size_t index = it->second;
  return get(index);
}

AbstractParameter & Group::get(std::size_t pos){
  return *obj->parameters[pos];
}


AbstractParameter & Group::operator[](const string& name){
  return get(name);
}

AbstractParameter & Group::operator[](std::size_t pos){
  return get(pos);
}

ostream& operator<<(ostream& os, const Group& group) {
  std::streamsize width = os.width();
  for(std::size_t i=0;i<group.size();i++){
    if(group.getType(i)==typeid(Group).name()){
      os << group.getName(i) << ":" << endl;
      os << setw(width+4);
      os << group.getGroup(i);
    }else{
      os << group.getName(i) << ":" << group.get(i) << endl;
      os << setw(width);
    }
  }
  return os;
}

bool Group::contains(const string& name) const{
  return obj->parametersIndex.find(escape(name))!=obj->parametersIndex.end();
}

void Group::Value::notifyParameterChanged(AbstractParameter & param){
  ofNotifyEvent(parameterChangedE,param);
  parents.erase(std::remove_if(parents.begin(),parents.end(),[&param](const weak_ptr<Value> & p){
    auto parent = p.lock();
    if(parent) parent->notifyParameterChanged(param);
    return !parent;
  }),parents.end());
}

const Group Group::getFirstParent() const{
  auto first = std::find_if(obj->parents.begin(),obj->parents.end(),[](const weak_ptr<Value> & p){return p.lock()!=nullptr;});
  if(first!=obj->parents.end()){
    return first->lock();
  }else{
    return shared_ptr<Value>(nullptr);
  }
}

ofEvent<AbstractParameter> & Group::parameterChangedE(){
  return obj->parameterChangedE;
}

AbstractParameter & Group::back(){
  return *obj->parameters.back();
}

AbstractParameter & Group::front(){
  return *obj->parameters.front();
}

const AbstractParameter & Group::back() const{
  return *obj->parameters.back();
}

const AbstractParameter & Group::front() const{
  return *obj->parameters.front();
}

void Group::setSerializable(bool _serializable){
  obj->serializable = _serializable;
}

bool Group::isSerializable() const{
  return obj->serializable;
}

bool Group::isReadOnly() const{
  return false;
}

const void* Group::getInternalObject() const{
  return obj.get();
}

shared_ptr<AbstractParameter> Group::newReference() const{
  return std::make_shared<Group>(*this);
}

void Group::setParent(Group & parent){
  obj->parents.emplace_back(parent.obj);
}

Group::operator bool() const{
  return obj != nullptr;
}

vector<shared_ptr<AbstractParameter> >::iterator Group::begin(){
  return obj->parameters.begin();
}

vector<shared_ptr<AbstractParameter> >::iterator Group::end(){
  return obj->parameters.end();
}

vector<shared_ptr<AbstractParameter> >::const_iterator Group::begin() const{
  return obj->parameters.begin();
}

vector<shared_ptr<AbstractParameter> >::const_iterator Group::end() const{
  return obj->parameters.end();
}

vector<shared_ptr<AbstractParameter> >::reverse_iterator Group::rbegin(){
  return obj->parameters.rbegin();
}

vector<shared_ptr<AbstractParameter> >::reverse_iterator Group::rend(){
  return obj->parameters.rend();
}

vector<shared_ptr<AbstractParameter> >::const_reverse_iterator Group::rbegin() const{
  return obj->parameters.rbegin();
}

vector<shared_ptr<AbstractParameter> >::const_reverse_iterator Group::rend() const{
  return obj->parameters.rend();
}



Parameter<void>::Parameter()
:obj(new Value){

}

Parameter<void>::Parameter(const string& name)
:obj(new Value(name)){

}

void Parameter<void>::setName(const string & name){
  obj->name = name;
}

string Parameter<void>::getName() const{
  return obj->name;
}

std::string Parameter<void>::toString() const{
  return "";
}

string Parameter<void>::valueType() const{
  return typeid(void).name();
}

void Parameter<void>::fromString(const std::string & name){

}

Parameter<void>& Parameter<void>::set(const std::string & name){
  setName(name);
  return *this;
}

void Parameter<void>::trigger(){
  ofNotifyEvent(obj->changedE,this);
    // Notify all parents, if there are any.
    if(!obj->parents.empty())
    {
        // Erase each invalid parent
        obj->parents.erase(std::remove_if(obj->parents.begin(),
                                          obj->parents.end(),
                                          [](const std::weak_ptr<Group::Value> & p){ return p.expired(); }),
                           obj->parents.end());
        
        // notify all leftover (valid) parents of this object's changed value.
        // this can't happen in the same iterator as above, because a notified listener
        // might perform similar cleanups that would corrupt our iterator
        // (which appens for example if the listener calls getFirstParent on us)
        for(auto & parent: obj->parents){
            auto p = parent.lock();
            if(p){
                p->notifyParameterChanged(*this);
            }
        }
    }
}

void Parameter<void>::trigger(const void * sender){
  ofNotifyEvent(obj->changedE,sender);
    // Notify all parents, if there are any.
    if(!obj->parents.empty())
    {
        // Erase each invalid parent
        obj->parents.erase(std::remove_if(obj->parents.begin(),
                                          obj->parents.end(),
                                          [](const std::weak_ptr<Group::Value> & p){ return p.expired(); }),
                           obj->parents.end());
        
        // notify all leftover (valid) parents of this object's changed value.
        // this can't happen in the same iterator as above, because a notified listener
        // might perform similar cleanups that would corrupt our iterator
        // (which appens for example if the listener calls getFirstParent on us)
        for(auto & parent: obj->parents){
            auto p = parent.lock();
            if(p){
                p->notifyParameterChanged(*this);
            }
        }
    }
}

void Parameter<void>::enableEvents(){
  obj->changedE.enable();
}

void Parameter<void>::disableEvents(){
  obj->changedE.disable();
}

bool Parameter<void>::isSerializable() const{
  return obj->serializable;
}

bool Parameter<void>::isReadOnly() const{
  return false;
}

void Parameter<void>::makeReferenceTo(Parameter<void> & mom){
  *this = mom;
}

void Parameter<void>::setSerializable(bool serializable){
  obj->serializable = serializable;
}

std::shared_ptr<AbstractParameter> Parameter<void>::newReference() const{
  return std::make_shared<Parameter<void>>(*this);
}

void Parameter<void>::setParent(Group & parent){
  obj->parents.emplace_back(parent.obj);
}

size_t Parameter<void>::getNumListeners() const{
  return obj->changedE.size();
}

};
