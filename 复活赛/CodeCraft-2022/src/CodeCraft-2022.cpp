#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include<algorithm>
#include <queue>
#include <time.h>
#include <unordered_set>
#include <math.h>
#include <unordered_map>
#include <float.h>

using namespace std;

//#define LOCALPATH

struct BandwithStream;
struct Client;
struct Site;

// 全局数据区域
/*****************************************************/
int qos_upper = 0;
int base_cost = 0;
vector<int> gStreamNumbers;
vector<vector<BandwithStream*>> gAllStreams;
vector<vector<vector<bool>>> gstreamIsLinkSite;
int RunTimes = 0 ;
int RunTimes90 = 0;
int loopNumbers = 0;
std::vector<Site> mvSite;
vector<Site*> mvSitePtr;

/*****************************************************/
typedef struct cmpRealPer5{
    bool operator()(pair<int, int>& a, pair<int, int>& b){
        return a.second > b.second;
    }
};


// 结构体成员区域
/*****************************************************/
// 结构体成员区域
/*****************************************************/
typedef struct BandwithStream{
    Client* mCli;
    Site* mSit;
    bool mIsAllocate;
    int mdemand;
    string mID;
//    int mtime;
//    int mLinkSitNumbers;
}BandwithStream;

typedef struct Site{
    unsigned long long mAlldemand;
    double mCost; //成本
    vector<unsigned long long> mvSumStream;
    vector<Client*> mvLinkClient;     // 与本边缘节点相连的客户节点ID（时延满足要求即视为相连）
    priority_queue<pair<int,int>, vector<pair<int,int>>, cmpRealPer5 > mRealPer5;
    int mbandwidth; //本边缘节点带宽上限
    int mCostBand;      // 不影响成本的前提下，未分配时刻的带宽上限
    bool IsSmooth = false;
    vector<int> mvAllocatedBand;      // 记录该边缘节点每个时刻已分配带宽
    vector<bool> mIsPer5Time;
    string mID;    //边缘节点ID
    bool mIsPer90 = false;


//    int mRunTimes;                  //记录5%函数中有偷跑带宽的时刻数(没啥用)


    // vector<vector<BandwithStream*>> mvLinkStream;

    // int 保存的是 time, sit->mvAllocatedBand[time]

//    unordered_set<Site*> msLinkSite;
}Site;

typedef struct Client{
    string mID;
    vector<vector<BandwithStream>> mvdemand;
    vector<unordered_map<Site*,vector<BandwithStream*>>> mvsolution;
    vector<unsigned long long> mvSumStream;
    vector<Site*> mvLinkSite;  //与本客户节点相连的边缘节点（时延满足要求即视为相连）
    unordered_set<Site*> msLinkSite;

    // 时序分配方案
    // vector控制时间t，unordered_map 控制分配给的边缘点，好找一点。
}Client;



typedef struct PathtInf{
    string configini;
    string demandcsv;
    string site_bandwidthcsv;
    string qoscsv;
    string solutiontxt;
    string SiteSolutiontxt;
} PathtInf;
/*****************************************************/

// 读入和写出文件区域
/*****************************************************/
//读取config文件，并将键-值保存于Content中
bool ReadConfig(const std::string& filename,
                std::unordered_map<std::string, int> &Content);
//读取边缘节点及其带宽上限
bool ReadSitedData(const std::string& filename,
                   std::vector<Site>& vSite,
                   std::unordered_map<std::string, int>& hash_Site_name_idx);
//读取数据demand.csv，客户节点ID保存于 vClient， 时间名称保存于vTime
bool ReadData(const std::string& filename,
              std::vector<std::string>& vTime,
              std::vector<Client>& vClient,
              std::unordered_map<std::string, int>& hash_Client_name_idx);
//读取QoS文件，并保存于 data ，其中列表示客户节点，行表示边缘节点，data[i][j]表示 vSiteID[i]、vClientID[j] 的网络时延
bool ReadQos(const std::string& filename,
             std::vector<Client>& vClient,
             std::vector<Site>& vSite,
             std::unordered_map<std::string, int>& hash_Client_name_idx,
             std::unordered_map<std::string, int>& hash_Site_name_idx);

//bool WriteSolution(const std::string& filename,
//                   std::vector<std::string>& vTime,
//                   std::vector<Client>& vClient);
bool WriteSolution(const std::string& filename,
                   std::vector<std::string>& vTime,
                   std::vector<Site>& vSite,
                   std::vector<Client>& vClient);

bool WriteSiteSolution(const std::string& filename,
                   std::vector<Site>& vSite);
/*****************************************************/

// 解决方案区域
/*****************************************************/
bool InitialData(vector<string>& mvTime,vector<Site>& mvSite,vector<Client>& mvClient, PathtInf& path);
void InitMember(vector<string>& mvTime,vector<Site>& mvSite,vector<Client>& mvClient);
void processPer5(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient);
void processPer95(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient);
bool processPer95_building(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient, double value);
void Smooth(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient);
void Smooth1(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient);
inline void allocateStreamToSite(int time, Site* sit, Client* cli,
                                BandwithStream* stream, bool IsPer5);
void calculateCost(int times, vector<Site*>& mvSitePtr);
void newSmooth(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient);
void findSiteStream( int time ,vector<BandwithStream*>& vec);
void Smoothtop(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient);
/*****************************************************/


// 比较函数代码区域
/*****************************************************/
// 对全局gAllStreams进行排序，按流片大的进行排序
//bool cmpBandwithStream_new( BandwithStream* a, BandwithStream* b){
//    return (a->mLinkSitNumbers < b->mLinkSitNumbers)||
//            (a->mLinkSitNumbers == b->mLinkSitNumbers && a->mdemand > b->mdemand);
//}

bool cmpBandwithStream( BandwithStream* a, BandwithStream* b){
    return a->mdemand > b->mdemand;
}

bool cmpBandwithStreamCli( BandwithStream a, BandwithStream b){
    return a.mdemand > b.mdemand;
}

bool cmpBandwiteSite(Site* a, Site* b){
    return a->mbandwidth > b->mbandwidth;
}

bool cmpClient(Client* a, Client* b){
    return a->mvLinkSite.size() < b->mvLinkSite.size();
}

//小顶堆，分别保存pair<时刻，该时刻该边缘节点的需求>，用来存放 5%
typedef struct cmpSitePri{
    bool operator()(pair<int, unsigned long long>& a, pair<int, unsigned long long>& b){
        return a.second > b.second;
    }
};

//小顶堆，分别保存pair<时刻，该时刻该边缘节点的需求>，用来存放 5%
typedef struct cmpPreSitePri{
    bool operator()(pair<int, int>& a, pair<int, int>& b){
        return a.second > b.second;
    }
};

bool cmpSitedemand(Site* a, Site* b){
    return a->mAlldemand > b->mAlldemand;
}
bool cmpstreamsmooth(BandwithStream* a, BandwithStream* b){
    Site* sit_a = a->mSit;
    Site* sit_b = b->mSit;
    int idx_a = 0, idx_b = 0;
    for(int i = 0; i < mvSitePtr.size(); ++i){
        if(mvSitePtr[i] == sit_a) idx_a = i;
        if(mvSitePtr[i] == sit_b) idx_b = i;
    }

    return idx_a > idx_b;
 }
/*****************************************************/


int main() {
    clock_t start, finish;
    start = clock();
    PathtInf path;
#ifdef LOCALPATH
    path.configini = "./data/config.ini";
    path.demandcsv = "./data/demand.csv";
    path.site_bandwidthcsv = "./data/site_bandwidth.csv";
    path.qoscsv = "./data/qos.csv";
    path.solutiontxt = "./output/solution.txt";
    path.SiteSolutiontxt = "./output/SiteSolution.csv";
#else
    path.configini = "/data/config.ini";
    path.demandcsv = "/data/demand.csv";
    path.site_bandwidthcsv = "/data/site_bandwidth.csv";
    path.qoscsv = "/data/qos.csv";
    path.solutiontxt = "/output/solution.txt";
    path.SiteSolutiontxt = "/output/SiteSolution.csv";
#endif



    vector<Client> mvClient;
    vector<string> mvTime;

    bool ret = InitialData(mvTime, mvSite, mvClient, path);

    clock_t finish_init = clock();
    std::cout << "初始化总耗时： " << (double)(finish_init - start) / CLOCKS_PER_SEC << std::endl;

    RunTimes = floor(0.05*mvTime.size());
    RunTimes90 = floor(0.1*mvTime.size());


    for(int s = 0; s < mvSite.size(); ++s){
        mvSitePtr.push_back(&mvSite[s]);
    }
    sort(mvSitePtr.begin(), mvSitePtr.end(), cmpSitedemand);    //按整个时序总需求进行排序，从大到小

    for(int i = 0; i < 10; ++i){
        mvSitePtr[i]->mIsPer90 = true;
    }

    processPer5(mvTime, mvSitePtr, mvClient);
#ifdef LOCALPATH
    clock_t finish_Per5 = clock();
    std::cout << "5% 方案运行时间： " << (double)(finish_Per5 - finish_init) / CLOCKS_PER_SEC << std::endl;
#endif
//    WriteSiteSolution(path.SiteSolutiontxt, mvSite);

    double value = 1.25;
    ret = false;
    ret = processPer95_building(mvTime, mvSitePtr, mvClient, value);
//
//    if(ret == false){
//        value = 1.5;
//        reverse(mvSitePtr.begin(), mvSitePtr.end());
//        ret = processPer95_building(mvTime, mvSitePtr, mvClient, value);
//        reverse(mvSitePtr.begin(), mvSitePtr.end());
//    }
//    if(ret == false){
//        value = 1.35;
//        ret = processPer95_building(mvTime, mvSitePtr, mvClient, value);
//    }
    calculateCost(mvTime.size(), mvSitePtr);
   // cout << "ret: " << ret << endl;
    if(ret == false) processPer95(mvTime, mvSitePtr, mvClient);

    clock_t finish_Per95 = clock();
//    std::cout << "95 方案运行时间： " << (double)(finish_Per95 - finish_building) / CLOCKS_PER_SEC << std::endl;
    //  newSmooth(mvTime, mvSitePtr, mvClient);
    Smoothtop(mvTime, mvSitePtr, mvClient);
    clock_t finish_smooth = clock();
    std::cout << "smooth 方案运行时间： " << (double)(finish_smooth - finish_Per95) / CLOCKS_PER_SEC << std::endl;

    WriteSolution(path.solutiontxt, mvTime,mvSite, mvClient);
    clock_t finish_write = clock();
    std::cout << "输出 方案运行时间： " << (double)(finish_write - finish_smooth) / CLOCKS_PER_SEC << std::endl;
#ifdef LOCALPATH
    cout <<"loopNumbers: " << loopNumbers << endl;
    finish = clock();
    std::cout << "总运行时间： " << (double)(finish - start) / CLOCKS_PER_SEC << std::endl;
//    WriteSiteSolution(path.SiteSolutiontxt, mvSite);

    finish = clock();
    std::cout << "运行时间： " << (double)(finish - start) / CLOCKS_PER_SEC << std::endl;
#endif
    return 0;
}


