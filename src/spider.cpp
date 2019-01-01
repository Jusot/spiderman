#include "PUEF.hpp"


Spider::Spider(const ::std::string url, const ::std::string url_limit) : finish_status(false), url_limit(url_limit)
{
    if (url_limit == "") this->url_limit = url;
    unprocessed.push(url);
}

bool Spider::has_processed(const ::std::string &url)
{
    return processed.count(url);
}

bool Spider::meet_limit(const ::std::string &url)
{
    return url.find(url_limit) != url.npos;
}

// need to complete
::std::vector<::std::string> Spider::gen_urls(const::std::string & str)
{
    ::std::vector<::std::string> urls;

    // TODO: parser input content and generate new urls in this content

    return urls;
}

bool Spider::finish()
{
    return finish_status;
}

void Spider::run(threadsafe_queue<::std::string> &results)
{
    while (!unprocessed.empty())
    {
        auto url = unprocessed.front();
        unprocessed.pop();

        processed.insert(url);

        auto response_body = Request::get(url).body;
        results.push(response_body);

        for (auto new_url : gen_urls(response_body))
        {
            if (meet_limit(new_url) && !has_processed(new_url))
            {
                unprocessed.push(new_url);
            }
        }
    }

    finish_status = true;
}