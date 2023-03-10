#include "include/v3pool.h"
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

    for (auto doc : cursor) {
        auto rawData = std::string{doc["handledData"].get_string().value};
        std::istringstream istr(rawData);
        result.push_back(v3::rawdata2event(istr));
    }
    // eventsColl.delete_many(document{} << "blockNumber" << BlockNumber << finalize);
    return std::make_pair(true, result);
}

int main(){
    initializeTicksPrice();
    mongocxx::instance instance{};
    mongocxx::uri uri("mongodb://10.71.99.125:27017/");
    mongocxx::client client(uri);
    mongocxx::database db = client["symbc"];
    eventsColl = db["queue"];

    int toHandleBlock = 12369738;
    while("💤Shu💝Yu💖Mo💤") {
        auto [exist, events] = fetchEvents(toHandleBlock);
        if(!exist){
            std::cout << "[W]   The `block " << toHandleBlock << "` have not been created yet." << std::endl;
            usleep(500 * 1000); // 500 ms.
            continue;
        }

        if(events.size())
            std::cout << "[S]   Fetched " << events.size() << " events from `block " << toHandleBlock << "`." << std::endl;

        for(auto e : events){
            if(e.type == v3::CRET){
                assert(!v3Pool.count(e.address));
                v3Pool[e.address] = new v3::V3Pool(e.fee, e.tickspace, e.liquidity);
                std::cout << "[S]   New v3 pool " << e.address << " created and been listened." << std::endl;
            } else {
                assert(v3Pool.count(e.address));
                v3Pool[e.address]->processEvent(e);
            }
            v3Pool[e.address]->save("./pool_state/" + e.address + ".ip");
        }
        toHandleBlock++;
        // FindCycle();
    }
    // error on 12376424
    return 0;
}