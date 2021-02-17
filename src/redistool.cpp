#include "redistool.h"
#include<sstream>
RedisTool::RedisTool()
{
    this->m_redis = NULL;
}
RedisTool::~RedisTool()
{
    if(this->m_redis)
    {
        redisFree(m_redis);//析构函数释放资源
        cout << "~RedisTool :: free redis connection " << endl;
    }
}

bool RedisTool::init()
{
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds 设置连接等待时间
    char ip[255];
    strcpy(ip, m_config.getRedisIP().c_str());
    cout << "init : ip = " << ip << endl;
    m_redis = redisConnectWithTimeout(ip, m_config.getRedisPort(), timeout);//建立连接
    if (m_redis->err) {
       printf("RedisTool : Connection error: %s\n", m_redis->errstr);
       return false;
    }
    else
    {
       cout << "init redis tool success " << endl;
       //REDIS_REPLY响应的类型type
       cout << "#define REDIS_REPLY_STRING 1"<< endl;
       cout << "#define REDIS_REPLY_ARRAY 2"<< endl;
       cout << "#define REDIS_REPLY_INTEGER 3"<< endl;
       cout << "#define REDIS_REPLY_NIL 4"<< endl;
       cout << "#define REDIS_REPLY_STATUS 5"<< endl;
       cout << "#define REDIS_REPLY_ERROR 6"<< endl;
    }
    return true;
}

// 设置string 类型的键值对
int RedisTool::setString(string key, string value)
{
    if(m_redis == NULL || m_redis->err)//int err; /* Error flags, 错误标识，0表示无错误 */
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return -1;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis,"SET %s %s", key.c_str(), value.c_str());//执行写入命令
    cout<<"set string type = "<<reply->type<<endl;//获取响应的枚举类型
    int result = 0;
    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        result = -1;
        cout << "set string fail : reply->str = NULL " << endl;
        //pthread_spin_unlock(&m_redis_flock);
        return -1;
    }
    else if(strcmp(reply->str, "OK") == 0)//根据不同的响应类型进行判断获取成功与否
    {
        result = 1;
    }
    else
    {
        result = -1;
        cout << "set string fail :" << reply->str << endl;
    }
    freeReplyObject(reply);//释放响应信息

    return result;
}
// 获取string类型的值
string RedisTool::getString(string key)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return NULL;
    }
    redisReply *reply;
    reply = (redisReply *)redisCommand(m_redis,"GET %s", key.c_str());
    cout<<"get string type = "<<reply->type<<endl;

    if(reply == NULL)
    {
        redisFree(m_redis);
        m_redis = NULL;
        cout << "ERROR getString: reply = NULL!!!!!!!!!!!! maybe redis server is down" << endl;
        return NULL;
    }
    else if(reply->len <= 0)
    {
        freeReplyObject(reply);
        return NULL;
    }
    else
    {
        stringstream ss;
        ss << reply->str;
        freeReplyObject(reply);
        return ss.str();
    }

}
// 设置list类型的键值对,尾添加
int RedisTool::listRPush(string key,vector<string> value)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return -1;
    }

    redisReply *reply;

    int valueSize = value.size();
    int result = 0;
    for(int i=0; i<valueSize; i++)
    {
        reply = (redisReply*)redisCommand(m_redis,"RPUSH %s %s", key.c_str(),value.at(i).c_str());
        int old = reply->integer;
        if(reply == NULL)
        {// 回复失败
            redisFree(m_redis);
            m_redis = NULL;
            result = -1;
            cout << "set list fail : reply->str = NULL " << endl;
            //pthread_spin_unlock(&m_redis_flock);
            return -1;
        }
        else if(reply->integer == old++)
        {
            result = 1; // 成功回复
            cout<<"rpush list ok"<<endl;
            continue;
        }
        else
        {
            result = -1;
            cout << "set list fail ,reply->integer = " << reply->integer << endl;
            return -1;
        }
    }
    freeReplyObject(reply);

    return result;
}

