#include "PUEF.hpp"


static std::string _conv2utf8(const std::string &source, const std::string &charset)
{
    if (charset.empty() || charset == "utf-8" || charset == "UTF-8") return source;

    std::string res;

    auto inlen = source.size() + 1;
    auto inbuf = (char *)malloc(inlen);
    inbuf[0] = 0;
    strcat(inbuf, source.c_str());

    auto outlen = inlen * 2;
    auto outbuf = (char *)malloc(outlen);

    auto inbuf_head = inbuf, outbuf_head = outbuf;

    // auto cd = iconv_open("utf-8", "gb2312");
    auto cd = iconv_open("utf-8", charset.c_str());
    iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
    iconv_close(cd);

    res = outbuf_head;

    free(inbuf_head);
    free(outbuf_head);

    return res;
}


static std::string _gen_host(std::string url)
{
    auto pos = url.find("://");
    if (pos != url.npos) url = url.substr(pos + 3);

    pos = url.find("/");
    return pos == url.npos ? url : url.substr(0, pos);
}

static std::string _gen_filepath(
    std::string dirpath,
    std::string date,
    std::string url,
    std::string title)
{
    // std::cout << date << std::endl << url << std::endl << title << std::endl;

    if (url.empty() || title.empty()) return "";

    if (dirpath.back() == '/') dirpath.pop_back();

    auto prepath = dirpath
        + '/' + _gen_host(url)
        + (date.empty() ? "" : '/' + date);

    system(("mkdir -p " + prepath).c_str());

    return prepath + '/' + title + ".txt";
}


bool Serialization::obj2file(
    WebSite website,
    const std::string &target_dirpath)
{
    auto &charset = website.metas["charset"];

    auto filepath = _gen_filepath(
        target_dirpath,
        website.metas["publishdate"],
        website.url,
        _conv2utf8(website.metas["title"], charset));

    if (filepath.empty()) return false;

#ifdef DEBUG
    std::cout << "[DEBUG] [SERIALIZATION] [FILENAME] [" << filename << "]" << std::endl;
#endif // DEBUG

    std::ofstream fout(filepath);

#ifdef LOGGING
    // std::cout << "[LOGGING] [SERIALIZATION] [STATUS " << fout.is_open() << "]" << std::endl;
#endif // LOGGING

    if (!fout.is_open()) return false;

    fout << _conv2utf8(website.url, charset) << std::endl;

    for (auto &meta : website.metas)
    {
        fout << _conv2utf8(meta.first, charset) << ':' << _conv2utf8(meta.second, charset) << '\n';
    }
    fout << std::endl;

    fout << _conv2utf8(website.text, charset) << std::endl;

    fout.close();

    return true;
}

WebSite Serialization::file2obj(
    const std::string &src_path)
{
    WebSite website;

    std::ifstream fin(src_path, std::ios::ate);

    if (!fin.is_open())
    {
        std::cout << "failed to open " << src_path << std::endl;
        exit(0);
    }

    auto end_pos = fin.tellg();
    fin.seekg(0);

    std::getline(fin, website.url);

    std::string tmp;
    while (std::getline(fin, tmp) && !tmp.empty())
    {
        website.metas[tmp.substr(0, tmp.find(':'))] = tmp.substr(tmp.find(':') + 1);
    }

    auto size = end_pos - fin.tellg();
    website.text = std::string(size, '\0');
    fin.read(&website.text[0], size);

    fin.close();

    return website;
}