bool ReadConfig(const std::string& filename,
                std::unordered_map<std::string, int> &Content) {
    std::ifstream infile(filename.c_str());
    bool ret = true;
    if (!infile) {
        std::cout << "config file open error!" << std::endl;
        ret = false;
        return ret;
    }

    string line;

    getline(infile, line);  //读掉第一行

    while (getline(infile, line))
    {
        string key;
        int value;

        int pos = line.find("=");
        key = line.substr(0, pos);

        int endpos = line.find("\r");
        value = stoi(line.substr(pos + 1, endpos));
        Content[key] = value;
    }

    infile.close();
    return ret;
}

bool ReadSitedData(const std::string& filename,
                   std::vector<Site>& vSite,
                   std::unordered_map<std::string, int>& hash_Site_name_idx){
    std::ifstream infile(filename.c_str());
    bool ret = true;
    if (!infile)
    {
        std::cout << "site_bandwidth file open error!" << std::endl;
        ret = false;
        return ret;
    }
    std::string line;
    int pos;
    getline(infile, line);  //读掉第一行

    while(getline(infile, line)){
        Site cur;
        //读取数据并保存在data中
        pos = line.find(",");
        cur.mID = line.substr(0, pos);

        line = line.substr(pos + 1);
        pos = line.find("\r"); //定位换行符
        cur.mbandwidth = stoi(line.substr(0, pos));
        vSite.push_back(cur);
    }

    //将 ID 和对应的 mvSite 下标记录成哈希表
    for(int s = 0; s < vSite.size(); ++s){
        hash_Site_name_idx[vSite[s].mID] = s;
    }
    infile.close();
    return ret;
}

//读取数据demand.csv，客户节点ID保存于 vClient， 时间名称保存于vTime
bool ReadData(const std::string& filename,
              std::vector<std::string>& vTime, std::vector<Client>& vClient,
              std::unordered_map<std::string, int>& hash_Client_name_idx){
    std::ifstream infile(filename.c_str());
    bool ret = true;
    if (!infile)
    {
        std::cout << "data file open error!" << std::endl;
        ret = false;
        return ret;
    }

    std::string line;
    int pos;

    getline(infile, line);
    pos = line.find("stream_id,");
    line = line.substr(pos + 10);    //去掉mtime,stream_id,
    //读取ClientID
    pos = line.find(",");
    while(pos > -1){
        Client cur;
        cur.mID = line.substr(0, pos);
        vClient.push_back(cur);

        line = line.substr(pos + 1);
        pos = line.find(",");
    }
    pos = line.find("\r"); //定位换行符

    Client end;
    end.mID = line.substr(0, pos);
    vClient.push_back(end);

    string Time_ID = "";
    int t = -1;
    //读取需求
    while(getline(infile, line)){
        //读取时间
        pos = line.find(",");
        string curLine_Time_ID = line.substr(0, pos);
        if(curLine_Time_ID != Time_ID){
            ++t;
            Time_ID = curLine_Time_ID;
            vTime.push_back(Time_ID);
            for(int c = 0; c < vClient.size(); ++c){
                vClient[c].mvdemand.push_back(vector<BandwithStream>());
            }
        }
        line = line.substr(pos + 1);

        pos = line.find(",");
        string Stream_ID = line.substr(0, pos); //读取当前行的stream_ID;
        line = line.substr(pos + 1);

        BandwithStream cur_stream;
        cur_stream.mID = Stream_ID;
        cur_stream.mIsAllocate = false;
//        cur_stream.mtime = t;

        int client_idx = 0;
        pos = line.find(",");
        while(pos > -1){
            cur_stream.mdemand = stoi(line.substr(0, pos));
            if(cur_stream.mdemand > 0){
                cur_stream.mCli = (&vClient[client_idx]);
                vClient[client_idx].mvdemand[t].push_back(cur_stream);
            }
            ++client_idx;

            line = line.substr(pos + 1);
            pos = line.find(",");
        }
        pos = line.find("\r"); //定位换行符

        cur_stream.mdemand = stoi(line.substr(0, pos));
        if(cur_stream.mdemand > 0){
            cur_stream.mCli = (&vClient[client_idx]);
            vClient[client_idx].mvdemand[t].push_back(cur_stream);
        }
    }

    //将 ID 和对应的 vClient 下标记录成哈希表
    for(int c = 0; c < vClient.size(); ++c){
        hash_Client_name_idx[vClient[c].mID] = c;
        for(int t = 0; t < vTime.size(); ++t){
            sort(vClient[c].mvdemand[t].begin(), vClient[c].mvdemand[t].end(), cmpBandwithStreamCli);
        }
    }

    infile.close();
    return ret;
}

bool ReadQos(const std::string& filename,
             std::vector<Client>& vClient,
             std::vector<Site>& vSite,
             std::unordered_map<std::string, int>& hash_Client_name_idx,
             std::unordered_map<std::string, int>& hash_Site_name_idx){
    bool ret = true;
    std::ifstream infile(filename.c_str());
    if (!infile)
    {
        std::cout << "QoS file open error!" << std::endl;
        ret = false;
        return ret;
    }

    std::string line;
    int pos;
    int site_idx = 0;
    vector<string> ClientID;    //存储每一列对应的客户节点 ID
    vector<string> SiteID;    //存储每一行对应的边缘节点 ID
    getline(infile, line);  //  读取第一行，即客户节点名称
    pos = line.find(",");
    line = line.substr(pos + 1);    //跳过第一个，即跳过 "site_name"
    int client_idx = 0;
    pos = line.find(",");
    while(pos > -1){
        string ID_c = line.substr(0, pos);
        ClientID.push_back(ID_c);
        line = line.substr(pos + 1);
        pos = line.find(",");
    }
    pos = line.find("\r"); //定位换行符
    string end_ID_c = line.substr(0, pos);
    ClientID.push_back(end_ID_c);

    //读取边缘节点 ID 及 qos
    while(getline(infile, line)){
        //读取数据并保存在data中
        pos = line.find(",");
        SiteID.push_back(line.substr(0, pos));  //插入每一行边缘节点的 ID
        line = line.substr(pos + 1);    //跳过第一个
        int client_idx = 0;
        pos = line.find(",");
        while(pos > -1){
            int cur_qos = stoi(line.substr(0, pos));
            if(cur_qos < qos_upper){
                vClient[hash_Client_name_idx[ClientID[client_idx]]].mvLinkSite.push_back(&vSite[hash_Site_name_idx[SiteID[site_idx]]]);
                vSite[hash_Site_name_idx[SiteID[site_idx]]].mvLinkClient.push_back(&vClient[hash_Client_name_idx[ClientID[client_idx]]]);
            }
            ++client_idx;
            line = line.substr(pos + 1);
            pos = line.find(",");
        }
        pos = line.find("\r"); //定位换行符
        int end_qos = stoi(line.substr(0, pos));
        if(end_qos < qos_upper){
            vClient[hash_Client_name_idx[ClientID[client_idx]]].mvLinkSite.push_back(&vSite[hash_Site_name_idx[SiteID[site_idx]]]);
            vSite[hash_Site_name_idx[SiteID[site_idx]]].mvLinkClient.push_back(&vClient[hash_Client_name_idx[ClientID[client_idx]]]);
        }
        ++site_idx;
    }
    infile.close();
    return ret;
}

bool WriteSolution1(const std::string& filename,
                   std::vector<std::string>& vTime,
                   std::vector<Client>& vClient){
    std::ofstream solution(filename, std::ios::out);
    int ret = true;
    if(!solution.is_open()){
        std::cout << "solution.txt flie open error" << std::endl;
        ret = false;
        return ret;
    }

    for(int t = 0; t < vTime.size(); ++t){
        for(int c = 0; c < vClient.size(); ++c){
            solution << vClient[c].mID <<":";
            bool begin_Client = true;

            unordered_map<Site*,vector<BandwithStream*>>::iterator it = vClient[c].mvsolution[t].begin();
            for(;it != vClient[c].mvsolution[t].end();++it){
                if(it->second.size() == 0)  //对于该边缘节点没有流分配
                    continue;

                if(!begin_Client){
                    solution <<",";
                }
                solution << "<" << it->first->mID;  //边缘节点ID

                for(int cur_stream = 0; cur_stream < (it->second.size()); ++cur_stream){
                    solution << ",";
                    solution << (it->second)[cur_stream]->mID;  //流ID
                }

                solution << ">";
                begin_Client = false;
            }

            if(t == vTime.size() - 1 && c == vClient.size() - 1) continue;  //去掉最后一行换行
            solution << std::endl;
        }
    }

    solution.close();
    return ret;
}
bool WriteSolution(const std::string& filename,
                   std::vector<std::string>& vTime,
                   std::vector<Site>& vSite,
                   std::vector<Client>& vClient){
    std::ofstream solution(filename, std::ios::out);
    int ret = true;
    if(!solution.is_open()){
#ifdef DEBUG
        std::cout << "solution.txt flie open error" << std::endl;
#endif
        ret = false;
        return ret;
    }

    bool begin_90 = true;
    //开头一行
    for(int s = 0; s < vSite.size(); ++s){
        Site* sit = &vSite[s];
        if(sit->mIsPer90) {
            if(!begin_90){
                solution << ",";
            }
            solution << sit->mID;
            begin_90 = false;
        }
    }
    solution << endl;
    for(int t = 0; t < vTime.size(); ++t){
        for(int c = 0; c < vClient.size(); ++c){
            solution << vClient[c].mID <<":";
            bool begin_Client = true;

            unordered_map<Site*,vector<BandwithStream*>>::iterator it = vClient[c].mvsolution[t].begin();
            for(;it != vClient[c].mvsolution[t].end();++it){
                if(it->second.size() == 0)  //对于该边缘节点没有流分配
                    continue;

                if(!begin_Client){
                    solution <<",";
                }
                solution << "<" << it->first->mID;  //边缘节点ID

                for(int cur_stream = 0; cur_stream < (it->second.size()); ++cur_stream){
                    solution << ",";
                    solution << (it->second)[cur_stream]->mID;  //流ID
                }

                solution << ">";
                begin_Client = false;
            }

            if(t == vTime.size() - 1 && c == vClient.size() - 1) continue;  //去掉最后一行换行
            solution << std::endl;
        }
    }

    solution.close();
    return ret;
}