int RedisTool::listLPush(string key,vector<string> value)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return -1;
    }

    redisReply *reply;

    int valueSize = value.size();
    int result = 0;
    for(int i=0; i<valueSize; i++)
    {
        reply = (redisReply*)redisCommand(m_redis,"LPUSH %s %s", key.c_str(),value.at(i).c_str());
        int old = reply->integer;
        if(reply == NULL)
        {// 回复失败
            redisFree(m_redis);
            m_redis = NULL;
            result = -1;
            cout << "set list fail : reply->str = NULL " << endl;
            //pthread_spin_unlock(&m_redis_flock);
            return -1;
        }
        else if(reply->integer == old++)
        {
            result = 1; // 成功回复
            cout<<"lpush list ok"<<endl;
            continue;
        }
        else
        {
            result = -1;
            cout << "set list fail ,reply->integer = " << reply->integer << endl;
            return -1;
        }
    }
    freeReplyObject(reply);
    return result;
}

// 获取list类型的值
vector<string> RedisTool::getList(string key)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return vector<string>{};//返回空的向量
    }
    redisReply *reply;
    // 首先拿到对应链表的长度
    reply = (redisReply*)redisCommand(m_redis,"LLEN %s", key.c_str());
    int valueSize = reply->integer;
    cout<<"List size is :"<<reply->integer<<endl;

    //在redis数据库中搜索，搜索的结果返回reply中
    reply = (redisReply*)redisCommand(m_redis,"LRANGE %s %d %d", key.c_str(),0,-1);
    cout<<"get list type = "<<reply->type<<endl;
    cout<<"get list size = "<<reply->elements<<endl;//对于数组类型可以用elements元素获取数组长度

    redisReply** replyVector = reply->element;//获取数组指针（迭代器？？？）
    vector<string> result;
    for(int i=0;i<valueSize;i++)
    {
        string temp =(*replyVector)->str;//遍历redisReply*数组,存入vector向量
        result.push_back(temp);
        replyVector++;  // 指针向后移动
    }

    cout<<"result size:"<<result.size()<<endl;
    return result;
}
// 向hash表中，添加对应的field与val
int RedisTool::hashPush(string key,vector<string> field,vector<string> val)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return -1;//返回空的向量
    }
    unsigned int fieldSize = field.size();
    // 若输入有问题，直接返回
    if(fieldSize == 0 || val.size() == 0 || fieldSize != val.size())
    {
        cout << "Redis hashPush error !!! please check input" << endl;
        return -1;
    }
    redisReply *reply; // 接受redis的回复
    int result;

    for(int i = 0;i<fieldSize;i++)
    {
        reply = (redisReply*)redisCommand(m_redis,"HMSET %s %s %s", key.c_str(),field.at(i).c_str(),val.at(i).c_str());
        int old = reply->integer;
        if(reply == NULL)
        {// 回复失败
            redisFree(m_redis);
            m_redis = NULL;
            result = -1;
            cout << "hash push fail : reply->str = NULL " << endl;
            return -1;
        }
        else if(reply->integer == old++)
        {
            result = 1; // 成功回复
            cout<<"hash push ok"<<endl;
            continue;
        }
        else
        {
            result = -1;
            cout << "hash push fail ,reply->integer = " << reply->integer << endl;
            return -1;
        }
    }
    freeReplyObject(reply);
    return result;
}

// 获取hash表中，field对应的元素
vector<string> RedisTool::getHash(string key,vector<string> field)
{
    if(m_redis == NULL || m_redis->err)
    {
        cout << "Redis init Error !!!" << endl;
        init();
        return vector<string>{};//返回空的向量
    }
    if(field.size() == 0)
        return vector<string>{};// 返回空

    // 首先判断对应的key值是否存在
    redisReply *replyBegin; // 定义返回值对象
    replyBegin = (redisReply *)redisCommand(m_redis,"HKEYS %s", key.c_str());
    if(replyBegin->elements <= 0)
        return vector<string>{};// 返回空

    vector<string> vecRes;
    for(int i = 0;i<field.size();i++)
    {
        redisReply *reply; // 定义返回值对象
        // 取得返回值
        reply = (redisReply *)redisCommand(m_redis,"HMGET %s %s", key.c_str(),field[i].c_str());
        if(reply == NULL)
        {// 回复失败
            redisFree(m_redis);
            m_redis = NULL;
            cout << "hash push fail : reply->str = NULL " << endl;
            return vector<string>{};
        }

        redisReply** replyVector = reply->element; // 得到返回元素的地址
        if((*replyVector)->len <= 0)
        {// 若长度小于等于0，说明没有取到元素
            vecRes.push_back("");
        }
        else
        {
            string temp =(*replyVector)->str;//遍历redisReply*数组,存入vector向量
            vecRes.push_back(temp);
        }
    }

    return vecRes;
}

