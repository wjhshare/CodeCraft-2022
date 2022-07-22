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
#include <climits>

using namespace std;

//#define LOCALPATH
#define TESTOUTPUT

struct Client;
typedef struct Site{
    std::string mID;    //边缘节点ID
    int mbandwidth; //本边缘节点带宽上限
    int RunTimes = 0;   //记录已经偷跑了几次
    int min_Run;    //记录每个节点偷跑的最后一位的数值
    int mT; //保存该边缘节点当前所属时刻，只在偷跑时用，95%时因为按时间顺序，用不到它
    vector<int> mvAllocatedBand;   //记录该边缘节点每个时刻已分配带宽
    std::vector<Client*> mvLinkClient;    //与本边缘节点相连的客户节点ID（时延满足要求即视为相连）
    std::vector<std::vector<std::pair<Client*, unsigned long long>>> mvsolution;    //解决方案（用来测试查看的）
    vector<unsigned long long> mvMax_bedemand;  //每个时刻的总需求
    unordered_set<int> Per5_t;  //偷跑的时刻
    unsigned long long Alldemand = 0;   //整个时序总需求

    unordered_set<Site*> commomSite;   //连通边缘点
}Site;

typedef struct Client{
    std::string mID;
    int mT;
    std::vector<int> mvdemand;  //时序需求带宽, mvdemand[t]：表示 t 时刻本客户节点需求的带宽
    std::vector<Site*> mvLinkSite;  //与本客户节点相连的边缘节点（时延满足要求即视为相连）
    vector<int> mvAllocateddemand;    //保存该客户节点每个时刻被分配了的带宽
    unsigned long long Alldemand = 0;
    //时序分配方案
    // mvsolution[t]：保存 t 时刻本客户节点将带宽分配给std::vector<std::pair<std::string, int>>个边缘节点，
    // 每个 pair 分别存储被本客户节点请求带宽 > 0 的边缘节点ID及被请求的带宽
    std::vector<std::vector<std::pair<Site*, unsigned long long>>> mvsolution;
}Client;


typedef struct PathtInf{
    std::string configini;
    std::string demandcsv;
    std::string site_bandwidthcsv;
    std::string qoscsv;
    std::string solutiontxt;
    std::string SiteSolutiontxt;
} PathtInf;


//读取config文件，并将键-值保存于Content中
bool ReadConfig(const std::string& filename,
                std::map<std::string, int> &Content,
                const char* section);

//读取数据demand.csv，客户节点ID保存于 vClient， 时间名称保存于vTime
bool ReadData(const std::string& filename,
              std::vector<std::string>& vTime,
              std::vector<Client>& vClient,
              std::unordered_map<std::string, int>& hash_Client_name_idx);

//读取数据 site_bandwidth.csv，将节点名保存到 vSiteID；带宽上限保存到 data 中, data[i] 即为 vSiteID[i] 的带宽上限
bool ReadSitedData(const std::string& filename,
                  std::vector<Site>& vSite,
                  std::unordered_map<std::string, int>& hash_Site_name_idx);

//读取QoS文件，并保存于 data ，其中列表示客户节点，行表示边缘节点，data[i][j]表示 vSiteID[i]、vClientID[j] 的网络时延
bool ReadQos(const std::string& filename,
             std::vector<Client>& vClient,
             std::vector<Site>& vSite,
             std::unordered_map<std::string, int>& hash_Client_name_idx,
             std::unordered_map<std::string, int>& hash_Site_name_idx,
             int qos_upper);

//输出,vSolution[i][j][q] 表示 vTime[i], vClientID[j], vSiteID[q] 的请求带宽
bool WriteSolution(const std::string& filename,
                   std::vector<std::string>& vTime,
                   std::vector<Client>& vClient);
//测试查看Site分配
void WriteSiteSolutionmax(const std::string& filename,
                          std::vector<Site>& vSite);

bool InitialData(std::map<std::string, int>& mContent,
                 std::vector<std::string>& mvTime,
                 std::vector<Client>& mvClient,
                 std::vector<Site>& mvSite,
                 std::unordered_map<std::string, int>& hash_Client_name_idx,
                 std::unordered_map<std::string, int>& hash_Site_name_idx,
                 int& qos_upper,
                 PathtInf path);

