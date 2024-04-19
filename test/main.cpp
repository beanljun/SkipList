#include <iostream>
#include "../include/skip_list.h"
#define FILE_PATH "../config/dumpFile"

//全功能测试
int main() {
  skip_list::SkipList<int, std::string> skip_list(6);
  // key使用int类型，如果使用其他类型，需要定义比较函数
  // 如果想修改key的类型，需要修改LoadFile函数
  skip_list.InsertElement(1, "Shall I compare thee to a summer's day?"); 
  skip_list.InsertElement(3, "Thou art more lovely and more temperate:"); 
  skip_list.InsertElement(7, "Rough winds do shake the darling buds of May,"); 
  skip_list.InsertElement(8, "And summer's lease hath all too short a date:"); 
  skip_list.InsertElement(9, "Sometime too hot the eye of heaven shines,"); 
  skip_list.InsertElement(19, "And often is his gold complexion dimmed;"); 
  skip_list.InsertElement(19, "And every fair from fair sometime declines,"); 

  std::cout << "skip_list size:" << skip_list.Size() << std::endl;

  skip_list.DumpFile();

  //skip_list.LoadFile();

  skip_list.SearchElement(9);
  skip_list.SearchElement(18);

  skip_list.PrintList();

  skip_list.DeleteElement(3);
  skip_list.DeleteElement(7);

  std::cout << "skip_list size:" << skip_list.Size() << std::endl;

  skip_list.PrintList();
}
