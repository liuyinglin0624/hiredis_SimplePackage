#ifndef REDISTOOL_H
#define REDISTOOL_H
#include <iostream>
#include <vector>
#include <hiredis/hiredis.h>
#include"redisconfig.h"
#include<string.h>
using namespace std;

class RedisTool
{
public:
    RedisTool();
    ~RedisTool();
    bool init();
    // string 类型
    int setString(string key, string value);
    string getString(string key);
    // list类型
    int listRPush(string key,vector<string> value);
    int listLPush(string key,vector<string> value);
    vector<string> getList(string key);
    // hash类型
    int hashPush(string key,vector<string> field,vector<string> val);
    vector<string> getHash(string key,vector<string> field);
    // set类型

    // hset类型

private:
    redisContext *m_redis;  //
    RedisConfig m_config;  // 用于获取Redis的IP与端口
};

#endif // REDISTOOL_H