// 采用暴力方法，无优化方案，测试输入输出
void TestFunction(std::vector<std::string>& mvTime,
                  std::vector<Client>& mvClient,
                  std::vector<Site>& mvSite );
// 优化方案写在此函数中
void WayOneFunction(std::vector<std::string>& mvTime,
                    std::vector<Client>& mvClient,
                    std::vector<Site>& mvSite );

void mxelse(std::vector<std::string>& mvTime,
            std::vector<Client>& mvClient,
            std::vector<Site>& mvSite,
            vector<Site*>& mvSitePtr);

/***************************************************************************************/
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

    std::map<std::string, int> mContent;
    std::vector<std::string> mvTime;
    std::vector<Client> mvClient;   //结构体存储客户节点
    std::vector<Site> mvSite;
    std::unordered_map<std::string, int> hash_Site_name_idx, hash_Client_name_idx;  //分别用哈希表分别存储 边缘节点的 ID 及其对应 mvSite 的下标，同理客户节点
    int qos_upper;
    if(  ! InitialData(mContent,mvTime,mvClient,mvSite,hash_Client_name_idx,hash_Site_name_idx,qos_upper,path) ){
        std::cout<<"initial failed"<<std::endl;
        return -1;
    }

#ifdef TESTOUTPUT
    //用于测试的输出
    TestFunction(mvTime,mvClient,mvSite);
//    WriteSiteSolutionmax(path.SiteSolutiontxt, mvSite);

    //判题
/*    for(int t = 0; t < mvTime.size(); ++t){
        for(int c = 0; c < mvClient.size(); ++c){
            Client* cur = &mvClient[c];
            unsigned long long count_solution_t = 0;
            for(int so = 0; so < mvClient[c].mvsolution[t].size(); ++so){
                count_solution_t += mvClient[c].mvsolution[t][so].second;
            }
            if(count_solution_t < mvClient[c].mvdemand[t]){
                cout << t <<"时刻未分配完 " << mvClient[c].mID << endl;
                bool m = false;
                for(int s = 0; s < mvClient[c].mvLinkSite.size(); ++s){
                    if(mvClient[c].mvLinkSite[s]->mbandwidth != mvClient[c].mvLinkSite[s]->mvAllocatedBand[t])
                        cout << "其连接的边缘节点" << mvClient[c].mvLinkSite[s]->mID << "有带宽" << endl;
                }
            }
            else if(count_solution_t > mvClient[c].mvdemand[t]) {
                cout << t << "时刻超分配 " << mvClient[c].mID << endl;
            }
        }
    }*/
#else
    WayOneFunction(mvTime,mvClient,mvSite);
#endif

    WriteSolution(path.solutiontxt, mvTime, mvClient);


    finish = clock();
    std::cout << "运行时间： " << (double)(finish - start) / CLOCKS_PER_SEC << std::endl;

    return 0;
}


bool InitialData(std::map<std::string, int>& mContent,
                 std::vector<std::string>& mvTime,
                 std::vector<Client>& mvClient,
                 std::vector<Site>& mvSite,
                 std::unordered_map<std::string, int>& hash_Client_name_idx,
                 std::unordered_map<std::string, int>& hash_Site_name_idx,
                 int& qos_upper,
                 PathtInf path){
    bool ret = true;
    ret = ret && ReadConfig(path.configini, mContent, "config");    //读取配置文件
    std::cout << mContent["qos_constraint"] << std::endl;
    qos_upper = mContent["qos_constraint"];

    ret = ret && ReadData(path.demandcsv, mvTime, mvClient, hash_Client_name_idx);  //读取客户节点
    std::cout << "Time: " << mvTime[mvTime.size() - 1] << std::endl;
    std::cout << mvClient[mvClient.size() - 1].mID <<", " << mvClient[mvClient.size() - 1].mvdemand[0] << std::endl;

    ret = ret && ReadSitedData(path.site_bandwidthcsv, mvSite, hash_Site_name_idx);  //读取边缘节点
    std::cout << "site id: " << mvSite[mvSite.size() - 1].mID <<" ,\t"
              <<"site bandwidth: " << mvSite[mvSite.size() - 1].mbandwidth << std::endl;

    ret = ret && ReadQos(path.qoscsv, mvClient, mvSite, hash_Client_name_idx, hash_Site_name_idx, qos_upper);  //读取QoS,并记录 client 和 site的连接关系
    std::cout << "边缘节点"<< mvSite[0].mID << "连接的客户节点数为： " << mvSite[0].mvLinkClient.size()
              << " ,\t" <<mvSite[0].mvLinkClient[0]->mID << std::endl;
    return ret;
}
void WayOneFunction(std::vector<std::string>& mvTime,
                    std::vector<Client>& mvClient,
                    std::vector<Site>& mvSite ){

}

