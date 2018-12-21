#pragma once


class Response
{
public:

    int           status_code;
    ::std::string reason;
    ::std::string body;

    ::std::unordered_map<::std::string, ::std::string> headers;


    Response() : status_code(400) {}

    Response(::std::string &data);

    Response(Response &&res) noexcept = default;
    ~Response() = default;


    Response& operator=(Response &rhs);

    Response& operator=(Response &&rhs) noexcept;
};


class Request
{
private:

    static ::std::string gen_host(::std::string url);

    static ::std::string gen_req(::std::string url);

    static Response request(const ::std::string &url, const ::std::string method, ::std::unordered_map<::std::string, ::std::string> &headers);

public:

    static Response get(::std::string url, ::std::unordered_map<::std::string, ::std::string> headers);
        
};