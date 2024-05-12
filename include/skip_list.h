/* ************************************************************************

> File Name:     skip_list.h
> Author:        beanljun
> Created Time:  04/19/2024
> Description:   This is a lightweight key-value storage engine based on skip list.

 ************************************************************************/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <memory>
#include <random>

#define STORE_FILE "../config/dumpFile" // 转存的文件名

namespace node
{
  // 定义K-V节点
  template <typename K, typename V>
  class Node
  {
  public:
    Node() {} // 默认构造函数
    ~Node() {} // 析构函数

    /// @brief 构造函数
    Node(const K k, const V v, int level)
        : key_(k), value_(v), node_level_(level),
          forward_(level, nullptr) {} 
    
    /// @brief 获取键
    K GetKey() const { 
        return key_; 
    }

    /// @brief 获取值
    V GetValue() const { 
        return value_; 
    } 

    /// @brief 设置值
    void SetValue(V value) { 
        value_ = value; 
    } 

    std::vector<std::shared_ptr<Node<K, V>>> forward_; 
    int node_level_; 

  private:
    K key_; 
    V value_; 
  };

  template <typename K, typename V>
  using NodeVec = std::vector<std::shared_ptr<node::Node<K, V>>>; // 定义一个包含Node智能指针的向量的别名
} //  namespace node


namespace skip_list
{
  std::string delimiter = ":";
  // define skip_list
  template <typename K, typename V>
  class SkipList
  {
  public:
    SkipList(int);
    ~SkipList();

    /// @brief 获取随机层级
    int GetRandomLevel() const;

    /// @brief 创建节点，创建一个节点，包含键、值和层级，并返回一个指向该节点的智能指针
    std::shared_ptr<node::Node<K, V>> CreateNode(K, V, int);

    ///@brief 插入元素，插入一个元素，如果元素已存在，则返回1，否则插入成功并返回0
    int InsertElement(K, V);

    ///@brief 打印列表，遍历每个层级的节点，控制台输出每个节点的键和值
    void PrintList() const;

    /// @brief 查找元素，从头节点开始，遍历每个层级的节点，直到找到key或者到达最后一个节点
    bool SearchElement(K) const;

    ///@brief 删除元素
    /// step 1 从头节点开始，遍历每个层级的节点，直到一个节点的键大于或等于目标键
    /// step 2 更新每个层级的forward指针，如果删除的是最高层的节点，需要降低跳表的层级
    void DeleteElement(K);

    ///@brief 将跳表的键值对写入文件，每行一个键值对，格式为“key:value”
    void DumpFile();

    /// @brief 从文件中加载键值对，从文件中读取每一行，解析出键和值，然后将键值对插入到跳表中
    void LoadFile();

    /// @brief 获取元素数量
    int Size() const { return element_count_; }

  private:

    /// @brief 从字符串中获取键值， 字符串形如“key:value”，解析后key为键，value为值
    void GetKeyValueFromString(const std::string &str, std::string &key, std::string &value) const;

    // @brief 判断字符串是否有效，如果字符串为空或者不包含分隔符，则返回false，否则返回true
    bool IsValidString(const std::string &str) const;

  private:
    int max_level_;                             // 最大层级
    int current_level_;                         // 当前层级
    std::shared_ptr<node::Node<K, V>> header_;  // 头节点
    std::ofstream file_writer_;                 // 文件写入
    std::ifstream file_reader_;                 // 文件读取
    int element_count_;                         // 元素数量
    std::mutex mutex_;                          // 互斥锁
  };

  /// @brief SkipList构造函数，初始化最大层级、当前层级和元素数量，创建一个头节点，键和值都为空，层级为max_level
  template <typename K, typename V>
  SkipList<K, V>::SkipList(int max_level) 
      : max_level_(max_level), current_level_(0), element_count_(0) { 
        header_ = std::make_shared<node::Node<K, V>>(K(), V(), max_level); 
      };

  /// @brief SkipList析构函数，关闭文件写入和读取，释放系统资源
  template <typename K, typename V>
  SkipList<K, V>::~SkipList()
  {
      if(file_writer_.is_open()) file_writer_.close();
      if(file_reader_.is_open()) file_reader_.close();

  }

  template <typename K, typename V>
  int SkipList<K, V>::GetRandomLevel() const
  {
    int level = 1;
    while(rand() % 2) {
      level++;
    }
    level =(level < max_level_) ? level : max_level_;
    return level;
  };

  template <typename K, typename V>
  std::shared_ptr<node::Node<K, V>> SkipList<K, V>::CreateNode(const K k, const V v, int level)
  {
    auto node = std::make_shared<node::Node<K, V>>(k, v, level);
    return node;
  }