//边缘节点按总需求从大到小排序
bool cmpSitedemand(Site* a, Site* b){
    return ((a->Alldemand > b->Alldemand) ||
            (a->Alldemand == b->Alldemand) && (a->mbandwidth > b->mbandwidth));
}
//客户节点按连接边缘节点数从少到多排序
bool cmpClientLink(Client* a, Client* b){
    return a->mvLinkSite.size() < b->mvLinkSite.size();
}

bool cmpClientLink2(Client* a, Client* b){
    return ((a->mvLinkSite.size() < b->mvLinkSite.size()) ||
            (a->mvLinkSite.size() == b->mvLinkSite.size() && a->Alldemand > b->Alldemand));
}

//小顶堆，分别保存pair<时刻，该时刻该边缘节点的需求>，用来存放 5%
typedef struct cmpSitePri{
    bool operator()(pair<int, unsigned long long>& a, pair<int, unsigned long long>& b){
        return a.second > b.second;
    }
};

void TestFunction(std::vector<std::string>& mvTime,
                  std::vector<Client>& mvClient,
                  std::vector<Site>& mvSite ){
    vector<Site*> mvSitePtr;    //保存边缘节点指针，用来对边缘节点进行排序用，避免直接排序 mvSite 导致客户节点中mvLinkSite出错
    //初始化客户节点和边缘节点的一些重要变量
    for(int c = 0; c < mvClient.size(); ++c){
        mvClient[c].mvsolution = vector<vector<pair<Site*, unsigned long long>>>(mvTime.size(), vector<pair<Site*, unsigned long long>>());
        mvClient[c].mvAllocateddemand = vector<int>(mvTime.size(), 0);
        for(int t = 0; t < mvTime.size(); ++t){
            mvClient[c].Alldemand += mvClient[c].mvdemand[t];
        }

        //根据客户点添加边缘点到边缘点的边
        for(int s = 0; s < mvClient[c].mvLinkSite.size(); ++s){
            for(int ss = 0; ss < mvClient[c].mvLinkSite.size(); ++ss){
                if(ss == s)
                    continue;
                else
                    mvClient[c].mvLinkSite[s]->commomSite.insert(mvClient[c].mvLinkSite[ss]);
            }
        }

    }
    for(int s = 0; s < mvSite.size(); ++s){
        mvSite[s].mvAllocatedBand = vector<int>(mvTime.size(), 0);
        mvSite[s].mvMax_bedemand = vector<unsigned long long>(mvTime.size(), 0);
        mvSite[s].RunTimes = 0;
        mvSite[s].min_Run = mvSite[s].mbandwidth;
        mvSite[s].mvsolution = vector<vector<pair<Client*, unsigned long long>>>(mvTime.size(), vector<pair<Client*, unsigned long long>>());
        for(int t = 0; t < mvTime.size(); ++t){
            for(int c = 0; c < mvSite[s].mvLinkClient.size(); ++c){
                mvSite[s].mvMax_bedemand[t] += mvSite[s].mvLinkClient[c]->mvdemand[t];
                mvSite[s].Alldemand += mvSite[s].mvLinkClient[c]->mvdemand[t];;
            }
        }
        //初始化mvSitePtr，即将每个边缘节点在 mvSite 中的地址存入
        mvSitePtr.push_back((&mvSite[s]));

    }


    mxelse(mvTime, mvClient, mvSite, mvSitePtr);

}

