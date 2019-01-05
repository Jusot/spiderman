#pragma once


class WebSite
{
public:

    ::std::string url, text;
    ::std::unordered_map<::std::string, ::std::string> metas;


    WebSite(const ::std::string &url, 
        ::std::unordered_map<::std::string, ::std::string> &&metas, 
        ::std::string &&text);

    WebSite(WebSite &&ws) noexcept = default;
    ~WebSite() = default;


    WebSite& operator=(WebSite &rhs);

    WebSite& operator=(WebSite &&rhs) noexcept;
};


class Parser
{
public:

    static WebSite parser(const ::std::pair<::std::string, ::std::string> &result);
};