//边缘节点输出：根据 mvAllocatedBand 将每个边缘节点输出成一列，第一行存放边缘节点ID，输出成.csv，可以在excel查看
bool WriteSiteSolution(const std::string& filename,
                       std::vector<Site>& vSite){
    bool ret = true;
    std::ofstream solution(filename, std::ios::out);
    if(!solution.is_open()){
        std::cout << "Site solution.txt flie open error" << std::endl;
        ret  = false;
        return ret;
    }

    for(int s = 0; s < vSite.size(); ++s){
        solution << vSite[s].mID << ", ";
    }
    solution << endl;

    for(int t = 0; t < vSite[0].mvAllocatedBand.size(); ++t){
        for(int s = 0; s < vSite.size(); ++s){
            solution << vSite[s].mvAllocatedBand[t] << ", ";
        }
        solution << endl;
    }

    solution.close();
    return ret;
}

void InitMember(vector<string>& mvTime,vector<Site>& mvSite,vector<Client>& mvClient){
    for(int c = 0; c < mvClient.size(); ++c){
        mvClient[c].mvsolution = vector<unordered_map<Site*,vector<BandwithStream*>>>(mvTime.size(), unordered_map<Site*,vector<BandwithStream*>>());
        mvClient[c].mvSumStream = vector<unsigned long long>(mvTime.size(), 0);
        sort(mvClient[c].mvLinkSite.begin(), mvClient[c].mvLinkSite.end(), cmpBandwiteSite);
        mvClient[c].msLinkSite = unordered_set<Site*>(mvClient[c].mvLinkSite.begin(), mvClient[c].mvLinkSite.end());
        /*for(int time = 0; time < mvTime.size(); ++time){
            for(int st = 0; st < mvClient[c].mvdemand[time].size(); ++st){
                mvClient[c].mvdemand[time][st].mLinkSitNumbers = mvClient[c].mvLinkSite.size();
            }
        }*/
    }
    for(int s = 0; s < mvSite.size(); ++s){
        mvSite[s].mCostBand = 0;
        mvSite[s].mCost = 0;
//        mvSite[s].mRunTimes = 0;
        mvSite[s].mAlldemand = 0;
        mvSite[s].IsSmooth = false;
        mvSite[s].mvAllocatedBand = vector<int>(mvTime.size(), 0);
        mvSite[s].mvSumStream = vector<unsigned long long>(mvTime.size(), 0);
        mvSite[s].mIsPer5Time = vector<bool>(mvTime.size(), false);
//        mvSite[s].mAllocateBandwithStream = vector< unordered_set<BandwithStream*> >(mvTime.size(), unordered_set<BandwithStream*>());
    }
}

bool InitialData(vector<string>& mvTime,vector<Site>& mvSite,vector<Client>& mvClient, PathtInf& path){
    std::unordered_map<std::string, int> Content;
    std::unordered_map<std::string, int> hash_Site_name_idx, hash_Client_name_idx;  //分别用哈希表分别存储 边缘节点的 ID 及其对应 mvSite 的下标，同理客户节点

    bool ret = true;
    ret = ReadConfig(path.configini,Content);
    qos_upper = Content["qos_constraint"];
    base_cost = Content["base_cost"];
    cout <<"qos_constraint: " << qos_upper <<", " << "base_cost: " << base_cost << endl;

    ret = ReadSitedData(path.site_bandwidthcsv, mvSite, hash_Site_name_idx);
    cout << "Site ID: " << mvSite[mvSite.size() - 1].mID <<", Site bandwidth: " << mvSite[mvSite.size() - 1].mbandwidth << endl;

    ret = ReadData(path.demandcsv,mvTime,mvClient,hash_Client_name_idx);
    cout <<"CLient ID: " << mvClient[mvClient.size() - 1].mID <<", 0时刻第一个有需求的流ID: "
         << mvClient[mvClient.size() - 1].mvdemand[0][0].mID << ", 需求带宽为： "
         << mvClient[mvClient.size() - 1].mvdemand[0][0].mdemand << endl;

    cout << "mvTime.size: " << mvTime.size() << endl;

    ret  = ReadQos(path.qoscsv, mvClient, mvSite,hash_Client_name_idx, hash_Site_name_idx);


    InitMember(mvTime, mvSite, mvClient);

    gAllStreams.reserve(mvTime.size());
    gstreamIsLinkSite.reserve(mvTime.size());
    for(int time = 0; time < mvTime.size(); ++time) {
        gAllStreams.push_back(vector<BandwithStream*>());
        gstreamIsLinkSite.push_back(vector<vector<bool>>());
        for (int c = 0; c < mvClient.size(); ++c) {
            Client *cli = (&mvClient[c]);
            for(int stream = 0; stream < cli->mvdemand[time].size(); ++stream){
                BandwithStream* str = (&(cli->mvdemand[time][stream]));
                gAllStreams[time].push_back(str);
                gstreamIsLinkSite[time].push_back(vector<bool>(mvSite.size(), false));
                cli->mvSumStream[time] += str->mdemand;
            }

        }

        sort(gAllStreams[time].begin(), gAllStreams[time].end(), cmpBandwithStream);
        gStreamNumbers.push_back(gAllStreams[time].size());

        for(int s = 0; s < mvSite.size(); ++s){
            Site* sit = (&mvSite[s]);
            for(int c = 0; c < sit->mvLinkClient.size(); ++c){
                Client* cli = sit->mvLinkClient[c];
                sit->mvSumStream[time] += cli->mvSumStream[time];
            }
            sit->mAlldemand += sit->mvSumStream[time];
        }

    }


    for(int time = 0; time < mvTime.size(); ++time){
        for(int st = 0; st < gAllStreams[time].size(); ++st){
            BandwithStream* stream = gAllStreams[time][st];
            Client* cli = stream->mCli;
            for(int s = 0; s < cli->mvLinkSite.size(); ++s){
                Site* sit = cli->mvLinkSite[s];
                int sit_idx = sit - (&mvSite[0]);
                gstreamIsLinkSite[time][st][sit_idx] = true;
//                 sit->mvLinkStream[time].push_back(stream);
            }
        }
    }
    cout << "gAllStreams[0].size: " << gAllStreams[0].size() << endl;

    return ret;
}

void findSiteStream( int time ,vector<BandwithStream*>& vec, Site* aimsit){
    int aimsit_idx = aimsit - (&mvSite[0]);
    for(int st = 0; st < gAllStreams[time].size(); ++st){
        BandwithStream* stream = gAllStreams[time][st];
        bool IsLink = gstreamIsLinkSite[time][st][aimsit_idx];
        Client* cli = stream->mCli;
        if( IsLink ){
            vec.push_back(stream);
        }
    }
}

inline void allocateStreamToSite(int time, Site* sit, Client* cli,
                                 BandwithStream* stream, bool IsPer5){
    if(stream->mIsAllocate) return;

//    sit->mvSumStream[time] -= stream->mdemand;
//    sit->mAllocateBandwithStream[time].insert(stream);
    sit->mvAllocatedBand[time] += stream->mdemand;
    stream->mSit = sit;

    if(IsPer5){
        sit->mCostBand = base_cost;
        sit->mCost = base_cost;
    }
    else{
        sit->mCostBand = max(sit->mCostBand, sit->mvAllocatedBand[time]);
        if(sit->mCostBand > 0 && sit->mCostBand < base_cost)
            sit->mCostBand = base_cost;

        if(sit->mCostBand == base_cost){
            sit->mCost = base_cost;
        }
        else if(sit->mCostBand == 0){
            sit->mCost = 0;
        }
        else{
            sit->mCost = (static_cast<double>(sit->mCostBand) - static_cast<double>(base_cost))*(static_cast<double>(sit->mCostBand) - static_cast<double>(base_cost))/static_cast<double>(sit->mbandwidth) + static_cast<double>(sit->mCostBand);
        }
    }

    cli->mvSumStream[time] -= stream->mdemand;
    cli->mvsolution[time][sit].push_back(stream);

    stream->mIsAllocate = true;

    gStreamNumbers[time] -= 1;
}

inline void myallocateStreamToSite(int time, Site* sit, Client* cli,
                                 BandwithStream* stream, bool IsPer5){
    if(stream->mIsAllocate) return;

//    sit->mvSumStream[time] -= stream->mdemand;
//    sit->mAllocateBandwithStream[time].insert(stream);
    sit->mvAllocatedBand[time] += stream->mdemand;
    stream->mSit = sit;

    if(IsPer5){
        sit->mCostBand = base_cost;
        sit->mCost = base_cost;
    }

    cli->mvSumStream[time] -= stream->mdemand;
    cli->mvsolution[time][sit].push_back(stream);

    stream->mIsAllocate = true;

    gStreamNumbers[time] -= 1;
}