void RunPer5(std::vector<std::string>& mvTime,
            std::vector<Client>& mvClient,
            vector<Site*>& mvSitePtr){
    int CanRunTimes = floor(0.05*mvTime.size());
    cout << "CanRunTImes: " <<CanRunTimes << endl;

    unordered_set<Site*> hasRun;    //记录已经偷跑完 5% 的边缘节点，避免重复偷跑
    Site* RunSite_cur = mvSitePtr[mvSitePtr.size() - 1];    //第一个偷跑的边缘节点是总需求最少的那个,每次更新，即这就当前偷跑的边缘节点
    unsigned long long Max_demand = mvSitePtr[0]->Alldemand + 10000;    //记录一个比最大边缘节点需求大的数，用来更新下一个偷跑边缘节点选取时用
    if(CanRunTimes > 0) {   //需要偷跑
        for (int s = mvSitePtr.size() - 1; s >= 0; --s) {   //这个 s 从前到后和从后到前无所谓，这只是用来记录偷跑节点数用的
            RunSite_cur->Per5_t.clear();    //偷跑的边缘节点已经偷跑的时刻清空
            hasRun.insert(RunSite_cur); //hasRun插入该边缘节点，因为后面要偷跑它
            sort(RunSite_cur->mvLinkClient.begin(), RunSite_cur->mvLinkClient.end(), cmpClientLink2);    //连接的客户节点按连接数从少到多排序
            priority_queue<pair<int, unsigned long long>, vector<pair<int, unsigned long long>>, cmpSitePri> Per5;  //小顶堆存放要偷跑的 5% 时刻及这几个时刻对应的需求，按需求构造小顶堆
            for (int t = 0; t < mvTime.size(); ++t) {
                if (Per5.size() < CanRunTimes) {
                    Per5.push(make_pair(t, RunSite_cur->mvMax_bedemand[t]));
                    continue;
                } else {
                    unsigned long long top_demand = Per5.top().second;
                    if (RunSite_cur->mvMax_bedemand[t] > top_demand) {  //需求大于堆顶，则更换
                        Per5.pop();
                        Per5.push(make_pair(t, RunSite_cur->mvMax_bedemand[t]));
                        continue;
                    }
                }
            }   //找完要偷跑的时刻了

            //开始偷跑这 5% 时刻
            while (Per5.size() > 0) {
                int t = Per5.top().first;   //得到偷跑的时刻
                RunSite_cur->Per5_t.insert(t);  //将这时刻放进 Per5_t 里面
                Per5.pop();
                //查找相连客户节点（在前面已经排序了）
                for (int c = 0; c < RunSite_cur->mvLinkClient.size(); ++c) {
                    if (RunSite_cur->mbandwidth == RunSite_cur->mvAllocatedBand[t]) //偷跑边缘节点这时刻塞满了
                        break;
                    //这个客户节点的剩余需求
                    unsigned long long demand = RunSite_cur->mvLinkClient[c]->mvdemand[t] - RunSite_cur->mvLinkClient[c]->mvAllocateddemand[t];
                    //如果客户节点需求大于边缘节点剩余容量，则只分配到边缘节点容量
                    if (demand + RunSite_cur->mvAllocatedBand[t] > RunSite_cur->mbandwidth)
                        demand = RunSite_cur->mbandwidth - RunSite_cur->mvAllocatedBand[t];

                    RunSite_cur->mvLinkClient[c]->mvsolution[t].push_back(make_pair(RunSite_cur, demand)); //客户节点解决方案插入该分配
                    RunSite_cur->mvLinkClient[c]->mvAllocateddemand[t] += demand;   //客户节点这时刻已分配带宽更新
                    RunSite_cur->mvLinkClient[c]->Alldemand -= demand;  //客户节点总未分配的带宽
                    RunSite_cur->mvAllocatedBand[t] += demand;  //边缘节点这时刻已分配带宽更新
                    RunSite_cur->mvsolution[t].push_back(make_pair(RunSite_cur->mvLinkClient[c], demand));  //插入到边缘节点解决方案中，测试查看用
                    //该客户节点已经分配在这一时刻出去 demand 带宽了，因此其相连的所有边缘节点对应的总需求及这一时刻的需求要对应更新
                    for (int ss = 0; ss < RunSite_cur->mvLinkClient[c]->mvLinkSite.size(); ++ss) {
                        RunSite_cur->mvLinkClient[c]->mvLinkSite[ss]->mvMax_bedemand[t] -= demand;
                        RunSite_cur->mvLinkClient[c]->mvLinkSite[ss]->Alldemand -= demand;
                    }
                }
            }


            //更新，挑选下一个要偷跑的边缘节点（在还没偷跑的边缘节点中重新挑选总需求最小的（之所以不按第一次排序后的顺序选择是因为上面说了，客户节点分配完后，边缘节点的需求会被更新））
            unsigned long long Alldemand = Max_demand;  //Max_demand确保比所有边缘节点的总需求都大
            for(int ss = mvSitePtr.size() - 1; ss >= 0; --ss){
                //如果是还没偷跑的边缘节点，且总需求最小
                if(hasRun.find(mvSitePtr[ss]) == hasRun.end() && mvSitePtr[ss]->Alldemand < Alldemand){
                    RunSite_cur = mvSitePtr[ss];    //RunSite记录新挑选的要偷跑的边缘节点
                    Alldemand = mvSitePtr[ss]->Alldemand;
                }
            }

        }
    }//5% 结束
}

