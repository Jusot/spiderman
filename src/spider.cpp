#include "PUEF.hpp"


Spider::Spider(const std::string url, const std::string url_limit) : finish_status(false), host(Request::gen_host(url)), url_limit(url_limit)
{
    if (url_limit == "") this->url_limit = url;
    unprocessed.push(url);
}

bool Spider::has_processed(const std::string &url)
{
    return processed.count(url);
}

bool Spider::is_legal_and_complete(std::string &url, const std::string &base)
{
    if (url.find(".css") != url.npos || url.find(".js") != url.npos
       || url.find("..") != url.npos || url.find(';') != url.npos
       || url.find('\n') != url.npos || url.find("#") != url.npos
       || url.find(':')  != url.npos) return false;
    else if (url.find("http") != url.npos)
    {
        url = url.substr(url.find("://") + 3);
        return true;
    }
    else if (url.find(host) != url.npos) return true;

    if (url[0] == '/') url = host + url;
    else
    {
        auto end_pos = base.rfind('/');
        url = end_pos == base.npos ? base + '/' + url : base.substr(0, end_pos + 1) + url;
    }
    return true;
}

bool Spider::meet_limit(const std::string &url)
{
    return url.find(url_limit) != url.npos;
}

std::vector<std::string> Spider::gen_urls(const std::string &str, const std::string &base)
{
    std::vector<std::string> urls;

    auto it = str.cbegin();

    auto safe_check = [&](decltype(str.cbegin()) &tmp_it, char c) -> bool
    {
        return (tmp_it >= str.cend())
               ? false
               : (*tmp_it++ == c);
    };

    auto checkpre = [&](const std::string &s) -> bool
    {
        auto tmp = it;
        for (auto c : s) if (!safe_check(tmp, c)) return false;
        it = tmp;
        return true;
    };

    while (it < str.cend())
    {
        if (checkpre("<!--")) while (it < str.cend() && !checkpre("-->")) ++it;
        else if (checkpre("href=\""))
        {
            std::string url;
            while (it < str.cend() && *it != '"') url += *it++;

            if (is_legal_and_complete(url, base)) urls.push_back(url);
        }
        else ++it;
    }

    return urls;
}

const bool Spider::finish()
{
    return finish_status;
}

void Spider::run(std::queue<std::pair<std::string, std::string>> &results)
{
    while (!unprocessed.empty())
    {
        auto url = unprocessed.front();
        unprocessed.pop();

        if (url.back() == '/') url.pop_back();

        if (has_processed(url)) continue;
        else processed.insert(url);

        auto response = Request::get(url);

#ifdef LOGGING
        std::cout << "[LOGGING] [URL] [STATUS " << response.status_code << "] " << url << std::endl;
#endif // LOGGING

#ifdef DEBUG
        std::cout << "[LOGGING] [URL] [STATUS " << response.status_code << "] " << url << std::endl;
#endif // DEBUG

        if (response.status_code >= 300) continue;

        auto response_body = response.body;

        results.push({ url, response_body });

        for (auto new_url : gen_urls(response_body, url))
        {
            if (meet_limit(new_url) && !has_processed(new_url))
            {
                if (new_url.back() == '/') new_url.pop_back();
                unprocessed.push(new_url);
            }
        }
    }

    finish_status = true;
    std::cout << "finish..." << std::endl;
}