void processPer5(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient){
//    sort(mvSitePtr.begin(), mvSitePtr.end(), cmpBandwiteSite);

//    int CanRuntimes = floor(mvTime.size()*0.05);
    if(RunTimes90 <= 0)
        return;


    //带宽上限大的边缘节点优先，整个时序假设都分配，最终真实选取的 5% 个时刻是这些时刻假设分配时分配最多的那 5% 个时刻。可能造成的影响：后面偷跑的量少了
    /*for(int s = 0; s < mvSitePtr.size(); ++s){
        Site* sit = mvSitePtr[s];
        vector<vector<bool>> usedStream;
        priority_queue<pair<int, int>, vector<pair<int, int>>, cmpPreSitePri> Per5;

        unsigned long long AllRunBand = 0;

        for(int time = 0; time < mvTime.size(); ++time){
            int AllocateBand_curtime = 0;
            vector<bool> usedStream_curtime(gAllStreams[time].size(), false);

            for(int st = 0; st < gAllStreams[time].size(); ++st) {
                BandwithStream *stream = gAllStreams[time][st];
                if (stream->mIsAllocate) continue;  //已经分配给前面的边缘节点了

                if(AllocateBand_curtime + stream->mdemand > sit->mbandwidth)    //当前时刻再分配 stream 给 sit 会超出其边缘上限
                    continue;

                Client* cli = stream->mCli;
                if(cli->msLinkSite.find(sit) == cli->msLinkSite.end())  //stream不可分配给 sit，没连通
                    continue;

                //可分配
                usedStream_curtime[st] = true;  //假设分配给 sit
                AllocateBand_curtime += stream->mdemand;    //更新假设情况 sit 已分配带宽
            }

            usedStream.push_back(usedStream_curtime);   //将当前时刻假设分配情况插入usedStream

            //得到 sit 假设分配后最多的 5% 个时刻
            if(Per5.size() < CanRuntimes){
                AllRunBand += AllocateBand_curtime;
                Per5.push(make_pair(time, AllocateBand_curtime));
            }
            else{
                if(AllocateBand_curtime > Per5.top().second){
                    AllRunBand -= Per5.top().second;
                    Per5.pop();
                    AllRunBand += AllocateBand_curtime;
                    Per5.push(make_pair(time, AllocateBand_curtime));
                }
            }
        }

        //如果偷跑的 5% 带宽加起来还不到 V ，那就不跑这个边缘节点
        if(AllRunBand < base_cost)
            continue;

        while(Per5.size() > 0){
            int time = Per5.top().first;
            Per5.pop();
            vector<bool> usedStream_curtime = usedStream[time];
            sit->mIsPer5Time[time] = true;

            for(int st = 0; st < usedStream_curtime.size(); ++st){
                if(!usedStream_curtime[st]) continue;

                BandwithStream* stream = gAllStreams[time][st];
                Client* cli = stream->mCli;

                allocateStreamToSite(time,sit,cli,stream,true);
            }
        }
    }*/

    double count_time1 = 0, count_time2 = 0, count_time3 = 0;
    int threshold = 2.5*base_cost*RunTimes;

    //整个时序总需求大的边缘节点优先，偷跑其中需求最大的5%个时刻（即优先开时序总需求大的边缘节点）
    for(int s = mvSitePtr.size() - 1; s >= 0; --s) {
     //    for(int s = 0; s < mvSitePtr.size(); ++s) {
        clock_t beginTime = clock();

        Site *sit = mvSitePtr[s];


        //小根堆，存时刻总需求最大的5%个时刻的 time 及其 时刻总需求
        priority_queue<pair<int, unsigned long long>, vector<pair<int, unsigned long long>>, cmpSitePri> Per5;

        unsigned long long AllRunBand = 0;  //记录5%能偷跑掉多少带宽

        int curRuntime = RunTimes;
        if(sit->mIsPer90 == true) curRuntime = RunTimes90;
        //找出那 5% 个时刻
        for (int time = 0; time < mvTime.size(); ++time) {
            if (Per5.size() < curRuntime) {
                Per5.push(make_pair(time, sit->mvSumStream[time]));
                sit->mIsPer5Time[time] = true;
                AllRunBand += sit->mvSumStream[time];
            } else {
                if (sit->mvSumStream[time] > Per5.top().second) {
                    sit->mIsPer5Time[Per5.top().first] = false;
                    AllRunBand -= Per5.top().second;
                    Per5.pop();
                    Per5.push(make_pair(time, sit->mvSumStream[time]));
                    sit->mIsPer5Time[time] = true;
                    AllRunBand += sit->mvSumStream[time];
                }
            }
        }

        clock_t endTime1 = clock();
        count_time1 += (double)(endTime1 - beginTime) / CLOCKS_PER_SEC;

        //对那 5% 个偷跑时刻进行分配流（即偷跑），注意：Per5中可能存着时刻总需求为 0 的时刻，因为这个小根堆是放满的，0也放进去
        while (Per5.size() > 0) {
            int time = Per5.top().first;    //时刻
            unsigned long long goal = Per5.top().second;    //时刻总需求
            Per5.pop();
            if (goal == 0) {    //时刻总需求为 0，那这个时刻就不偷跑
                sit->mIsPer5Time[time] = false; //好像可以去掉
                continue;
            }

            // if(AllRunBand < 4*base_cost){   //如果最大的 5% 个时刻的总需求 < 4*base_cost，那么这个边缘节点就不开，即此处会不断跳过，最终这个边缘节点没偷跑
//            if(AllRunBand < threshold){   //0.25*(base_cost/sit->mbandwidth)*sit->mbandwidth*RunTimes
//                sit->mIsPer5Time[time] = false;
//                continue;
//            }
            // 算 sit 在 time时刻旗下的 流片
            vector<BandwithStream*> vecStream;
            findSiteStream( time ,vecStream,sit);

            //进入到这里证明这个边缘节点是要偷跑的，且 time 这个时刻总需求不为 0
            //实际对 time 这个时刻进行流量分配，能塞多少塞多少
            // for (int st = 0; st < sit->mvLinkStream[time].size(); ++st) {
            for (int st = 0; st < vecStream.size(); ++st) {
                BandwithStream *stream = vecStream[st];
                if (stream->mIsAllocate) continue;

                if (sit->mvAllocatedBand[time] + stream->mdemand > sit->mbandwidth)
                    continue;

                Client *cli = stream->mCli;
                if (cli->msLinkSite.find(sit) == cli->msLinkSite.end())
                    continue;

                allocateStreamToSite(time, sit, cli, stream, true);

                //每分配出一个流量片，都要将与其相连的所有边缘节点的对应时刻的总需求 减掉，这会影响选择 5% 个时刻的时候选了哪些时刻
                for (int cls = 0; cls < cli->mvLinkSite.size(); ++cls) {
                    Site *CliLinkSti = cli->mvLinkSite[cls];
                    CliLinkSti->mvSumStream[time] -= stream->mdemand;
                }
            }//该时刻分配结束

            //这个5%时刻偷跑了，所以已偷跑的时刻 +1，同时将该时刻实际分配的流量存入 mRealPer5
            if (sit->mvAllocatedBand[time] > 0) {
                sit->mRealPer5.push(make_pair(time, sit->mvAllocatedBand[time]));
            }
        }

        //到这里 5% 时刻的流量分配就已经结束了，接下来是如果该边缘节点有偷跑，则0-95%的部分平铺一个 V
        clock_t endTime2 = clock();
        count_time2 += (double)(endTime2 - endTime1) / CLOCKS_PER_SEC;

        if (sit->mCost > 0 && base_cost > 0) {   //该边缘节点偷跑了，则铺平 0-95% 到 V
            for (int t = 0; t < mvTime.size(); ++t) {
                if (!sit->mIsPer5Time[t]) {
                    // 算 sit 在 time时刻旗下的 流片
                    vector<BandwithStream*> vecStream;
                    findSiteStream( t ,vecStream,sit);

                    for (int st = 0; st < vecStream.size(); ++st) {
                        BandwithStream *stream = vecStream[st];
                        if (stream->mIsAllocate) continue;
                        //只要这个流还未分配且放进去后不超过 V，就尽量放
                        if (stream->mdemand + sit->mvAllocatedBand[t] <= base_cost) {
                            Client *cli = stream->mCli;

                            allocateStreamToSite(t, sit, cli, stream, false);

                            //每分配出一个流量片，都要将与其相连的所有边缘节点的对应时刻的总需求 减掉，这会影响选择 5% 个时刻的时候选了哪些时刻
                            for (int cls = 0; cls < cli->mvLinkSite.size(); ++cls) {
                                Site *CliLinkSti = cli->mvLinkSite[cls];
                                CliLinkSti->mvSumStream[t] -= stream->mdemand;
                            }
                        }
                    }
                }
            }
        }

        clock_t endTime3 = clock();
        count_time3 += (double)(endTime3 - endTime2) / CLOCKS_PER_SEC;
    }
#ifdef LOCALPATH
    std::cout << "找出5%时刻运行时间： " << count_time1 << std::endl;
    std::cout << "5%时刻分配运行时间： " << count_time2 << std::endl;
    std::cout << "平铺运行时间： " << count_time3 << std::endl;
#endif

}

