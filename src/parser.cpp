#include "PUEF.hpp"


static const long   LengthOfInterval = 100;
static const double Alpha            = 0.6;


WebSite::WebSite(
    const ::std::string &url,
    ::std::unordered_map<::std::string, ::std::string> &&metas,
    ::std::string &&text
) : url(url), metas(::std::move(metas)), text(::std::move(text))
{
    // it's completed
}

WebSite& WebSite::operator=(WebSite &rhs)
{
    url = rhs.url;
    metas = rhs.metas;
    text = rhs.text;
}

WebSite& WebSite::operator=(WebSite &&rhs) noexcept
{
    ::std::swap(url, rhs.url);
    ::std::swap(metas, rhs.metas);
    ::std::swap(text, rhs.text);
}


// --- static functions for Parser::parser ---

/*
 * 一次遍历
 * 记录所有索引以及是否为标签
 * 过滤掉注释<!--.*?-->
 * 以及<script>.*?</script>
 * 和<style>.*?</style>部分
 */
static ::std::unordered_map<::std::string, ::std::string>
_parse_metas_and_mark_tags_and_filter(
    const ::std::string &source, 
    ::std::vector<::std::pair<size_t, bool>> &indexs)
{
    ::std::unordered_map<::std::string, ::std::string> metas;

    size_t i = 0;

    auto safe_check = [&](size_t &ind, char c) -> bool
    {
        return (ind == source.length())
               ? false
               : (source[ind++] == c);
    };

    auto checkpre = [&](const ::std::string &s) -> bool
    {
        auto tmp = i;
        for (auto c : s) if (!safe_check(tmp, c)) return false;
        i = tmp;
        return true;
    };
    
    while (!checkpre("<body"))
    {
        if (checkpre("<meta"))
        {
            ::std::string name, content;
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
        else if (checkpre("<title>"))
        {
            ::std::string content;
            while (source[i] != '<') content += source[i++];
            metas["title"] = content;
        }
        else ++i;
    }

    while (i != source.length())
    {
        if      (checkpre("/*"))      while (!checkpre("*/")) ++i;
        else if (checkpre("<!--"))    while (!checkpre("-->")) ++i;
        else if (checkpre("<style"))  while (!checkpre("/style>")) ++i;
        else if (checkpre("<script")) while (!checkpre("/script>")) ++i;
        else if (source[i] == '<')    while (i != source.length() && source[i - 1] != '>') indexs.push_back({ i++, 0 });
        else    indexs.push_back({ i++, 1 });
    }

    return metas;
}

/*
 * 一次遍历
 * 将indexs分为n个区间
 * 计算非标签占比>=Alpha的最长连续区间[start, end]
 */
static size_t _calculate_mid(
    const ::std::vector<::std::pair<size_t, bool>> &indexs)
{
    ::std::vector<double> alphas;
    size_t start = 0, end = 0;

    for (size_t _start = 0, _end = 0, interval_i = 0;
         _end < indexs.size() / LengthOfInterval;
         ++_end, interval_i += LengthOfInterval)
    {
        double s = 0;
        for (auto i = interval_i; i < ::std::min(interval_i + LengthOfInterval, indexs.size()); ++i)
        {
            s += indexs[i].second;
        }
        alphas[_end] = s / LengthOfInterval;

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
 * 一次遍历
 * 计算正文可能开始位置x
 */
static size_t _calculate_x(
    size_t mid,
    const ::std::vector<::std::pair<size_t, bool>> &indexs)
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
 * 一次遍历
 * 计算正文可能结束位置y
 */
static size_t _calculate_y(
    size_t mid, 
    const ::std::vector<::std::pair<size_t, bool>> &indexs)
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


WebSite Parser::parser(const ::std::pair<::std::string, ::std::string> &result)
{
    auto &url = result.first, &source = result.second;

    // 0 means it's tag, 1 means it's txt
    ::std::vector<::std::pair<size_t, bool>> indexs;

    auto metas = _parse_metas_and_mark_tags_and_filter(source, indexs);

    auto mid = _calculate_mid(indexs);
    auto x   = _calculate_x(mid, indexs),
         y   = _calculate_y(mid, indexs);

    // generate final text from source string
    ::std::string text;
    for (auto i = x; i <= y; ++i) if (indexs[i].second) text += source[indexs[i].first];

    return WebSite(url, ::std::move(metas), ::std::move(text));
}
