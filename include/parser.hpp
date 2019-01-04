#pragma once


class WebSite
{
public:

    WebSite();
    WebSite(WebSite &&ws);

    WebSite& operator=(WebSite &&ws);

    ~WebSite();
};


class Parser
{
public:

    static WebSite parser(const ::std::string &source);
};
