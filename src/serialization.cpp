#include "PUEF.hpp"


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
    auto filepath = _gen_filepath(
        target_dirpath,
        website.metas["publishdate"],
        website.url,
        website.metas["title"]);

    if (filepath.empty()) return false;

#ifdef DEBUG
    std::cout << "[DEBUG] [SERIALIZATION] [FILENAME] [" << filename << "]" << std::endl;
#endif // DEBUG

    std::ofstream fout(filepath);

#ifdef LOGGING
    // std::cout << "[LOGGING] [SERIALIZATION] [STATUS " << fout.is_open() << "]" << std::endl;
#endif // LOGGING

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
