 #include"index.hpp"
 #include"util.hpp"
 #include<algorithm>
 #include<jsoncpp/json/json.h>

 namespace ns_searcher{//建立索引的本质是把磁盘上去标签化的文档，以索引的形式正排倒排的形式加载到内存中
    class Searcher{//将index建立成单例模式，让searcher直接获取单例就可以
        private:
            ns_index::Index *index;//供系统进行查找的索引
        public:
            Searcher(){}
            ~Searcher(){}
        public:
            void InitSearcher(const std::string &input)
            {
                //1.获取或者建立index对象   //使用的是单例
                index = ns_index::Index::GetInstance();
                std::cout<<"获取index单例成功..."<<std::endl;
                //2.根据index对象建立索引
                index->BuildIndex(input);
                std::cout<<"建立正排和倒排索引成功..."<<std::endl;
            }

            //query:搜索关键字
            //json_string:返回给用户浏览器的搜索结果
            void Search(const std::string &query,std::string *json_string)
            {
                //对于搜索关键字在服务器上也要进行分词，然后才能进行查找index

                //1.【分词】：对我们搜索词query按照searcher的要求进行分词
                std::vector<std::string> words;
                ns_util::JiebaUtil::CutString(query,&words);

                //2.【触发】：就是根据分词的各个词，进行index查找,建立index是忽略大小写，所以搜索关键字也要忽略大小写
                ns_index::InvertedList inverted_list_all;//所有搜索分词的倒排拉链
                for(std::string word:words){
                    boost::to_lower(word);

                    ns_index::InvertedList *inverted_list = index->GetInvertedList(word);
                    if(nullptr == inverted_list){
                        continue;
                    }
                    inverted_list_all.insert(inverted_list_all.end(),inverted_list->begin(),inverted_list->end());
                }

                //3.【合并排序】：汇总查找结果，按照相关性weight降序排序
                std::sort(inverted_list_all.begin(),inverted_list_all.end(),\
                        [](const ns_index::InvertedElem &e1,const ns_index::InvertedElem &e2){
                            return e1.weight>e2.weight;
                        });

                //4.【构建】：根据查找出来的结果，构建json串--jsoncpp--通过jsoncpp完成序列化和反序列化
                Json::Value root;
                for(auto &item : inverted_list_all){
                    ns_index::DocInfo * doc = index->GetForwardIndex(item.doc_id);
                    if(nullptr == doc){
                        continue;
                    }
                    Json::Value elem;
                    elem["title"] = doc->title;
                    elem["desc"] = GetDesc(doc->content,item.word);//内容描述摘要
                    elem["url"] = doc->url;

                    //for debug 便于查看关键词搜索出的相关文档倒排权重，就是看看文档的相关顺序是否正确
                    elem["id"] = (int)item.doc_id;
                    elem["weight"] = item.weight;

                    root.append(elem); 
                }
                Json::StyledWriter writer;
                *json_string = writer.write(root);

            }

            std::string GetDesc(const std::string &html_content,const std::string &word)
            {
                //找到word在html—content中首次出现，然后往前找50字节（如果没有，从begin开始）,往后找100字节（如果没有，到end就可以），截取出这部分内容
                const int prev_step = 50;
                const int next_step = 100;

                //1.1找到首次出现,直接使用find进行查找内容和关键词大小不匹配
                /*std::size_t pos = html_content.find(word);
                if(pos == std::string::npos)
                {
                    return "None";
                }*/

                //1.2解决方案
                auto iter = std::search(html_content.begin(),html_content.end(),word.begin(),word.end(),[](int x,int y){
                    return (std::tolower(x) == std::tolower(y));
                    });
                if(iter == html_content.end()){
                    return "None";
                }
                int pos = std::distance(html_content.begin(),iter);

                //2.获取start，end
                int start = 0;
                int end = html_content.size() - 1;
                if(pos > start + prev_step )start = pos - prev_step;
                if(pos < end - next_step)end = pos + next_step;
                //3.截取子串
                if(start>=end) return "None";
                std::string desc = html_content.substr(start,end - start);
                desc+="...";
                return desc;
            }
    };
 }