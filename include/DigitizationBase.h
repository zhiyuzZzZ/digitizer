#ifndef DIGITIZATION_BASE_H
#define DIGITIZATION_BASE_H

#include "DetectorParameters.h"
#include <TF1.h>
#include <TRandom3.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TFile.h>
#include <TTree.h>
#include <vector>
#include <string>
#include <memory>
#include <chrono>

class DigitizationBase {
public:
    DigitizationBase(const std::string& name);
    virtual ~DigitizationBase();
    
    // 数字化单个能量值
    virtual double digitize(double energy) = 0;
    
    // 运行数字化过程并生成结果
    virtual void run(int nEvents = 100000);
    
    // 保存结果
    virtual void saveResults(const std::string& outputFile);
    
    // 获取能量直方图
    std::vector<TH1D*> getEnergyHistograms() const;
    
    // 设置随机数种子
    void setRandomSeed(unsigned int seed) { rand.SetSeed(seed); }
    
    // 获取结果直方图和图表的访问器
    TH1D* getResponseHistogram(double energy) const;
    
    // 获取分辨率图（已弃用）
    TGraphErrors* getResolutionGraph() const;
    
    // 新增：设置是否进行均匀能量抽样
    void setUniformSampling(bool enable) { uniformSampling = enable; }
    
    // 新增：设置均匀抽样的能量范围
    void setSamplingRange(double min, double max) { 
        samplingMinEnergy = min; 
        samplingMaxEnergy = max; 
    }
    
    // 清除所有能量点
    void clearEnergyPoints() {
        energies.clear();
        // 清除相关的直方图
        h_Energies.clear();
    }
    
    // 添加能量点
    void addEnergyPoint(double energy) {
        energies.push_back(energy);
    }
    
    // 更新参数
    virtual void updateParameters() {
        // 不需要重新获取参数实例，因为params是引用
        // 直接使用现有的引用即可
        
        // 更新函数参数
        initializeFunctions();
        
        std::cout << "已更新 " << moduleName << " 数字化器的参数" << std::endl;
    }
    
protected:
    // 模块名称
    std::string moduleName;
    
    // 参数访问
    DetectorParameters& params;
    
    // 随机数生成器
    TRandom3 rand;
    
    // 能量点
    std::vector<double> energies;
    
    // 直方图和图表 - 只用于计算过程，不存储
    std::vector<std::unique_ptr<TH1D>> h_Energies;
    std::unique_ptr<TH2D> h2_dynamic;
    
    // 添加Tree来保存事件数据
    std::unique_ptr<TTree> dataTree;
    
    // 新增：均匀抽样的事件树
    std::unique_ptr<TTree> samplingTree;
    
    // 事件级数据树 - 保存每个事件的能量响应
    std::unique_ptr<TTree> eventTreePtr;
    
    // 数据存储结构
    struct ResolutionData {
        std::vector<double> energy;
        std::vector<double> resolution;
        std::vector<double> resError;
    };
    
    struct LinearityData {
        std::vector<double> inputEnergy;
        std::vector<double> responseDiff;
    };
    
    // 存储数据结构
    ResolutionData resolutionData;
    LinearityData linearityData;
    
    // 响应函数
    std::unique_ptr<TF1> f_SiPMResponse;
    std::unique_ptr<TF1> f_SiPMSigmaDet;
    std::unique_ptr<TF1> f_SiPMSigmaRecp;
    std::unique_ptr<TF1> f_SiPMSigmaRecm;
    std::unique_ptr<TF1> f_AsymGauss;
    std::unique_ptr<TF1> f_DarkNoise;
    
    // 2D直方图容器
    std::vector<TH2D*> histograms2D;
    
    // 初始化函数
    void initializeHistograms();
    void initializeFunctions();
    
    // 添加初始化树的方法
    virtual void initializeTree();
    
    // 新增：初始化均匀抽样树的方法
    virtual void initializeSamplingTree();
    
    // 初始化数据树
    void initializeDataTrees();
    
    // 计算能量分辨率（不再进行拟合）
    void calculateResolution();
    
    // 计算暗噪声平均值
    double calculateMeanCT();
    
    // 新增：执行均匀能量抽样
    void runUniformSampling(int nEvents);
    
    // 新增：均匀抽样相关变量
    bool uniformSampling = false;
    double samplingMinEnergy = 0.0;
    double samplingMaxEnergy = 0.0;
    
    // 保存参数到ROOT文件
    void saveParametersToFile(TFile* file);
    
    double inputEnergy;  // 输入能量
};

#endif // DIGITIZATION_BASE_H 