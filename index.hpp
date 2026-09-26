#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<fstream>
#include"util.hpp"
#include<mutex>

namespace ns_index{
    struct DocInfo{
        std::string title;//文档标题
        std::string content;//文档去标签内容
        std::string url;//官网文档url
        uint64_t doc_id;//文档ID
    };

    struct InvertedElem{//倒排索引所对应元素
        uint64_t doc_id;
        std::string word;
        int weight;     
    };

    //倒排拉链
    typedef std::vector<InvertedElem> InvertedList;//一个关键字可能对应多个文档

    class Index{
        private:
            //正派索引的数据结构用数组，数组下标天然是文档ID
            std::vector<DocInfo> forward_index;
            //倒排索引一定是一个关键字和一组（个）InvertedElem对应
            std::unordered_map<std::string,InvertedList> inverted_index;

        private://index单例模式的构建
            Index(){}
            Index(const Index&) = delete;
            Index& operator = (const Index&) = delete;

            static Index* instance;
            static std::mutex mtx;
        
        public:
            ~Index(){}

        public:
            //获取单例的函数
            static Index* GetInstance()
            {
                if(nullptr == instance){
                    mtx.lock();
                    if(nullptr == instance){
                        instance = new Index();
                    }
                    mtx.unlock();
                }
                return instance;
            }

            //根据doc_id找到文档内容
            DocInfo *GetForwardIndex(uint64_t doc_id)
            {
                if(doc_id>=forward_index.size()){
                    std::cerr<<"doc_id out range,error!"<<std::endl;
                    return nullptr;
                }
                return &forward_index[doc_id];
            }

            //根据关键字string，获得倒排拉链
            InvertedList *GetInvertedList(const std::string &word)
            {
                auto iter = inverted_index.find(word);
                if(iter == inverted_index.end()){
                    std::cerr<<word<<" have no InvertedList"<<std::endl;
                    return nullptr;
                }
                return &(iter->second);
            }

            //根据去标签，格式化后的文档，构建正排倒排索引
            //data/raw_html/raw.txt
            bool BuildIndex(const std::string &input)//parse处理完毕的数据
            {
                std::ifstream in(input,std::ios::in | std::ios::binary);
                if(!in.is_open()){
                    std::cerr<<"sorry, "<<input<<" open error"<<std::endl;
                    return false;
                }

                std::string line;
                int count = 0;
                while(std::getline(in,line)){
                    DocInfo *doc = BuildForwardIndex(line);
                    if(nullptr == doc){
                        std::cerr<<"build "<<line<<" error"<<std::endl;
                        continue;
                    }
                    BuildInvertedIndex(*doc);
                    count++;
                    if(count%50 == 0){
                        std::cout<<"当前已经建立的索引文档："<<count<<std::endl;
                    }
                }
                return true;
            }
        private:
            DocInfo *BuildForwardIndex(const std::string &line)
            {
                //1.解析line,字符串切分
                std::vector<std::string> results;
                const std::string sep = "\3";
                ns_util::StringUtil::Split(line,&results,sep);
                if(results.size()!=3){
                    return nullptr;
                }

                //2.字符串进行填充到DocInfo
                DocInfo doc;
                doc.title = results[0];
                doc.content = results[1];
                doc.url = results[2];
                doc.doc_id = forward_index.size();//先进性保存id，在插入，对应的id就是当前doc在vector中的下标
                //3.插入到正排索引的vector
                forward_index.push_back(std::move(doc));
                return &forward_index.back();
            }

            bool BuildInvertedIndex(const DocInfo &doc)
            {
                 //word -> 倒排拉链

                 struct word_cnt{
                    int title_cnt;
                    int content_cnt;

                    word_cnt():title_cnt(0),content_cnt(0){}
                 };

                 std::unordered_map<std::string,word_cnt> word_map;//用来暂存词频的映射表

                 //对标题进行分词
                 std::vector<std::string> title_words;
                 ns_util::JiebaUtil::CutString(doc.title,&title_words);
                //对标题进行词频统计
                 for(auto s:title_words){
                    boost::to_lower(s);//将我们的分词进行统一转换成小写的
                    word_map[s].title_cnt++;//如果存在就获取，如果不存在就新建
                 }

                 //对文档内容进行分词
                 std::vector<std::string> content_words;
                 ns_util::JiebaUtil::CutString(doc.content,&content_words);
                 //对内容进行词频统计
                 for(auto s:content_words){
                    boost::to_lower(s);//将我们的分词进行统一转换成小写的
                    word_map[s].content_cnt++;
                 }

#define X 10
#define Y 1
                for(auto &word_pair:word_map)
                {
                    InvertedElem item;
                    item.doc_id = doc.doc_id;
                    item.word = word_pair.first;
                    item.weight = X*word_pair.second.title_cnt + Y*word_pair.second.content_cnt;
                    // 写法A，课程代码现在这样
                    InvertedList &inverted_list = inverted_index[word_pair.first];
                    inverted_list.push_back(std::move(item));

                    // //写法B，等价简写，日常可以直接这么写
                    // inverted_index[word_pair.first].push_back(item);
                    /*如果 key 不存在，`map[key]`会自动插入这个 key，并且构造一个空的 value（这里就是空 vector InvertedList），
                    返回这个 value 的引用；如果 key 已经存在，直接返回已有 vector 引用。*/
                }
                return true;
            }
    };
    Index* Index::instance = nullptr;
    std::mutex Index::mtx;
}
