#pragma once


class WebSite
{
public:

    WebSite();
    WebSite(WebSite &&ws);

    WebSite& operator=(WebSite &&ws);

    ~WebSite() = default;
};


class Parser
{
public:

    static WebSite parser(const ::std::pair<::std::string, ::std::string> &result);
};