Site* chooseSiteToAllocateStream( int time , BandwithStream* stream ){
    Site* ret = NULL;
    double newCostDistance = DBL_MAX;
    Client* cli = stream->mCli;
    for( int idx = 0; idx < cli->mvLinkSite.size(); idx++ ){
        Site* sit = cli->mvLinkSite[idx];
        int curRunTime = RunTimes;
        if(sit->mIsPer90 == true) curRunTime = RunTimes90;
//        if( sit->mIsPer5Time[time] )
//            continue;
        //cout<<"site connections "<<sit->mvLinkClient.size()<<endl;
        int siteGetBandwith = sit->mvAllocatedBand[time] + stream->mdemand;
        // 选择价值最低的边缘点
        if( sit->mbandwidth  > siteGetBandwith && !stream->mIsAllocate ){
            if( siteGetBandwith > sit->mCostBand ){
                if(siteGetBandwith < base_cost) //进入该条件只可能是 sit->mCostBand = 0，即还没开的边缘节点（开了的最少都是 base_cost）
                    siteGetBandwith = base_cost;

                if(sit->mRealPer5.size() < curRunTime) {
                    siteGetBandwith = base_cost;
                }
                else if (sit->mRealPer5.size() == curRunTime && sit->mIsPer5Time[time] == true){
                    siteGetBandwith = sit->mCostBand;
                }
                else if (sit->mRealPer5.size() == curRunTime && sit->mIsPer5Time[time] == false){
                    siteGetBandwith = sit->mRealPer5.top().second + stream->mdemand;
                }

                // 如果进入这个循环说明，没能顶掉堆顶，所以一定是小于堆顶的，
                // 但是又大于mCostBand，所以需要更新的mcostBand就是 siteGetBandwith
                double newCostTemp = (static_cast<double>(siteGetBandwith) - static_cast<double>(base_cost) )*(static_cast<double>(siteGetBandwith) - static_cast<double>(base_cost) )/static_cast<double>(sit->mbandwidth) + static_cast<double>(siteGetBandwith);
                if( newCostDistance > newCostTemp - sit->mCost ){
                    newCostDistance = newCostTemp - sit->mCost;
                    ret = sit;
                }
            }else{
                ret = sit;
                break;
            }
        }
    }

    if(ret){
        // 看是否能顶替掉per5的时刻
        int curRunTime = RunTimes;
        if(ret->mIsPer90 == true) curRunTime = RunTimes90;
        if(ret->mRealPer5.size() >= 0 ){
            int siteGetBandwith = ret->mvAllocatedBand[time] + stream->mdemand;

            if(ret->mRealPer5.size() < curRunTime)    //如果是衅开的或者还没跑够 5% ，插入一个0，后面再pop掉
                ret->mRealPer5.push(make_pair(-1,0));

            int demand = ret->mRealPer5.top().second;
            if( siteGetBandwith > demand ){
                ++loopNumbers;
                int leftDemandBandwith = ret->mbandwidth - ret->mvAllocatedBand[time];
                if( leftDemandBandwith >= stream->mdemand ){
                    allocateStreamToSite(time,ret,stream->mCli,stream,0);
                    leftDemandBandwith -= stream->mdemand;
                }
                // treaversal all the stream and allocate the stream to the site
                for( int c = 0; c < ret->mvLinkClient.size(); c++ ){
                    Client* cli = ret->mvLinkClient[c];
                    for( int s = 0; s < cli->mvdemand[time].size(); s++ ){
                        BandwithStream* str = &cli->mvdemand[time][s];
                        if( leftDemandBandwith >= str->mdemand ){
                            allocateStreamToSite(time,ret,stream->mCli,str,0);
                            leftDemandBandwith -= str->mdemand;
                        }
                    }
                }
                if(time != ret->mRealPer5.top().first) ret->mCostBand = ret->mRealPer5.top().second;
                if(ret->mCostBand < base_cost)  //因为这个边缘节点到这里一定有用到了，所以就算 ret->mRealPer5.top() < base_cost, mCostBand 也要 = base_cost
                    ret->mCostBand = base_cost;

                if( ret->mCostBand > base_cost ){
                    ret->mCost =(static_cast<double>(ret->mCostBand) - static_cast<double>(base_cost) ) *( static_cast<double>(ret->mCostBand) - static_cast<double>(base_cost) ) / static_cast<double>(ret->mbandwidth) + static_cast<double>(ret->mCostBand);
                }else if( ret->mCostBand > 0 ){
                    ret->mCost = base_cost;
                }else{
                    ret->mCost = 0;
                }
                ret->mRealPer5.push(make_pair(time, ret->mvAllocatedBand[time]));
                ret->mRealPer5.pop();
            }
        }
    }

    return ret;
}
/*
Site* chooseSiteToAllocateStreamCost( int time , BandwithStream* stream ){
    Site* ret = NULL;
    double newCostDistance = DBL_MAX;
    Client* cli = stream->mCli;
    for( int idx = 0; idx < cli->mvLinkSite.size(); idx++ ){
        Site* sit = cli->mvLinkSite[idx];
        if( sit->mIsPer5Time[time] )
            continue;
        //cout<<"site connections "<<sit->mvLinkClient.size()<<endl;
        int siteGetBandwith = sit->mvAllocatedBand[time] + stream->mdemand;
        // 选择价值最低的边缘点
        if( sit->mbandwidth  > siteGetBandwith && !stream->mIsAllocate ){
            if( siteGetBandwith > sit->mCostBand ){
                if(siteGetBandwith < base_cost) //进入该条件只可能是 sit->mCostBand = 0，即还没开的边缘节点（开了的最少都是 base_cost）
                    siteGetBandwith = base_cost;

                if(sit->mRealPer5.size() < RunTimes) {
                    siteGetBandwith = base_cost;
                }
                else if (sit->mRealPer5.size() == RunTimes && (siteGetBandwith > (sit->mRealPer5.top().second))){
                    siteGetBandwith = sit->mRealPer5.top().second + stream->mdemand;
                }

                // 如果进入这个循环说明，没能顶掉堆顶，所以一定是小于堆顶的，
                // 但是又大于mCostBand，所以需要更新的mcostBand就是 siteGetBandwith
                double newCostTemp;
                if(sit->mCostBand > 0) {
                    newCostTemp = (static_cast<double>(siteGetBandwith) - static_cast<double>(base_cost)) *(static_cast<double>(siteGetBandwith) - static_cast<double>(base_cost)) /static_cast<double>(sit->mbandwidth) + static_cast<double>(siteGetBandwith);
                    newCostTemp /= static_cast<double>(siteGetBandwith);
                }
                else{
                    newCostTemp = static_cast<double>(base_cost)/static_cast<double>(stream->mdemand);
                }
                if( newCostDistance > newCostTemp ){
                    newCostDistance = newCostTemp;
                    ret = sit;
                }
            }else{
                ret = sit;
                break;
            }
        }
    }

    if(ret){
        // 看是否能顶替掉per5的时刻
        if(ret->mRealPer5.size() >= 0 ){
            int siteGetBandwith = ret->mvAllocatedBand[time] + stream->mdemand;

            if(ret->mRealPer5.size() < RunTimes)    //如果是衅开的或者还没跑够 5% ，插入一个0，后面再pop掉
                ret->mRealPer5.push(make_pair(-1,0));

            int demand = ret->mRealPer5.top().second;
            if( siteGetBandwith > demand ){
                ++loopNumbers;
                int leftDemandBandwith = ret->mbandwidth - ret->mvAllocatedBand[time];
                if( leftDemandBandwith >= stream->mdemand ){
                    allocateStreamToSite(time,ret,stream->mCli,stream,0);
                    leftDemandBandwith -= stream->mdemand;
                }
                // treaversal all the stream and allocate the stream to the site
                for( int c = 0; c < ret->mvLinkClient.size(); c++ ){
                    Client* cli = ret->mvLinkClient[c];
                    for( int s = 0; s < cli->mvdemand[time].size(); s++ ){
                        BandwithStream* str = &cli->mvdemand[time][s];
                        if( leftDemandBandwith >= str->mdemand ){
                            allocateStreamToSite(time,ret,stream->mCli,str,0);
                            leftDemandBandwith -= str->mdemand;
                        }
                    }
                }
                ret->mCostBand = ret->mRealPer5.top().second;
                if(ret->mCostBand < base_cost)  //因为这个边缘节点到这里一定有用到了，所以就算 ret->mRealPer5.top() < base_cost, mCostBand 也要 = base_cost
                    ret->mCostBand = base_cost;

                if( ret->mCostBand > base_cost ){
                    ret->mCost =(static_cast<double>(ret->mCostBand) - static_cast<double>(base_cost) ) *( static_cast<double>(ret->mCostBand) - static_cast<double>(base_cost) ) / static_cast<double>(ret->mbandwidth) + static_cast<double>(ret->mCostBand);
                }else if( ret->mCostBand > 0 ){
                    ret->mCost = base_cost;
                }else{
                    ret->mCost = 0;
                }
                ret->mRealPer5.push(make_pair(time, ret->mvAllocatedBand[time]));
                ret->mRealPer5.pop();
            }
        }
    }

    return ret;
}
*/

