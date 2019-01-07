#include "PUEF.hpp"


Spider::Spider(const ::std::string url, const ::std::string url_limit) : finish_status(false), host(Request::gen_host(url)), url_limit(url_limit)
{
    if (url_limit == "") this->url_limit = url;
    unprocessed.push(url);
}

bool Spider::has_processed(const ::std::string &url)
{
    return processed.count(url);
}

bool Spider::is_legal_and_complete(::std::string &url)
{
    if (url.find('/') == 0)
    {
        url = host + url;
        return true;
    }
    else if (url.find("http") != url.npos || url.find(".") != url.npos) return true;
    else return false;
}

bool Spider::meet_limit(const ::std::string &url)
{
    return url.find(url_limit) != url.npos;
}

::std::vector<::std::string> Spider::gen_urls(const ::std::string &str)
{
    ::std::vector<::std::string> urls;

    auto it = str.cbegin();

    auto safe_check = [&](char c) -> bool
    {
        return (it == str.cend()) ? false : (*it++ == c);
    };

    auto checkpre = [&](::std::string s) -> bool
    {
        for (auto c : s) if (!safe_check(c)) return false;
        return true;
    };

    while (it != str.cend())
    {
        if (checkpre("href=\""))
        {
            ::std::string url;
            while (it != str.cend() && *it != '"') url += *it++;

            if (is_legal_and_complete(url)) urls.push_back(url);
        }
    }

    return urls;
}

bool Spider::finish()
{
    return finish_status;
}

void Spider::run(threadsafe_queue<::std::pair<::std::string, ::std::string>> &results)
{
    while (!unprocessed.empty())
    {
        auto url = unprocessed.front();
        unprocessed.pop();

        if (has_processed(url)) continue;
        else processed.insert(url);

#ifdef LOGGING
        ::std::cout << "[LOGGING] FIND NEW URL: " << url << ::std::endl;
#endif // LOGGING

        auto response_body = Request::get(url).body;

        for (auto new_url : gen_urls(response_body))
        {
            if (meet_limit(new_url) && !has_processed(new_url))
            {
                unprocessed.push(new_url);
            }
        }

        results.push({ url, std::move(response_body) });
    }

    finish_status = true;
}