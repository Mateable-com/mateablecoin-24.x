#include <stdlib.h>
#include <time.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <ostream>
#include <sstream>

typedef std::pair<std::string, std::string> exchangeEntry;

bool fetch_price_data(std::string& outstr)
{
    try
    {
        const char* host = "5.78.43.251";
        const char* port = "80";

        boost::asio::io_context ioc;
        boost::beast::net::ip::tcp::resolver resolver(ioc);
        boost::beast::tcp_stream stream(ioc);

        auto const results = resolver.resolve(host, port);
        stream.connect(results);

        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/test", int(10)};
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, "mateable/1.0");

        boost::beast::http::write(stream, req);
        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::dynamic_body> res;
        boost::beast::http::read(stream, buffer, res);

        std::stringstream ss;
        ss << res;
        outstr = ss.str();

        boost::beast::error_code ec;
        stream.socket().shutdown(boost::beast::net::ip::tcp::socket::shutdown_both, ec);
    } 
    catch(std::exception const& e)
    {
        return false;
    }

    return true;
}

bool parse_price_data(std::vector<exchangeEntry>& exchangeList)
{
    std::string outstr;
    if (!fetch_price_data(outstr)) {
        return false;
    }

    bool parse_begin = false;
    std::istringstream instr(outstr);
    for (std::string line; std::getline(instr, line); ) {
        if (line.find("start") != std::string::npos) {
            parse_begin = true;
            continue;
        }
        if (parse_begin) {
            int delimpos = 0;
            for (int i=0; i<line.size(); i++) {
                 if (line[i] == 58) {
                     delimpos = i;
                     break;
                 }
            }
            if (delimpos != 0 && delimpos+1 < line.size()) {
                 exchangeEntry entry;
                 entry.first = line.substr(0, delimpos);
                 entry.second = line.substr(delimpos+1, line.size());
                 exchangeList.push_back(entry);
            }
        }
    }
    return true;
}

void return_random_exchange(std::string& exchangeData)
{
    std::vector<exchangeEntry> exchangeList;
    parse_price_data(exchangeList);
    exchangeData.clear();

    int totalExchanges = 0;
    for (const auto& l : exchangeList) {
        ++totalExchanges;
    }

    // we didnt find any data
    if (totalExchanges == 0) {
        return;
    }

    // choose a random one
    srand(time(NULL));
    int randExchange = rand() % totalExchanges + 1;

    int exchangeCount = 0;
    for (const auto& l : exchangeList) {
        ++exchangeCount;
        if (exchangeCount == randExchange) {
            exchangeData = l.first + "    " + l.second;
        }
    }
}