Site* mychooseSiteToAllocateStream( int time , BandwithStream* stream ){
    Site* ret = NULL;
    double newCostDistance = DBL_MAX;
    Client* cli = stream->mCli;
    for( int idx = 0; idx < cli->mvLinkSite.size(); idx++ ){
        Site* sit = cli->mvLinkSite[idx];
        int threshold = sit->mCostBand;
        if( sit->mIsPer5Time[time] )
            threshold = sit->mbandwidth;
        //cout<<"site connections "<<sit->mvLinkClient.size()<<endl;
        int siteGetBandwith = sit->mvAllocatedBand[time] + stream->mdemand;
        // 选择价值最低的边缘点
        if( sit->mbandwidth  >= siteGetBandwith && !stream->mIsAllocate ){
            if( siteGetBandwith > threshold ){
                if(siteGetBandwith < base_cost) //进入该条件只可能是 sit->mCostBand = 0，即还没开的边缘节点（开了的最少都是 base_cost）
                    siteGetBandwith = base_cost;

                // 如果进入这个循环说明，没能顶掉堆顶，所以一定是小于堆顶的，
                // 但是又大于mCostBand，所以需要更新的mcostBand就是 siteGetBandwith
                if(sit->mRealPer5.size() < RunTimes) {
                    siteGetBandwith = base_cost;
                }
                else if (sit->mRealPer5.size() == RunTimes && siteGetBandwith > sit->mRealPer5.top().second){
                    siteGetBandwith = sit->mRealPer5.top().second;
                }
                double newCostTemp = ( siteGetBandwith - base_cost )*( siteGetBandwith - base_cost )/sit->mbandwidth + siteGetBandwith;
                if( newCostDistance > newCostTemp - sit->mCost ){   //离谱 newCostTemp 改成 double 比较也会运行失败
                    newCostDistance = newCostTemp - sit->mCost;
                    ret = sit;
                }
            }else{
                ret = sit;
                break;
            }
        }
    }

    if(ret){
        // 看是否能顶替掉per5的时刻
        if(ret->mRealPer5.size() >= 0 ){
            int siteGetBandwith = ret->mvAllocatedBand[time] + stream->mdemand;

            if(ret->mRealPer5.size() < RunTimes)    //如果是衅开的或者还没跑够 5% ，插入一个0，后面再pop掉
                ret->mRealPer5.push(make_pair(-1,0));

            int demand = ret->mRealPer5.top().second;
            if( siteGetBandwith > demand ){
                ++loopNumbers;
                int leftDemandBandwith = ret->mbandwidth - ret->mvAllocatedBand[time];
                if( leftDemandBandwith >= stream->mdemand ){
                    allocateStreamToSite(time,ret,stream->mCli,stream,0);
                    leftDemandBandwith -= stream->mdemand;
                }
                // treaversal all the stream and allocate the stream to the site
                /*for( int c = 0; c < ret->mvLinkClient.size(); c++ ){
                    Client* cli = ret->mvLinkClient[c];
                    for( int s = 0; s < cli->mvdemand[time].size(); s++ ){
                        BandwithStream* str = &cli->mvdemand[time][s];
                        if( leftDemandBandwith >= str->mdemand ){
                            allocateStreamToSite(time,ret,stream->mCli,str,0);
                            leftDemandBandwith -= str->mdemand;
                        }
                    }
                }*/

                ret->mCostBand = ret->mRealPer5.top().second;
                if(ret->mCostBand < base_cost)  //因为这个边缘节点到这里一定有用到了，所以就算 ret->mRealPer5.top() < base_cost, mCostBand 也要 = base_cost
                    ret->mCostBand = base_cost;

                if( ret->mCostBand > base_cost ){
                    ret->mCost =( ret->mCostBand - static_cast<double>(base_cost) ) *( ret->mCostBand - static_cast<double>(base_cost) ) / ret->mbandwidth + ret->mCostBand;
                }else if( ret->mCostBand > 0 ){
                    ret->mCost = base_cost;
                }else{
                    ret->mCost = 0;
                }
                ret->mIsPer5Time[time] = true;
                ret->mRealPer5.push(make_pair(time, ret->mvAllocatedBand[time]));
                if(ret->mRealPer5.top().first >= 0)
                    ret->mIsPer5Time[ret->mRealPer5.top().first] = false;
                ret->mRealPer5.pop();
            }
            else{
                allocateStreamToSite(time,ret,stream->mCli,stream,0);
                ret->mCostBand = max(ret->mCostBand, ret->mvAllocatedBand[time]);

                if(ret->mCostBand < base_cost)
                    ret->mCostBand = base_cost;

                ret->mCost = (ret->mCostBand - base_cost)*(ret->mCostBand - base_cost)/ret->mbandwidth + ret->mCostBand;
            }
        }
    }

    return ret;
}
int solvethreshold(double value, int bandwidth){
    double b = -(2* static_cast<double>(base_cost) + (value - 1.0)*static_cast<double>(bandwidth));
    double a = 1;
    double c = static_cast<double>(base_cost)*static_cast<double>(base_cost);
    int threshold = ceil((-b + sqrt(b*b - 4*a*c))/(2*a));
    return threshold;
}
inline void allocateStreamToSite_building(int time, Site* sit, Client* cli,
                                          BandwithStream* stream){
//    sit->mAllocateBandwithStream[time].insert(stream);
    sit->mvAllocatedBand[time] += stream->mdemand;
    stream->mSit = sit;

    cli->mvSumStream[time] -= stream->mdemand;
    cli->mvsolution[time][sit].push_back(stream);

    stream->mIsAllocate = true;

    gStreamNumbers[time] -= 1;
}


void processPer95(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient){
    for(int time = 0; time < mvTime.size(); time++ ){
        // 直接按照gAllStreams中顺序选取最大的
        vector<BandwithStream*> vecBandwithStream = gAllStreams[time];
        int stremNumbers = gStreamNumbers[time];
        for(int idx = 0; idx < vecBandwithStream.size(); idx++ ){
            BandwithStream* stream = vecBandwithStream[idx];
            if(stream->mIsAllocate) continue;

            Site* sit = chooseSiteToAllocateStream( time, stream );
            if( sit ){
                allocateStreamToSite(time,sit,stream->mCli,stream,0 );
            }else{
                #ifdef LOCALPATH
                cout<<" choose site error: "<<__FILE__<<", "<<__LINE__<<endl;
                #endif
            }
            stremNumbers--;
        }
    }
}
bool processPer95_building(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient, double value){
    bool ret = true;
    /*for(int time = 0; time < mvTime.size(); ++time){
        for(int s = 0; s < mvSitePtr.size(); ++s){
            Site* sit = mvSitePtr[s];
            int threshold = solvethreshold(value,sit->mbandwidth);

            if(s == 0 && time == 0) cout << "threshold: " << threshold << endl;

            // 算sit 在 time 时刻旗下的流片
            vector<BandwithStream*> vecStream;
            findSiteStream( time ,vecStream,sit);

            for(int st = 0; st < vecStream.size(); ++st){

                BandwithStream* stream = vecStream[st];

                if(stream->mIsAllocate)
                    continue;
                if(stream->mdemand + sit->mvAllocatedBand[time] > threshold){
                    ret = false;
                    continue;
                }

                Client* cli = stream->mCli;
                allocateStreamToSite_building(time, sit, cli, stream);
            }
        }
    }*/

    int arrthreshold[135] = {0};
    for(int s = 0; s < mvSitePtr.size(); ++s) {
        arrthreshold[s] = solvethreshold(value, mvSitePtr[s]->mbandwidth);
    }
    for(int time = 0; time < mvTime.size(); ++time){
        vector<BandwithStream*> vstream = gAllStreams[time];
        for(int st = 0; st < vstream.size(); ++st){
            BandwithStream* stream = vstream[st];
            if(stream->mIsAllocate) continue;

            for(int s = 0; s < mvSitePtr.size(); ++s){
                Site* sit = mvSitePtr[s];
                int sit_idx = sit - (&mvSite[0]);
                int threshold = arrthreshold[s];
                bool link = gstreamIsLinkSite[time][st][sit_idx];
                if(!link) continue;

                if(stream->mdemand + sit->mvAllocatedBand[time] > threshold){
                    ret = false;
                    continue;
                }
                else {
                    Client *cli = stream->mCli;
                    allocateStreamToSite_building(time, sit, cli, stream);
                    break;
                }
            }
        }
    }
    return ret;
}
void calculateCost(int times, vector<Site*>& mvSitePtr){
    for(int s = 0; s < mvSitePtr.size(); ++s){
        Site* sit = mvSitePtr[s];
        int curRunTime = RunTimes;
        if(sit->mIsPer90 == true) curRunTime = RunTimes90;
        priority_queue<pair<int, int>, vector<pair<int, int>>, cmpPreSitePri> qcost;
        for(int time = 0; time < times; ++time){
            if(sit->mvAllocatedBand[time] == 0) continue;
            if(qcost.size() < curRunTime + 1){
                qcost.push(make_pair(time, sit->mvAllocatedBand[time]));
            }
            else{
                qcost.pop();
                qcost.push(make_pair(time, sit->mvAllocatedBand[time]));
            }
        }
        //caculate CostBand
        if(qcost.empty())
            sit->mCostBand = 0;
        else if(qcost.size() < curRunTime + 1)
            sit->mCostBand = base_cost;
        else{
            sit->mCostBand = max(base_cost, qcost.top().second);
            qcost.pop();
        }
        //caculate Cost
        if(sit->mCostBand == 0)
            sit->mCost = 0;
        else
            sit->mCost = (static_cast<double>(sit->mCostBand) - static_cast<double>(base_cost))*(static_cast<double>(sit->mCostBand) - static_cast<double>(base_cost))/static_cast<double>(sit->mbandwidth) + sit->mCostBand;
        //reset RealPer5 and IsPer5Time
        while(!sit->mRealPer5.empty()){
            sit->mRealPer5.pop();
        }
        sit->mIsPer5Time = vector<bool>(times, false);
        while(!qcost.empty()){
            int time = qcost.top().first;
            sit->mRealPer5.push(make_pair(time, sit->mvAllocatedBand[time]));
            sit->mIsPer5Time[time] = true;
            qcost.pop();
        }

    }
}


//平滑函数的流量再分配
inline void SmoothallocateStreamToSite(int time, Site* sit, Client* cli, BandwithStream* stream){
    stream->mSit = sit;
    stream->mCli = cli;

    sit->mvAllocatedBand[time] += stream->mdemand;
    cli->mvsolution[time][sit].push_back(stream);
}