//小顶堆，分别保存pair<时刻，该时刻该边缘节点的需求>，用来存放 5%
typedef struct cmpReAllocate{
    bool operator()(pair<int, int>& a, pair<int, unsigned long long>& b){
        return a.second > b.second;
    }
};

//连通的边缘点按总需求从小到大排序
bool cmpcommom(Site* a, Site* b){
    return a->Alldemand < b->Alldemand;
}

void RunPer95(std::vector<std::string>& mvTime,
             std::vector<Client>& mvClient,
             vector<Site*>& mvSitePtr){
    //95%,总需求大的优先
    //偷跑完了，再排一次序
    sort(mvSitePtr.begin(), mvSitePtr.end(), cmpSitedemand);

    for(int t = 0; t < mvTime.size(); ++t){
        for(int s = 0; s < mvSitePtr.size(); ++s){
//            mvSitePtr[s]->Per5_t.clear();
//            if(mvSitePtr[s]->Per5_t.find(t) != mvSitePtr[s]->Per5_t.end())
//                continue;
            if(mvSitePtr[s]->mvAllocatedBand[t] == mvSitePtr[s]->mbandwidth)
                continue;
            sort(mvSitePtr[s]->mvLinkClient.begin(), mvSitePtr[s]->mvLinkClient.end(), cmpClientLink2);
            for(int c = 0; c < mvSitePtr[s]->mvLinkClient.size(); ++c){
                if(mvSitePtr[s]->mbandwidth == mvSitePtr[s]->mvAllocatedBand[t])
                    break;
                //客户节点剩余需求
                int demand = mvSitePtr[s]->mvLinkClient[c]->mvdemand[t] - mvSitePtr[s]->mvLinkClient[c]->mvAllocateddemand[t];
                //剩余需求 > 边缘节点剩余空间，则值分配满边缘节点
                if(demand + mvSitePtr[s]->mvAllocatedBand[t] > mvSitePtr[s]->mbandwidth)
                    demand = mvSitePtr[s]->mbandwidth - mvSitePtr[s]->mvAllocatedBand[t];
                if(demand <= 0)
                    continue;
                int so = 0;
                //如果该边缘节点已经出现在客户结点的分配方案中了，那就分配的带宽 +=
                for(; so < mvSitePtr[s]->mvLinkClient[c]->mvsolution[t].size(); ++so){
                    if(mvSitePtr[s]->mvLinkClient[c]->mvsolution[t][so].first == mvSitePtr[s]){
                        mvSitePtr[s]->mvLinkClient[c]->mvsolution[t][so].second += demand;
                        break;
                    }
                }
                //如果该边缘节点没出现在客户结点的分配方案中，则 push 进去
                if(so == mvSitePtr[s]->mvLinkClient[c]->mvsolution[t].size())
                    mvSitePtr[s]->mvLinkClient[c]->mvsolution[t].push_back(make_pair(mvSitePtr[s], demand));
                mvSitePtr[s]->mvLinkClient[c]->mvAllocateddemand[t] += demand;  //更新客户节点已分配需求
//                mvSitePtr[s]->mvLinkClient[c]->Alldemand -= demand; //客户节点总未分配需求更新（注释掉，因为后面跑三角形要根据 5% 方案跑完后这个值进行连通边缘节点排序）
                mvSitePtr[s]->mvAllocatedBand[t] += demand; //更新边缘节点已分配需求
                mvSitePtr[s]->mvsolution[t].push_back(make_pair(mvSitePtr[s]->mvLinkClient[c], demand));//更新边缘节点解决方案（测试用）
                //更新该客户节点连接的所有边缘节点在当前时刻的需求
                for(int ss = 0; ss < mvSitePtr[s]->mvLinkClient[c]->mvLinkSite.size(); ++ss){
                    mvSitePtr[s]->mvLinkClient[c]->mvLinkSite[ss]->mvMax_bedemand[t] -= demand;
                }
            }
        }
    }


}
//分配三角形，跑完95%后，对 0-94% 进行再分配，将 0-94%尽可能拉到95%
void triangle(std::vector<std::string>& mvTime,
              std::vector<Client>& mvClient,
              std::vector<Site>& mvSite,
              vector<Site*>& mvSitePtr){
    unordered_set<Site*> hasRe; //记录已经再分配过的边缘节点
    int CanRunTimes = floor(0.05*mvTime.size());    //5%的长度
    //因为mvSitePtr之前就排了顺序，是总需求从大到小排的，这次再分配从小到大排（其实就是和跑95%顺序相反，即弱者优先，不过这里不要进行再次排序，因为和95%顺序相反才是目的）
    for(int s = mvSitePtr.size() - 1; s >= 0; --s){
        hasRe.insert(mvSitePtr[s]); //mvSitePtr[s]是当前要再分配的边缘节点，后面称为当前边缘节点

        //找出 95% 的值，放在 Per5Add1.top()，小顶堆
        priority_queue<int, vector<int>, greater<int>> Per5Add1;
        for(int t = 0; t < mvTime.size(); ++t){
            if(t < CanRunTimes + 1){
                Per5Add1.push(mvSitePtr[s]->mvAllocatedBand[t]);
            }
            else{
                int top_Allocate = Per5Add1.top();
                if(mvSitePtr[s]->mvAllocatedBand[t] > top_Allocate){
                    Per5Add1.pop();
                    Per5Add1.push(mvSitePtr[s]->mvAllocatedBand[t]);
                }
            }
        }

        //commomSite记录和当前要再分配的边缘节点有公共客户节点的边缘节点,后面称为连通边缘节点
        vector<Site*> commomSite(mvSitePtr[s]->commomSite.begin(), mvSitePtr[s]->commomSite.end());
        sort(commomSite.begin(), commomSite.end(), cmpcommom);  //连通边缘节点按总需求从小到大排序（是跑完 5% 还没跑 95% 的总需求，在跑95%的时候只要不将总需求做变动就行）

        //sLinkClient 保存当前边缘节点连通的客户节点，用来查询用的
        unordered_set<Client*> sLinkClient(mvSitePtr[s]->mvLinkClient.begin(),mvSitePtr[s]->mvLinkClient.end());

        int Per95_value = Per5Add1.top();   //当前节点 95% 位置的值，即计费值，后面称为计费值
        for(int t = 0; t < mvTime.size(); ++t){
            if(mvSitePtr[s]->mvAllocatedBand[t] >= Per95_value) // 因为目的是让 0-94%的值 尽量和计费值一样大，因此 >= 计费值 的值只有两种（归属于 5% 的值 和 0-94%中已经和 计费值 一样大了的值，这部分就不用再分配了）
                continue;
            //进入这里表示这个时刻的值是 < 计费值的（即 0-94% 中可以提高值也不影响当前边缘节点成本）
            //目标是压到和 95% 一样大（这样如果取到其他边缘节点 95%时刻 的值就可以降低其他边缘节点的成本了）
            int goal = Per95_value - mvSitePtr[s]->mvAllocatedBand[t];  //目标值

            for(int ss = 0; ss < commomSite.size(); ++ss){  //遍历当前边缘节点的 连通边缘节点（优先需求小的）
                if(hasRe.find(commomSite[ss]) != hasRe.end())   //该连通边缘节点已经再分配过了（即它在当前边缘节点再分配之前就进行再分配了，那么就不从这个边缘节点中取值，不然会陷入循环）
                    continue;

                //遍历当前连通边缘节点的客户节点
                for(int cc = 0; cc < commomSite[ss]->mvLinkClient.size(); ++cc){
                    Client* commomCite = commomSite[ss]->mvLinkClient[cc];  //当前连通边缘节点的客户节点
                    if(sLinkClient.find(commomCite) == sLinkClient.end())    //（查询当前边缘节点的客户节点，如果查得到证明这是公共边缘节点）公共客户节点（优先连接边缘节点数少的，这在跑 能者优先的时候就已经进行排序了，所以这部分就不用再排序了）
                        continue;       //不是公共客户节点

                    //进入到这里证明这是公共客户节点
                    //尽可能将公共客户节点的带宽在当前连通边缘节点中取出
                    int Allocate_Value = 0; //记录能取出多少
                    for(int cco = 0; cco < commomCite->mvsolution[t].size(); ++cco){    //查找公共客户节点的解决方案
                        if(commomCite->mvsolution[t][cco].first == commomSite[ss]){ //如果公共客户节点对该连通边缘节点有分配，则尽可能取出，分给当前边缘节点
                            Allocate_Value = commomCite->mvsolution[t][cco].second;
                            //尽可能取出
                            if(Allocate_Value > goal){  //如果能取出的值大于目标值，则取出目标值大小即可（因为取出 goal 分配给当前边缘节点即可让当前边缘节点在该时刻的值达到和 计费值 一样大了）
                                Allocate_Value = goal;
                                commomCite->mvsolution[t][cco].second -= Allocate_Value;    //从公共客户节点中取出带宽
                                commomSite[ss]->mvAllocatedBand[t] -= Allocate_Value;   //对应的当前连通边缘节点在当前时刻的已分配带宽要更新
                            }
                            else{   //如果能取出的值 <= 目标值，那么就全取出来，因为当前边缘节点能放下这个值且不超过计费值（即不会让当前边缘节点的成本提高）
                                //将公共客户节点解决方案中对应被取出的部分剔除（和末尾互换然后 pop_back()，因为被全取出来了）
                                commomCite->mvsolution[t][cco] = commomCite->mvsolution[t][commomCite->mvsolution[t].size() - 1];
                                commomCite->mvsolution[t].pop_back();
                                commomSite[ss]->mvAllocatedBand[t] -= Allocate_Value;   //对应的当前连通边缘节点在当前时刻的已分配带宽要更新
                            }
                            break;  //到了这里证明这个公共客户节点在当前连通边缘节点的分配带宽已经尽可能取出来u了，所以break
                        }
                    }

                    if(Allocate_Value == 0)
                        continue;   //公共客户节点对当前连通边缘节点没分配

                    //当前公共客户节点对当前连通边缘节点有分配，则将取出的分配值 给当前边缘节点
                    goal -= Allocate_Value; //目标 -Allocate_Value，更新目标，因为还有其他连通边缘节点到时还要查看，所以要更新目标值
                    mvSitePtr[s]->mvAllocatedBand[t] += Allocate_Value; //更新的当前边缘节点的已分配带宽
                    //查看当前公共客户节点是否对当前边缘节点有分配，有的话直接 +=，没有就 push
                    int in = 0;
                    for(; in < commomCite->mvsolution[t].size(); ++in){
                        if(commomCite->mvsolution[t][in].first == mvSitePtr[s]){    //前公共客户节点对当前边缘节点有分配
                            commomCite->mvsolution[t][in].second += Allocate_Value;
                            break;  //记得break，不然后面会再 push 一次
                        }
                    }
                    if(in == commomCite->mvsolution[t].size()){ //前公共客户节点是否对当前边缘节点没分配，push
                        commomCite->mvsolution[t].push_back(make_pair(mvSitePtr[s], Allocate_Value));
                    }

                }
            }
        }
    }
}

