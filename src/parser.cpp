#include "PUEF.hpp"


static const long   LengthOfInterval = 100;
static const double Alpha            = 0.6;


WebSite::WebSite(
    const std::string &url,
    std::string &&text,
    std::unordered_map<std::string, std::string> &&metas
) : url(url), text(std::move(text)), metas(std::move(metas))
{
    // it's completed
}

WebSite& WebSite::operator=(WebSite &rhs)
{
    url = rhs.url;
    metas = rhs.metas;
    text = rhs.text;

    return *this;
}

WebSite& WebSite::operator=(WebSite &&rhs) noexcept
{
    std::swap(url, rhs.url);
    std::swap(metas, rhs.metas);
    std::swap(text, rhs.text);

    return *this;
}


// --- static functions for Parser::parser ---

static bool _isspace(const char c)
{
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

/*
 * O(n)
 * parse metas in header
 * store indexs and the corresponding character of an index consists of a label
 * filter comments between '<!--' and '-->'
 * filter js code between '<script' and '/script>'
 * filter css code between '<style' and '/style>'
 */
static std::unordered_map<std::string, std::string>
_parse_metas_and_mark_tags_and_filter(
    const std::string &source,
    std::vector<std::pair<size_t, bool>> &indexs)
{
    std::unordered_map<std::string, std::string> metas;

    size_t i = 0;

    auto safe_check = [&](size_t &ind, char c) -> bool
    {
        return (ind >= source.length())
               ? false
               : (source[ind++] == c);
    };

    auto checkpre = [&](const std::string &s) -> bool
    {
        auto tmp = i;
        for (auto c : s) if (!safe_check(tmp, c)) return false;
        i = tmp;
        return true;
    };

#ifdef DEBUG
    std::cout << "[DEBUG] [PARSING] [METAS]" << std::endl;
#endif // DEBUG

    while (i < source.length())
    {
        if (checkpre("<body") || checkpre("<BODY")) break;
        else if (checkpre("&nbsp;"));
        else if (checkpre("<meta ") || checkpre("<META "))
        {
            std::string name, content;
            while (source[i] != '>')
            {
                if (checkpre("name=\"")) while (source[i] != '"')
                    name += source[i++];
                else if (checkpre("content=\"")) while (source[i] != '"')
                    content += source[i++];
                else ++i;
            }
            if (!name.empty() && !content.empty())
                metas[name] = content;
        }
        else if (checkpre("<title>") || checkpre("<TITLE>"))
        {
            std::string content;
            while (i < source.length() && !checkpre("</title>") && !checkpre("</TITLE>")) content += source[i++];
            metas["title"] = content;
        }
        else ++i;
    }

#ifdef DEBUG
    std::cout << "[DEBUG] [PARSING] [FILTER]" << std::endl;
#endif // DEBUG

    while (i < source.length())
    {
        if (checkpre("<!--"))
            while (i < source.length() && !checkpre("-->")) ++i;
        else if (checkpre("<style") || checkpre("<STYLE"))
            while (i < source.length() && !checkpre("/style>") && !checkpre("/STYLE>")) ++i;
        else if (checkpre("<script") || checkpre("<SCRIPT"))
            while (i < source.length() && !checkpre("/script>") && !checkpre("/SCRIPT>")) ++i;
        // fix bug when meeting '><'
        else if (source[i] == '<')
        {
            indexs.push_back({ i++, 0 });
            while (i < source.length() && source[i - 1] != '>') indexs.push_back({ i++, 0 });
        }
        else if (checkpre("&nbsp;"));
        else if (_isspace(source[i]))
        {
            indexs.push_back({ i++, 1 });
            while (i < source.length() && _isspace(source[i])) ++i;
        }
        else indexs.push_back({ i++, 1 });
    }

    // filtering spaces in metas
    for (auto &meta : metas)
    {
        auto &content = meta.second;
        content.erase(std::remove_if(content.begin(),
            content.end(),
            _isspace),
            content.end());
    }

    return metas;
}

/*
 * O(n)
 * divide the indexs into n intervals
 * calculate the longest continuous interval, [start, end], each text percent >= Alpha
 * return mid index
 */
static size_t _calculate_mid(
    const std::vector<std::pair<size_t, bool>> &indexs)
{
    std::vector<double> alphas;
    size_t start = 0, end = 0;

    for (size_t _start = 0, _end = 0, interval_i = 0;
         _end < indexs.size() / LengthOfInterval;
         ++_end, interval_i += LengthOfInterval)
    {
        double s = 0;
        for (auto i = interval_i;
             i < std::min(interval_i + LengthOfInterval, indexs.size());
             ++i)
        {
            s += indexs[i].second;
        }
        alphas.push_back(s / LengthOfInterval);

        if (alphas[_end] >= Alpha)
        {
            if (_end - _start > end - start)
            {
                start = _start + 1;
                end = _end;
            }
        }
        else _start = _end;
    }

    return (start + end + 1) * LengthOfInterval / 2;
}

/*
 * O(n)
 * calculate the possible start position x of the text
 * return x
 */
static size_t _calculate_x(
    size_t mid,
    const std::vector<std::pair<size_t, bool>> &indexs)
{
    size_t x = 0, _x = 0, ltags = 0, rtxts = 0, max_sum;
    for (size_t i = _x; i < mid; ++i) rtxts += indexs[i].second;

    max_sum = rtxts;

    while (++_x < mid)
    {
        ltags += 1 - indexs[_x].second;
        rtxts -= indexs[_x].second;

        auto sum = ltags + rtxts;
        if (sum > max_sum)
        {
            max_sum = sum;
            x = _x;
        }
    }

    return x;
}

/*
 * O(n)
 * calculate the possible end position y of the text
 * return y
 */
static size_t _calculate_y(
    size_t mid,
    const std::vector<std::pair<size_t, bool>> &indexs)
{
    size_t y = indexs.size() - 1, _y = indexs.size() - 1, ltxts = 0, rtags = 0, max_sum;
    for (size_t i = _y; i > mid; --i) ltxts += indexs[i].second;

    max_sum = ltxts;

    while (--_y > mid)
    {
        ltxts -= indexs[_y].second;
        rtags += 1 - indexs[_y].second;

        auto sum = ltxts + rtags;
        if (sum > max_sum)
        {
            max_sum = sum;
            y = _y;
        }
    }

    return y;
}


WebSite Parser::parser(const std::pair<std::string, std::string> &result)
{
    auto &url = result.first, &source = result.second;

    // 0 means it's tag, 1 means it's txt
    std::vector<std::pair<size_t, bool>> indexs;

#ifdef DEBUG
    std::cout << "[DEBUG] [PARSING] [METAS & FILETER]" << std::endl;
#endif // DEBUG

    auto metas = _parse_metas_and_mark_tags_and_filter(source, indexs);

    if (indexs.empty()) return WebSite();

#ifdef DEBUG
    std::cout << "[DEBUG] [PARSING] [CALCULATE MID X Y]" << std::endl;
#endif // DEBUG

    auto mid = _calculate_mid(indexs);
    auto x   = _calculate_x(mid, indexs),
         y   = _calculate_y(mid, indexs);

#ifdef DEBUG
    std::cout << "[DEBUG] [PARSING] [CALCULATE MID X Y] [OVER]" << std::endl;
#endif // DEBUG

    // generate final text from source string
    std::string text;
    for (auto i = x; i <= y; ++i) if (indexs[i].second) text += source[indexs[i].first];

    return WebSite(url, std::move(text), std::move(metas));
}