void ClearSmooth(vector<Site*>& mvSitePtr){
    for(int s = 0; s < mvSitePtr.size(); ++s){
        mvSitePtr[s]->IsSmooth = false;
    }
}
//按95逆顺序抹平 Smooth_Inverse95
void Smooth(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient){
    //TODO:
    ClearSmooth(mvSitePtr);
    double count_find = 0;
    double count_In95 = 0;
    double count_In5 = 0;
    for(int s = mvSitePtr.size() - 1; s > 0; --s){
        clock_t smooth_begin = clock();

        Site* sit = mvSitePtr[s];

        sit->IsSmooth = true;   //记录该边缘节点为已平滑
        if(sit->mCost == 0) continue;   //每用过的节点直接跳过，这样每用过的节点也会被记录成已平滑

        //进入到这里代表这个边缘节点还没平滑过且肯定是有用的
        //找出计费时刻的带宽（再找一次，避免上面 5 或 95函数哪个过程出现了小错误导致 mCostBand 记录出错）
        priority_queue<pair<int, int>, vector<pair<int, int>>, cmpRealPer5> Per5Add1;    //长度为 5% + 1，所以堆顶就是 95% 位置的带宽
        //更新小根堆找出 95% 位置带宽（0 放入 ）
        for(int time = 0; time < mvTime.size(); ++time){
            if(Per5Add1.size() < RunTimes + 1){
                Per5Add1.push(make_pair(time, mvSitePtr[s]->mvAllocatedBand[time]));
            }
            else{
                if(mvSitePtr[s]->mvAllocatedBand[time] > Per5Add1.top().second){
                    Per5Add1.pop();
                    Per5Add1.push(make_pair(time, mvSitePtr[s]->mvAllocatedBand[time]));
                }
            }
        }

        clock_t smooth_find = clock();
        count_find += (double)(smooth_find - smooth_begin) / CLOCKS_PER_SEC;

        int newCostBand = base_cost;    //因为该边缘节点有用过，直接将其 95% 位置最小带宽设置为 base_cost，这样就可以直接用第三条公式了，就算 Per5Add1.top() < base_cost，这样操作后第三条公式计算出来后还是 V
        if(Per5Add1.top().second > base_cost)   //如果堆顶超过 V 且堆顶是 95% 位置的带宽（因为我们在前面放进堆的时候 0 不放入，因此这里只有 Per5Add1.size() == RunTimes + 1 才能说明堆顶是 95% 位置的带宽）
            newCostBand = Per5Add1.top().second;
        Per5Add1.pop();

        sit->mIsPer5Time = vector<bool>(mvTime.size(), false);
        while(Per5Add1.size() > 0){
            int time = Per5Add1.top().first;
            sit->mIsPer5Time[time] = true;
            Per5Add1.pop();
        }

        //更新mCostBand，到这里为止都是为了避免 mCostBand出错
        mvSitePtr[s]->mCostBand = newCostBand;
        //计算成本(其实用不到，顺手)
        mvSitePtr[s]->mCost = (static_cast<double>(newCostBand) - static_cast<double>(base_cost))*(static_cast<double>(newCostBand) - static_cast<double>(base_cost))/static_cast<double>(mvSitePtr[s]->mbandwidth) + static_cast<double>(newCostBand);

        int goal_bandwidth = sit->mCostBand;    //0-95%的目标就是拉到 sit->mCostBand

        for(int time = 0; time < mvTime.size(); ++time){
            clock_t cbegin_In = clock();
            if(sit->mvAllocatedBand[time] == goal_bandwidth && sit->mIsPer5Time[time] == false)
                continue; //0-95 等于的就不用平滑了

            int goal = goal_bandwidth - sit->mvAllocatedBand[time]; //0 - 95%达到 goal_bandwidth 还需要多少带宽
            if(sit->mIsPer5Time[time] == true) //如果 sit->mvAllocatedBand[time] > goal_bandwidth ，表示这是偷跑的时刻，因为 95% 可能出现顶替现象，所以这里再把5%部分拉大一下，目标是拉到真实带宽上限
                goal = sit->mbandwidth - sit->mvAllocatedBand[time];

            vector<BandwithStream*> vStreamInPer5;

            // 算 sit 在 time时刻旗下的 流片
            vector<BandwithStream*> vecStream;
            findSiteStream( time ,vecStream,sit);
            //到了这里，时刻 time 还需要从别处取多少带宽就已经算出来了，就是 goal
            //开始根据从该边缘节点连接的流中取
            for(int st = 0; st < vecStream.size(); ++st){
                BandwithStream* stream = vecStream[st];

                //如果这个流是分配给该边缘节点本身或分配给已经平滑过的边缘节点，则跳过
                if(stream->mSit == sit || stream->mdemand > goal || stream->mSit->IsSmooth == true)
                    continue;

                if(stream->mSit->mIsPer5Time[time] == true){
                    vStreamInPer5.emplace_back(stream);
                    continue;
                }

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }

            clock_t csmoothIn95 = clock() ;
            count_In95 += (double)(csmoothIn95 - cbegin_In) / CLOCKS_PER_SEC;

            //分配属于 5% 的流给sit
            for(int st = 0; st < vStreamInPer5.size() && goal > 0; ++st){
                BandwithStream* stream = vStreamInPer5[st];
                if(stream->mdemand > goal)
                    continue;

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }
            clock_t csmoothIn5 = clock() ;
            count_In5 += (double)(csmoothIn5 - csmoothIn95) / CLOCKS_PER_SEC;
        }

    }

    cout << "smooth 查找时刻耗时：" << count_find << endl;
    cout << "smooth 取其他95耗时：" << count_find << endl;
    cout << "smooth 取其他5耗时：" << count_find << endl;
}
void Smooth1(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient){
    //TODO:
    ClearSmooth(mvSitePtr);
    for(int s = 0; s < mvSitePtr.size() - 1; ++s){
        Site* sit = mvSitePtr[s];

        sit->IsSmooth = true;   //记录该边缘节点为已平滑
        if(sit->mCost == 0) continue;   //每用过的节点直接跳过，这样每用过的节点也会被记录成已平滑

        //进入到这里代表这个边缘节点还没平滑过且肯定是有用的
        //找出计费时刻的带宽（再找一次，避免上面 5 或 95函数哪个过程出现了小错误导致 mCostBand 记录出错）
        priority_queue<pair<int, int>, vector<pair<int, int>>, cmpRealPer5> Per5Add1;    //长度为 5% + 1，所以堆顶就是 95% 位置的带宽
        //更新小根堆找出 95% 位置带宽（0 不放入 ）
        for(int time = 0; time < mvTime.size(); ++time){
            if(Per5Add1.size() < RunTimes + 1){
                Per5Add1.push(make_pair(time, mvSitePtr[s]->mvAllocatedBand[time]));
            }
            else{
                if(mvSitePtr[s]->mvAllocatedBand[time] > Per5Add1.top().second){
                    Per5Add1.pop();
                    Per5Add1.push(make_pair(time, mvSitePtr[s]->mvAllocatedBand[time]));
                }
            }
        }

        int newCostBand = base_cost;    //因为该边缘节点有用过，直接将其 95% 位置最小带宽设置为 base_cost，这样就可以直接用第三条公式了，就算 Per5Add1.top() < base_cost，这样操作后第三条公式计算出来后还是 V
        if(Per5Add1.top().second > base_cost)   //如果堆顶超过 V 且堆顶是 95% 位置的带宽（因为我们在前面放进堆的时候 0 不放入，因此这里只有 Per5Add1.size() == RunTimes + 1 才能说明堆顶是 95% 位置的带宽）
            newCostBand = Per5Add1.top().second;
        Per5Add1.pop();

        sit->mIsPer5Time = vector<bool>(mvTime.size(), false);
        while(Per5Add1.size() > 0){
            int time = Per5Add1.top().first;
            sit->mIsPer5Time[time] = true;
            Per5Add1.pop();
        }

        //更新mCostBand，到这里为止都是为了避免 mCostBand出错
        mvSitePtr[s]->mCostBand = newCostBand;
        //计算成本(其实用不到，顺手)
        mvSitePtr[s]->mCost = (static_cast<double>(newCostBand) - static_cast<double>(base_cost))*(static_cast<double>(newCostBand) - static_cast<double>(base_cost))/static_cast<double>(mvSitePtr[s]->mbandwidth) + static_cast<double>(newCostBand);

        int goal_bandwidth = sit->mCostBand;    //0-95%的目标就是拉到 sit->mCostBand

        for(int time = 0; time < mvTime.size(); ++time){
            if(sit->mvAllocatedBand[time] == goal_bandwidth && sit->mIsPer5Time[time] == false)
                continue; //0-95 等于的就不用平滑了

            int goal = goal_bandwidth - sit->mvAllocatedBand[time]; //0 - 95%达到 goal_bandwidth 还需要多少带宽
            if(sit->mIsPer5Time[time] == true) //如果 sit->mvAllocatedBand[time] > goal_bandwidth ，表示这是偷跑的时刻，因为 95% 可能出现顶替现象，所以这里再把5%部分拉大一下，目标是拉到真实带宽上限
                goal = sit->mbandwidth - sit->mvAllocatedBand[time];

            vector<BandwithStream*> vStreamInPer5;
            // 算 sit 在 time时刻旗下的 流片
            vector<BandwithStream*> vecStream;
            findSiteStream( time ,vecStream,sit);
            //到了这里，时刻 time 还需要从别处取多少带宽就已经算出来了，就是 goal
            //开始根据从该边缘节点连接的流中取
            for(int st = 0; st < vecStream.size(); ++st){
                BandwithStream* stream = vecStream[st];

                //如果这个流是分配给该边缘节点本身或分配给已经平滑过的边缘节点，则跳过
                if(stream->mSit == sit || stream->mdemand > goal || stream->mSit->IsSmooth == true)
                    continue;

                if(stream->mSit->mIsPer5Time[time] == true){
                    vStreamInPer5.push_back(stream);
                    continue;
                }

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }

            //分配属于 5% 的流给sit
            for(int st = 0; st < vStreamInPer5.size() && goal > 0; ++st){
                BandwithStream* stream = vStreamInPer5[st];
                if(stream->mdemand > goal)
                    continue;

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }
        }

    }
}
void caculateCost(Site* sit){
    if( sit->mCostBand > base_cost ){
        sit->mCost =( sit->mCostBand - static_cast<double>(base_cost) ) *( static_cast<double>(sit->mCostBand) - static_cast<double>(base_cost) ) / static_cast<double>(sit->mbandwidth) + static_cast<double>(sit->mCostBand);
    }else if( sit->mCostBand > 0 ){
        sit->mCost = base_cost;
    }else{
        sit->mCost = 0;
    }
}
void calculateCost(int times, Site* sit){
    priority_queue<pair<int, int>, vector<pair<int, int>>, cmpPreSitePri> qcost;
    for(int time = 0; time < times; ++time){
        if(sit->mvAllocatedBand[time] == 0) continue;
        if((qcost.size() < RunTimes + 1 && sit->mIsPer90 == false) || (qcost.size() < RunTimes90 + 1 && sit->mIsPer90 == true)){
            qcost.push(make_pair(time, sit->mvAllocatedBand[time]));
        }
        else{
            qcost.pop();
            qcost.push(make_pair(time, sit->mvAllocatedBand[time]));
        }
    }
    //caculate CostBand
    if(qcost.empty())
        sit->mCostBand = 0;
    else if((qcost.size() < RunTimes + 1 && sit->mIsPer90 == false) || (qcost.size() < RunTimes90 + 1 && sit->mIsPer90 == true))
        sit->mCostBand = base_cost;
    else{
        sit->mCostBand = max(base_cost, qcost.top().second);
        qcost.pop();
    }
    //caculate Cost
    caculateCost(sit);
    //reset RealPer5 and IsPer5Time
    while(!sit->mRealPer5.empty()){
        sit->mRealPer5.pop();
    }
    sit->mIsPer5Time = vector<bool>(times, false);
    while(!qcost.empty()){
        int time = qcost.top().first;
        sit->mRealPer5.push(make_pair(time, sit->mvAllocatedBand[time]));
        sit->mIsPer5Time[time] = true;
        qcost.pop();
    }
}