void mxelse(std::vector<std::string>& mvTime,
            std::vector<Client>& mvClient,
            std::vector<Site>& mvSite,
            vector<Site*>& mvSitePtr){
    //边缘节点排序，按总需求从大到小排序
    sort(mvSitePtr.begin(), mvSitePtr.end(), cmpSitedemand);
    RunPer5(mvTime, mvClient, mvSitePtr);
    RunPer95(mvTime, mvClient, mvSitePtr);
    triangle(mvTime, mvClient, mvSite, mvSitePtr);
}

bool WriteSolution(const std::string& filename,
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
            for(int s = 0; s < vClient[c].mvsolution[t].size(); ++s){
                if(!begin_Client){
                    solution <<",";
                }
                solution << "<" << vClient[c].mvsolution[t][s].first->mID << "," << vClient[c].mvsolution[t][s].second << ">";
                begin_Client = false;
            }
            if(t == vTime.size() - 1 && c == vClient.size() - 1) continue;  //去掉最后一行换行
            solution << std::endl;
        }
    }

    solution.close();
    return ret;
}

void WriteSiteSolutionmax(const std::string& filename,
                       std::vector<Site>& vSite){
    std::ofstream solution(filename, std::ios::out);
    if(!solution.is_open()){
        std::cout << "Site solution.txt flie open error" << std::endl;
        return;
    }

    for(int t = 0; t < vSite[0].mvAllocatedBand.size(); ++t){
        for(int s = 0; s < vSite.size(); ++s){
                solution << vSite[s].mvAllocatedBand[t] << ", ";
            }
        solution << endl;
    }
    for(int s = 0; s < vSite.size(); ++s){
        solution << vSite[s].mID << ", ";
    }
    solution.close();
    return;
}

