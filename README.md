## Prerequisites
 - [`redis-plus-plus`](https://github.com/sewenew/redis-plus-plus) serves as the client for *redis*.

## compile command
```
g++ {{sourceFile}} -o {{targetFile}} -Wall -std=c++17 -O3 -lredis++ -lhiredis -pthread -D VALIDATE
```

## Structure of Database in *Redis*
 - `UpdatedToBlockNumber`: [string] The last synced pool/pair state is exactly after the block with `blockNumber`.
 - `v3poolsData`: hash table for binary data of each uniswap v3 pool.
    - `address`: [string] binary data for bigint uniswap v3 pool with `address`.
 - `v3poolsInfo`: hash table for infomation of each uniswap v3 pool.
    - `address`: [string] the combination of `'{idx} {token0} {token1}'`, which are the the hashIdx of latest handled event, its token0 and token1 respectly.
 - `v2pairsData`: hash table.
    - `address`: [string] `{idx} {token0} {token1} {reserve0} {reserve1}`

## Other Comments
 - `HashIdx`: refer to the combination of `blockNumber` and `logIndex` ( `LogIndex` may start with serval leading zero to make the decimal length is exactly $5$ ).
 - All address in this project is passed the process of *checksum*.
 - You should always run the `main.cpp`(you may choose to use script `run.sh` directly) firstly. And then run the other model.