#pragma once

#include "ofUtils.h"
#include "ofJson.h"

namespace ofxCortex::core::types {

template<typename T>
struct Select {
protected:
  std::vector<std::pair<T, std::string>> items;
  size_t selectedIndex;
  
public:
  Select() : selectedIndex(0) {}
  Select(const std::vector<std::pair<T, std::string>>& items, size_t index = 0)
  : items(items), selectedIndex(index % items.size()) {}
  
  Select(std::initializer_list<std::pair<T, std::string>> list, size_t index = 0)
  : items(list), selectedIndex(index % list.size()) {}
  
  Select(const Select& other) : items(other.items), selectedIndex(other.selectedIndex) {}
  
  // Copy assignment operator
  Select& operator=(const Select& other) {
    if (this != &other) {  // Check for self-assignment
      items = other.items;
      selectedIndex = other.selectedIndex;
    }
    return *this;
  }
  
  // Add an item (value and string representation)
  void add(const T& value, const std::string& str) {
    items.push_back(std::make_pair(value, str));
  }
  
  void add(const std::vector<std::pair<T, std::string>> & _items) {
    items.insert(items.end(), _items.begin(), _items.end());
  }
  
  // Total number of items
  size_t size() const { return items.size(); }
  
  operator T() const { return items[selectedIndex].first; }
  const T& selected() const { return items[selectedIndex].first; }
  const T& getSelected() const { return items[selectedIndex].first; }
  
  const T& operator[](size_t index) const { return items[index % size()].first; }
  
  size_t getSelectedIndex() const { return selectedIndex; }
  void setSelectedIndex(size_t index) {
    selectedIndex = index % size();
  }
  
  void setSelectedValue(const T & value) {
    size_t indexOfValue = ofxCortex::core::utils::Array::findIndex(items, [&](const std::pair<T, std::string> & item){ return item.first == value; });
    if (indexOfValue != -1) setSelectedIndex(indexOfValue);
  }
  
  operator std::string() const { return items[selectedIndex].second; }
  const std::string& getSelectedString() const {
    return items[selectedIndex].second;
  }
  
  std::vector<T> getKeys() const {
    std::vector<T> output;
    for (auto & [key, _] : items) output.push_back(key);
    return output;
  }
  
  std::vector<std::string> getValues() const {
    std::vector<std::string> output;
    for (auto & [_, value] : items) output.push_back(value);
    return output;
  }
  
  // Get the vector of const char* for ImGui::Combo
  std::vector<const char*> getItemStrings() const {
    std::vector<const char*> strings;
    for (const auto& item : items) {
      strings.push_back(item.second.c_str());  // Use the stored string
    }
    return strings;
  }
  
  std::string toString()
  {
    std::stringstream stream;
    stream << "Select<>" << '\n';
    for (int i = 0; i < items.size(); i++)
    {
      const auto & [value, label] = items[i];
      
      stream << '\t' << (i == selectedIndex ? "●" : "○") << " " << value << " => " << label << '\n';
    }
    return stream.str();
  }
  
  // Simple Serialization
  friend std::ostream& operator<<(std::ostream& os, const Select<T>& select)
  {
    os << select.selectedIndex;
    return os;
  }
  
  friend std::istream& operator>> (std::istream &is, Select<T>& select)
  {
    is >> select.selectedIndex;
    return is;
  }
  
//  virtual ofJson toJSON() const {
//    ofJson output;
//    output["selectedIndex"] = this->selectedIndex;
//    return output;
//  }
//  
//  virtual void fromJSON(const ofJson & json)
//  {
//    this->selectedIndex = json["selectedIndex"].get<int>();
//  }
};

}