bool ReadQos(const std::string& filename,
             std::vector<Client>& vClient,
             std::vector<Site>& vSite,
             std::unordered_map<std::string, int>& hash_Client_name_idx,
             std::unordered_map<std::string, int>& hash_Site_name_idx,
             int qos_upper){
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
    pos = line.find("mtime,");
    line = line.substr(pos + 6);    //去掉mtime,
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

    //读取需求
    while(getline(infile, line)){
        //读取时间
        pos = line.find(",");
        vTime.push_back(line.substr(0, pos));
        line = line.substr(pos + 1);

        int client_idx = 0;
        pos = line.find(",");
        while(pos > -1){
            vClient[client_idx].mvdemand.push_back(stoi(line.substr(0, pos)));
            ++client_idx;

            line = line.substr(pos + 1);
            pos = line.find(",");
        }
        pos = line.find("\r"); //定位换行符

        vClient[client_idx].mvdemand.push_back(stoi(line.substr(0, pos)));
    }

    //将 ID 和对应的 vClient 下标记录成哈希表
    for(int c = 0; c < vClient.size(); ++c){
        hash_Client_name_idx[vClient[c].mID] = c;
    }

    infile.close();
    return ret;
}

bool ReadConfig(const std::string& filename,
                std::map<std::string, int> &Content,
                const char* section){
    std::ifstream infile(filename.c_str());
    bool ret = true;
    if (!infile)
    {
        std::cout << "config file open error!" << std::endl;
        ret = false;
        return ret;
    }

    std::string line, key;
    int value;
    int pos = 0;
    std::string Tsection = std::string("[") + section + "]";
    bool flag = false;
    while (getline(infile, line))
    {
        //读取配置文件的section，即 [config]
        if(!flag)
        {
            pos = line.find(Tsection, 0);
            if(-1 == pos)
            {
                continue;
            }
            else
            {
                flag = true;
                getline(infile, line);
            }
        }

        pos = line.find("=");
        if(pos == -1 || line.find("#") == 0) continue;

        key = line.substr(0, pos);
        int endpos = line.find("\r\n");
        Content[key] = stoi(line.substr(pos + 1, endpos));
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