  template <typename K, typename V>
  int SkipList<K, V>::InsertElement(const K key, const V value)
  {
    std::unique_lock<std::mutex> lck(mutex_); // 加锁
    auto current = header_; 
    auto update = node::NodeVec<K, V>(max_level_ + 1);// 创建一个用于更新节点的向量，存储每一层小于插入键的最后一个节点

    // 从当前层级开始，向下遍历到第0层
    for(int i = current_level_; i >= 0; i--)
    {
      // 在当前层级中向右移动，直到找到一个键大于或等于插入键的节点，或者到达当前层级的最右端
      while(current -> forward_[i] != nullptr && current -> forward_[i] -> GetKey() < key) {
        current = current -> forward_[i]; // 将当前节点移动到右边的节点
      }
      
      update[i] = current;// 在每个层级中，保存搜索路径上的最后一个节点
    }

    current = current -> forward_[0];// 插入位置

    // 如果当前节点的键等于插入的键，那么这个键已经存在，解锁然后返回
    if(current != nullptr && current -> GetKey() == key) {
      // std::cout << "key: " << key << ",exists" << std::endl;
      lck.unlock();
      return 1;
    }

    // 如果当前节点是空的，或者当前节点的键不等于插入的键，那么需要插入新节点
    if(current == nullptr || current -> GetKey() != key) {
      int random_level = GetRandomLevel(); // 生成一个随机的层级数

      // 如果随机生成的层级数大于当前的层级数，增加跳表的层级，并更新update
      if(random_level > current_level_) {
        for(int i = current_level_ + 1; i < random_level + 1; i++) {
          update[i] = header_;
        }
        current_level_ = random_level;
      }

      auto inserted_node = CreateNode(key, value, random_level); // 创建新节点

      // 插入新节点
      for(int i = 0; i < random_level; i++) {
        inserted_node -> forward_[i] = update[i] -> forward_[i];
        update[i] -> forward_[i] = inserted_node;
      }
      // std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
      element_count_++; // 跳表的元素数量增1
    }

    lck.unlock(); // 解锁
    return 0;
  }

  template <typename K, typename V>
  void SkipList<K, V>::DeleteElement(K key)
  {
    std::unique_lock<std::mutex> lck(mutex_); 
    auto current = header_; 
    auto update = node::NodeVec<K, V>(max_level_ + 1); 
    for(int i = current_level_; i >= 0; i--) {
      while(current -> forward_[i] != nullptr && current -> forward_[i] -> GetKey() < key) {
        current = current -> forward_[i]; 
      }

      update[i] = current; 
    }

    current = current -> forward_[0];
    if(current != nullptr && current -> GetKey() == key) {

      for(int i = 0; i <= current_level_; i++) {
        if(update[i] -> forward_[i] != current)
          break;
        update[i] -> forward_[i] = current -> forward_[i];
      }

      while(current_level_ > 0 && header_ -> forward_[current_level_] == 0) {
        current_level_--;
      }

      std::cout << "Successfully deleted key " << key << std::endl;
      element_count_--; 
    }

    lck.unlock();
    return;
  }


  template <typename K, typename V>
  bool SkipList<K, V>::SearchElement(K key) const
  {
    std::cout << "search_element-----------------" << std::endl;
    auto current = header_; 

    for(int i = current_level_; i >= 0; i--) {
      while(current -> forward_[i] != nullptr && current -> forward_[i] -> GetKey() < key) {
        current = current -> forward_[i]; 
      }
    }

    current = current -> forward_[0];
    
    if(current && current -> GetKey() == key) {
      std::cout << "Found key: " << key << ", value: " << current -> GetValue() << std::endl;
      return true;
    }
    std::cout << "Not Found Key:" << key << std::endl;
    return false;
  }



  template <typename K, typename V>
  void SkipList<K, V>::PrintList() const
  {
    std::cout << "\n*****Skip List*****" << "\n";
    for(int i = 0; i <= current_level_; i++) {
      auto node = header_ -> forward_[i];
      std::cout << "Level " << i << ": ";
      while(node != nullptr) {
        // std::cout << node -> GetKey() << ":" << node -> GetValue() << ";";
        node = node -> forward_[i];//指向下一个节点
      }
      std::cout << std::endl;
    }
  }

  template <typename K, typename V>
  void SkipList<K, V>::DumpFile()
  {
    // std::cout << "dump_file-----------------" << std::endl; 
    file_writer_.open(STORE_FILE); 

    auto node = header_ -> forward_[0];
    while(node != nullptr) {
      file_writer_ << node -> GetKey() << ":" << node -> GetValue() << "\n";
      // std::cout << node -> GetKey() << ":" << node -> GetValue() << ";\n";
      node = node -> forward_[0];
    }

    file_writer_.flush(); // 确保所有数据都已写入文件
    file_writer_.close(); // 关闭文件
    return;
  }
  
  template <typename K, typename V>
  void SkipList<K, V>::LoadFile()
  {
    file_reader_.open(STORE_FILE); 
    std::cout << "load_file-----------------" << std::endl; // 输出开始加载的消息

    std::string str; 
    std::string key; 
    std::string value; 

    while(getline(file_reader_, str)) {
      GetKeyValueFromString(str, key, value);// 从每一行中解析出键和值
      if(key.empty() || value.empty()) continue;
      InsertElement(key, value);// 将解析出的键和值插入到跳表中
      std::cout << "key:" << key << "value:" << value << std::endl;// 输出插入的键和值
    }

    file_reader_.close(); // 关闭文件
  }

  template <typename K, typename V>
  void SkipList<K, V>::GetKeyValueFromString(const std::string &str, std::string &key, std::string &value) const
  {
    if(!IsValidString(str)) return;
    key = str.substr(0, str.find(delimiter));
    value = str.substr(str.find(delimiter) + 1, str.length());
  }
  
  template <typename K, typename V>
  bool SkipList<K, V>::IsValidString(const std::string &str) const
  {
    if(str.empty() || str.find(delimiter) == std::string::npos) return false;
    return true;
  }
  
} 