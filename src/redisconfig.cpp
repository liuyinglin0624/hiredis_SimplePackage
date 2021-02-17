#include "redisconfig.h"

RedisConfig::RedisConfig()
{

}

std::string RedisConfig::getRedisIP()//获取ip
{
    return "127.0.0.1";
}

int RedisConfig::getRedisPort()//获取端口号
{
    return 6379;
}
