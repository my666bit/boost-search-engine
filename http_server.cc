#include"searcher.hpp"
#include"httplib.h"

const std::string input = "data/raw_html/raw.txt";
const std::string root_path = "./wwwroot";

int main()
{
    ns_searcher::Searcher search;
    search.InitSearcher(input);

     //eg:百度搜索引擎https://www.baidu.com/s?tn=22073068_5_oem_dg&ie=utf-8&wd=%E4%BD%A0%E5%A5%BD
    httplib::Server svr;
    svr.set_base_dir(root_path.c_str());
    svr.Get("/s",[&search](const httplib::Request &req,httplib::Response &rsp){
        if(!req.has_param("word")){
            rsp.set_content("必须要有搜索关键字！","text/plain: charset=utf-8");
            return;
        }
        std::string word = req.get_param_value("word");
        std::cout<<"用户在搜索："<<word<<std::endl;
        std::string json_string;
        search.Search(word,&json_string);
        rsp.set_content(json_string,"application/json");
    });


    //test:
    // svr.Get("/hi",[](const httplib::Request &req,httplib::Response &rsp){
    //     rsp.set_content("hello world!","text/plain: charset=utf-8");
    //     });
    svr.listen("192.168.247.128",8888);
    return 0;
}