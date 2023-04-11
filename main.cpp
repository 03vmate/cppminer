#include <iostream>
#include <boost/asio.hpp>
#include <boost/json/src.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "CurrentWorkUnit.h"

using boost::asio::ip::tcp;

void parse_initial_notify(const boost::json::value& json) {
    const boost::json::array& result = json.at("result").as_array();

    CurrentWorkUnit::extranonce2_size = result[2].as_int64();
    CurrentWorkUnit::extranonce1 = result[1].as_string().c_str();
    CurrentWorkUnit::subscription_id = result[0].as_array()[0].as_array()[1].as_string().c_str();
    std::cout << "Extranonce1: " << CurrentWorkUnit::extranonce1 << std::endl;
    std::cout << "Extranonce2_size: " << CurrentWorkUnit::extranonce2_size << std::endl;
    std::cout << "Subscription_id: " << CurrentWorkUnit::subscription_id << std::endl;
}

void parse_difficulty(const boost::json::value& json) {
    const boost::json::array& params = json.at("params").as_array();
    CurrentWorkUnit::difficulty = params[0].as_int64();
    std::cout << "Difficulty: " << CurrentWorkUnit::difficulty << std::endl;
}

void parse_work(const boost::json::value& json) {
    const boost::json::array& params = json.at("params").as_array();
    std::string job_id = params[0].as_string().c_str();
    std::string prevhash = params[1].as_string().c_str();
    std::string coinb1 = params[2].as_string().c_str();
    std::string coinb2 = params[3].as_string().c_str();
    boost::json::array merkle_branch = params[4].as_array();
    std::string version = params[5].as_string().c_str();
    std::string nbits = params[6].as_string().c_str();
    std::string ntime = params[7].as_string().c_str();
    bool clean_jobs = params[8].as_bool();

    std::cout << "Job_id: " << job_id << std::endl;
    std::cout << "Prevhash: " << prevhash << std::endl;
    std::cout << "Coinb1: " << coinb1 << std::endl;
    std::cout << "Coinb2: " << coinb2 << std::endl;
    std::cout << "Version: " << version << std::endl;
    std::cout << "Nbits: " << nbits << std::endl;
    std::cout << "Ntime: " << ntime << std::endl;
    std::cout << "Clean_jobs: " << clean_jobs << std::endl;
}

void parse_json(const std::string& response) {
    std::cout << "Parsing JSON: " << response << std::endl;
    try {
        boost::json::value json = boost::json::parse(response);
        //Response to mining.subscribe
        if (json.at("id") == 1) {
            parse_initial_notify(json);
        }
        //Response to mining.authorize
        else if(json.at("id") == 2) {
            if(json.at("result").as_bool()) {
                std::cout << "Authorized" << std::endl;
            }
            else {
                std::cout << "Not authorized, exiting" << std::endl;
                exit(1);
            }
        }
        //Diff change
        else if(json.at("method").as_string() == "mining.set_difficulty") {
            parse_difficulty(json);
        }
        //New work
        else if(json.at("method").as_string() == "mining.notify") {
            parse_work(json);
        }
        
    }
    catch (const std::exception& e) {
        std::cout << "Error parsing JSON: " << e.what() << std::endl;
    }

    std::cout << "\n\n" << std::endl;
}

int main() {
    //Connect to pool
    boost::asio::io_context io_context;
    tcp::socket socket(io_context);
    socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("51.81.56.15"), 3333));

    //Send mining.subscribe
    const std::string mining_subscribe = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": []}\n";
    boost::asio::write(socket, boost::asio::buffer(mining_subscribe));

    //Send mining.authorize
    const std::string mining_authorize = "{\"id\": 2, \"method\": \"mining.authorize\", \"params\": [\"1PKN98VN2z5gwSGZvGKS2bj8aADZBkyhkZ\", \"x\"]}\n";
    boost::asio::write(socket, boost::asio::buffer(mining_authorize));

    while(1) {
        std::string response;
        boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), '\n');
        boost::algorithm::trim(response);
        std::cout << "Received message:\n" << response << std::endl;

        //Split response lines
        std::vector<std::string> jsonlines;
        boost::split(jsonlines, response, boost::is_any_of("\n"), boost::token_compress_on);
        for(std::string jsonline : jsonlines) {
            parse_json(jsonline);
        }
    }

    return 0;
}