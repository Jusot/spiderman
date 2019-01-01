#include "PUEF.hpp"


Response::Response() : status_code(400) {}

Response::Response(const ::std::string &data)
{
    ::std::stringstream ss(data);
    ss >> reason; ss >> status_code; ss.get();
    ::std::getline(ss, reason); reason.pop_back();

    auto header_endpos = data.find("\r\n\r\n") + (::std::size_t)4, html_beginpos = data.find("<html>");
    body = html_beginpos == data.npos ? data.substr(header_endpos) : data.substr(html_beginpos);

    ::std::string tmp;
    while (::std::getline(ss, tmp) && tmp != "\r")
    {
        tmp.pop_back();
        auto pos = tmp.find(':');
        if (pos != tmp.npos)
        {
            auto k = tmp.substr(0, pos), v = tmp.substr(pos + (::std::size_t)2);
            headers[k] += v;
        }
    }
}

Response& Response::operator=(Response &rhs)
{
    status_code = rhs.status_code;
    reason = rhs.reason;
    body = rhs.body;
    headers = rhs.headers;

    return *this;
}

Response& Response::operator=(Response &&rhs) noexcept
{
    ::std::swap(status_code, rhs.status_code);
    ::std::swap(reason, rhs.reason);
    ::std::swap(body, rhs.body);
    ::std::swap(headers, rhs.headers);

    return *this;
}


::std::string Request::gen_host(::std::string url)
{
    auto pos = url.find("://");
    if (pos != url.npos) url = url.substr(pos + (::std::size_t)3);

    pos = url.find("/");
    return pos == url.npos ? url : url.substr(0, pos);
}

::std::string Request::gen_req(::std::string url)
{
    auto pos = url.find("://");
    if (pos != url.npos) url = url.substr(pos + (::std::size_t)3);

    pos = url.find('/');
    return pos == url.npos ? "/" : url.substr(pos);
}

Response Request::request(const ::std::string &url, const ::std::string method, ::std::unordered_map<::std::string, ::std::string> &headers)
{
    ::std::string response_msg;


    // init server_addr
    addrinfo hints, *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    getaddrinfo(gen_host(url).c_str(), "80", &hints, &result);


    // connect to server
    auto socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    for (auto addr = result; addr != NULL; addr = addr->ai_next)
    {
        if (connect(socket_fd, addr->ai_addr, addr->ai_addrlen) == 0)
        {
            break;
        }
    }


    // send request message to server
    ::std::string request_msg = method + gen_req(url) + " HTTP/1.1\r\n";
    for (auto &kv : headers) request_msg += kv.first + ": " + kv.second + "\r\n";
    request_msg += "\r\n";

    send(socket_fd, request_msg.c_str(), request_msg.length(), 0);


    // receive response message from server
    char tmp[4096] = { 0 };

    while (recv(socket_fd, tmp, 4096, 0) > 0)
    {
        response_msg += tmp;
        memset(tmp, 0, sizeof(tmp));
    }

    close(socket_fd);
    freeaddrinfo(result);

    return Response(response_msg);
}

Response Request::get(::std::string url, ::std::unordered_map<::std::string, ::std::string> headers)
{
    decltype(headers) hds = {
            {"Host", gen_host(url)},
            {"Content-type", "text/html"},
            {"Connection", "Close"},
            {"Accept-Language", "zh-CN,zh,en-US"},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:63.0) Gecko/20100101 Firefox/63.0"},
    };
    for (auto &kv : headers) hds[kv.first] = kv.second;

    return request(url, "GET ", hds);
}
