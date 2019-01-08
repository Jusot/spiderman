#include "PUEF.hpp"


static std::string _gen_host(std::string url)
{
    auto pos = url.find("://");
    if (pos != url.npos) url = url.substr(pos + 3);

    pos = url.find("/");
    return pos == url.npos ? url : url.substr(0, pos);
}

static std::string _gen_filename(
    std::string date,
    std::string url,
    std::string title)
{
    // std::cout << date << std::endl << url << std::endl << title << std::endl;

    if (date.empty() || url.empty() || title.empty()) return "";
    return '/' + date 
         + '-' + _gen_host(url) 
         + '-' + title + ".txt";
}


bool Serialization::obj2file(
    WebSite website,
    const std::string &target_dirpath)
{
    auto filename = _gen_filename(
        website.metas["publishdate"], 
        website.url, 
        website.metas["title"]);

    if (filename.empty()) return false;

    std::ofstream fout(target_dirpath + filename);

    if (!fout.is_open()) return false;

    fout << website.url << std::endl;
    
    for (auto &meta : website.metas)
    {
        fout << meta.first << ':' << meta.second << '\n';
    }
    fout << std::endl;

    fout << website.text << std::endl;

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
