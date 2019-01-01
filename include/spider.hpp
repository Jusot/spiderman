#pragma once


class Spider
{
private:

    bool finish_status;
    ::std::string url_limit;
    ::std::set<::std::string> processed;
    ::std::queue<::std::string> unprocessed;


    bool has_processed(const ::std::string &url);

    bool meet_limit(const ::std::string &url);

    ::std::vector<::std::string> gen_urls(const ::std::string &str);

public:

    Spider(const ::std::string target, const ::std::string url_limit = "");

    
    bool finish();

    void run(threadsafe_queue<::std::string> &results);
};