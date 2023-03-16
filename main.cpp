#include "include/graph.h"
#include <map>
#include <cstring>
#include <ctime>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;



std::map<std::string, v3::V3Pool*> v3Pool;

mongocxx::collection eventsColl;

/* 尝试获取 `BlockNumber` 上的 events. 若这一个块还没出现，返回 false. */
std::pair<bool, std::vector<v3::V3Event>> fetchEvents(int BlockNumber) {
    std::vector<v3::V3Event> result;
    auto cnt = eventsColl.count_documents(document{} << "blockNumber" << open_document << "$gte" << BlockNumber << close_document << finalize);

    if(cnt <= 0) {
        return std::make_pair(false, result);
    }

    mongocxx::cursor cursor = eventsColl.find(document{} << "blockNumber" << BlockNumber << finalize);

    std::vector<std::pair<int, std::string>> temp;

    for (auto doc : cursor) {
        auto rawData = std::string{doc["handledData"].get_string().value};
        int index = doc["_id"]["logIndex"].get_int32().value;
        temp.push_back(std::make_pair(index, rawData));
    }

    sort(temp.begin(), temp.end());

    for(auto [logindex, rawevent] : temp) {
        std::istringstream istr(rawevent);
        result.push_back(v3::rawdata2event(istr));
#ifdef DEBUG
        FILE * fptr = fopen(("./log/" + (result.end() - 1)->address + ".txt").c_str(), "a+");
        fprintf(fptr, "%s", rawevent.c_str());
        fclose(fptr);
#endif
    }

   eventsColl.delete_many(document{} << "blockNumber" << BlockNumber << finalize);

    return std::make_pair(true, result);
}

int main(){
    initializeTicksPrice();
    mongocxx::instance instance{};
    mongocxx::uri uri("mongodb://10.71.99.125:27017/");
    mongocxx::client client(uri);
    mongocxx::database db = client["symbc"];
    eventsColl = db["queue"];
    std::ofstream fout("circle_founded_info.log");

    int toHandleBlock = 12369738;
    // int tt = 0;
    while("💤Shu💝Yu💖Mo💤") {
        auto [exist, events] = fetchEvents(toHandleBlock);
        if(!exist){
            static int waitedtime = 0;
            std::cout << "[W]   The `block " << toHandleBlock << "` have not been created yet. waitedtime = " << ++waitedtime << std::endl;
            usleep(500 * 1000); // 500 ms.
            continue;
        }

        // tt += events.size();
        // if(events.size())
        //     if(tt > 600) break; else std::cerr << "TT  =  " << tt << std::endl;

        if(events.size())
            std::cout << "[S]   Fetched " << events.size() << " events from `block " << toHandleBlock << "`." << std::endl;

        for(auto e : events){
            if(e.type == v3::CRET){
                assert(!v3Pool.count(e.address));
                v3Pool[e.address] = new v3::V3Pool(e.fee, e.tickspace, e.liquidity);
                graph::addV3Pool(e.token0, e.token1, v3Pool[e.address]);
                std::cout << "[S]   New v3 pool " << e.address << " created and been listened. the number of recognised token = " << graph::token_num << " the number of pools = " << graph::v3_pool_num  << "." << std::endl;
            } else {
                assert(v3Pool.count(e.address));
                v3Pool[e.address]->processEvent(e);
            }
            v3Pool[e.address]->save("./pool_state/" + e.address + ".ip");
        }
        if(events.size()) {
            auto [found, circle] = graph::FindCircle();
            if(found) {
                fout << "After block: " << toHandleBlock << std::endl;
                fout << circle << std::endl;
            }
        }
        toHandleBlock++;
    }
    return 0;
}