Site* chooseSmoothSite(int times, vector<Site*>& mvSitePtr){
    Site* ret = NULL;
    unsigned long long space = 0;
    double DisCost = DBL_MAX;
    for(int s = 0; s < mvSitePtr.size(); ++s){
        Site* sit = mvSitePtr[s];
        if(sit->IsSmooth || sit->mCost == 0) continue;
        else
            calculateCost(times,sit);

//        if(sit->mCost < DisCost){
//            DisCost = sit->mCost;
//            ret = sit;
//        }
        unsigned long long spaceTemp = 0;
        for(int time = 0; time < times; ++time){
            if(sit->mvAllocatedBand[time] < sit->mCostBand)
                spaceTemp += sit->mCostBand - sit->mvAllocatedBand[time];
        }
        if(spaceTemp > space){
            space = spaceTemp;
            ret = sit;
        }
    }
    return ret;
}
void newSmooth(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient){
    //TODO:
    ClearSmooth(mvSitePtr);
    int times = mvTime.size();
    for(int s = mvSitePtr.size() - 1; s > 0; --s){
        Site* sit = chooseSmoothSite(times, mvSitePtr);
        if(sit == NULL) continue;
        sit->IsSmooth = true;   //记录该边缘节点为已平滑

        int goal_bandwidth = sit->mCostBand;    //0-95%的目标就是拉到 sit->mCostBand

        for(int time = 0; time < mvTime.size(); ++time){
            if(sit->mvAllocatedBand[time] == goal_bandwidth && sit->mIsPer5Time[time] == false)
                continue; //0-95 等于的就不用平滑了

            int goal = goal_bandwidth - sit->mvAllocatedBand[time]; //0 - 95%达到 goal_bandwidth 还需要多少带宽
            if(sit->mIsPer5Time[time] == true) //如果 sit->mvAllocatedBand[time] > goal_bandwidth ，表示这是偷跑的时刻，因为 95% 可能出现顶替现象，所以这里再把5%部分拉大一下，目标是拉到真实带宽上限
                goal = sit->mbandwidth - sit->mvAllocatedBand[time];

            vector<BandwithStream*> vStreamInPer5;
            // 算 sit 在 time时刻旗下的 流片
            vector<BandwithStream*> vecStream;
            findSiteStream( time ,vecStream,sit);
            //到了这里，时刻 time 还需要从别处取多少带宽就已经算出来了，就是 goal
            //开始根据从该边缘节点连接的流中取
            for(int st = 0; st < vecStream.size(); ++st){
                BandwithStream* stream = vecStream[st];

                //如果这个流是分配给该边缘节点本身或分配给已经平滑过的边缘节点，则跳过
                if(stream->mSit == sit || stream->mdemand > goal || stream->mSit->IsSmooth == true)
                    continue;

                if(stream->mSit->mIsPer5Time[time] == true){
                    vStreamInPer5.push_back(stream);
                    continue;
                }

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                
                if( cli->mvsolution[time].find(src_sit) == cli->mvsolution[time].end() ){
                    continue;
                }
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }

            //分配属于 5% 的流给sit
            for(int st = 0; st < vStreamInPer5.size() && goal > 0; ++st){
                BandwithStream* stream = vStreamInPer5[st];
                if(stream->mdemand > goal)
                    continue;

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }
        }

    }
}

void newcaculateCost(int times, Site* sit){
    priority_queue<pair<int, int>, vector<pair<int, int>>, cmpPreSitePri> qcost;
    int cur_RunTimes = RunTimes;
    if(sit->mIsPer90) cur_RunTimes = RunTimes90;

    bool space_node = true;
    for(int time = 0; time < times; ++time){
        if(sit->mvAllocatedBand[time] > 0) space_node = false;

        if(qcost.size() < cur_RunTimes + 1){
            qcost.push(make_pair(time, sit->mvAllocatedBand[time]));
        }
        else{
            qcost.pop();
            qcost.push(make_pair(time, sit->mvAllocatedBand[time]));
        }
    }
    //caculate CostBand
    if(space_node)
        sit->mCostBand = 0;
    else{
        sit->mCostBand = max(base_cost, qcost.top().second);
        qcost.pop();
    }
    //caculate Cost
    caculateCost(sit);
    //reset RealPer5 and IsPer5Time
    while(!sit->mRealPer5.empty()){
        sit->mRealPer5.pop();
    }
    sit->mIsPer5Time = vector<bool>(times, false);
    while(!qcost.empty()){
        int time = qcost.top().first;
        sit->mRealPer5.push(make_pair(time, sit->mvAllocatedBand[time]));
        sit->mIsPer5Time[time] = true;
        qcost.pop();
    }
}
void Smoothtop(vector<string>& mvTime,vector<Site*>& mvSitePtr,vector<Client>& mvClient){
    //TODO:
    ClearSmooth(mvSitePtr);
    for(int s = 0; s < mvSitePtr.size(); ++s){
        newcaculateCost(mvTime.size(), mvSitePtr[s]);
    }

    for(int s = mvSitePtr.size() - 1; s > 0 ; --s){
        Site* sit = mvSitePtr[s];

        sit->IsSmooth = true;   //记录该边缘节点为已平滑
        if(sit->mCost == 0) continue;   //每用过的节点直接跳过，这样每用过的节点也会被记录成已平滑

        //进入到这里代表这个边缘节点还没平滑过且肯定是有用的
        //找出计费时刻的带宽（再找一次，避免上面 5 或 95函数哪个过程出现了小错误导致 mCostBand 记录出错）
        priority_queue<pair<int, int>, vector<pair<int, int>>, cmpRealPer5> Per5Add1;    //长度为 5% + 1，所以堆顶就是 95% 位置的带宽
        int cur_RunTimes = RunTimes;
        if(sit->mIsPer90) cur_RunTimes = RunTimes90;
        //更新小根堆找出 95% 位置带宽（0 放入 ）
        for(int time = 0; time < mvTime.size(); ++time){
            if(Per5Add1.size() < cur_RunTimes + 1){
                Per5Add1.push(make_pair(time, mvSitePtr[s]->mvAllocatedBand[time]));
            }
            else{
                if(mvSitePtr[s]->mvAllocatedBand[time] > Per5Add1.top().second){
                    Per5Add1.pop();
                    Per5Add1.push(make_pair(time, mvSitePtr[s]->mvAllocatedBand[time]));
                }
            }
        }

        int newCostBand = base_cost;    //因为该边缘节点有用过，直接将其 95% 位置最小带宽设置为 base_cost，这样就可以直接用第三条公式了，就算 Per5Add1.top() < base_cost，这样操作后第三条公式计算出来后还是 V
        if(Per5Add1.top().second > base_cost)   //如果堆顶超过 V 且堆顶是 95% 位置的带宽（因为我们在前面放进堆的时候 0 不放入，因此这里只有 Per5Add1.size() == RunTimes + 1 才能说明堆顶是 95% 位置的带宽）
            newCostBand = Per5Add1.top().second;
        Per5Add1.pop();

        sit->mIsPer5Time = vector<bool>(mvTime.size(), false);
        while(Per5Add1.size() > 0){
            int time = Per5Add1.top().first;
            sit->mIsPer5Time[time] = true;
            Per5Add1.pop();
        }

        //更新mCostBand，到这里为止都是为了避免 mCostBand出错
        sit->mCostBand = newCostBand;
        //计算成本(其实用不到，顺手)
        sit->mCost = (static_cast<double>(newCostBand) - static_cast<double>(base_cost))*(static_cast<double>(newCostBand) - static_cast<double>(base_cost))/static_cast<double>(mvSitePtr[s]->mbandwidth) + static_cast<double>(newCostBand);

        int goal_bandwidth = sit->mCostBand;    //0-95%的目标就是拉到 sit->mCostBand

        for(int time = 0; time < mvTime.size(); ++time){
            if(sit->mvAllocatedBand[time] == goal_bandwidth && sit->mIsPer5Time[time] == false)
                continue; //0-95 等于的就不用平滑了

            int goal = goal_bandwidth - sit->mvAllocatedBand[time]; //0 - 95%达到 goal_bandwidth 还需要多少带宽
            if(sit->mIsPer5Time[time] == true) //如果 sit->mvAllocatedBand[time] > goal_bandwidth ，表示这是偷跑的时刻，因为 95% 可能出现顶替现象，所以这里再把5%部分拉大一下，目标是拉到真实带宽上限
                goal = sit->mbandwidth - sit->mvAllocatedBand[time];

            vector<BandwithStream*> vStreamInPer5;
            //到了这里，时刻 time 还需要从别处取多少带宽就已经算出来了，就是 goal
            //开始根据从该边缘节点连接的流中取
            // 算 sit 在 time时刻旗下的 流片
            vector<BandwithStream*> vecStream;
            findSiteStream( time ,vecStream,sit);
//            sort(vecStream.begin(), vecStream.end(), cmpstreamsmooth);
            //到了这里，时刻 time 还需要从别处取多少带宽就已经算出来了，就是 goal
            //开始根据从该边缘节点连接的流中取
            for(int st = 0; st < vecStream.size(); ++st){
                BandwithStream* stream = vecStream[st];

                //如果这个流是分配给该边缘节点本身或分配给已经平滑过的边缘节点，则跳过
                if(stream->mSit == sit || stream->mdemand > goal || stream->mSit->IsSmooth == true)
                    continue;

                if(stream->mSit->mIsPer5Time[time] == true){
                    vStreamInPer5.emplace_back(stream);
                    continue;
                }

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }


            //分配属于 5% 的流给sit
            for(int st = 0; st < vStreamInPer5.size() && goal > 0; ++st){
                BandwithStream* stream = vStreamInPer5[st];
                if(stream->mdemand > goal)
                    continue;

                Client* cli = stream->mCli; //该流原先属于哪个客户节点
                Site* src_sit = stream->mSit;   //该流原先分配给哪个边缘节点

                //在 cli 的解决方案中找到该流，将其从 分配给 src_sit 改成分配给 sit

                //在 cli 的解决方案中找到该流，从分配给 src_sit 中删除，同时更新下 src_sit->mvAllocatedBand[time]
                for(int so = 0; so < cli->mvsolution[time][src_sit].size(); ++so){
                    if(cli->mvsolution[time][src_sit][so] == stream){
                        cli->mvsolution[time][src_sit][so] = cli->mvsolution[time][src_sit][cli->mvsolution[time][src_sit].size() - 1];
                        src_sit->mvAllocatedBand[time] -= stream->mdemand;
                        cli->mvsolution[time][src_sit].pop_back();
                    }
                }

                goal -= stream->mdemand;
                //分配给 sit
                SmoothallocateStreamToSite(time, sit, cli, stream);
            }
        }

    }
}

