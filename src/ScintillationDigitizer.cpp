#include "ScintillationDigitizer.h"

ScintillationDigitizer::ScintillationDigitizer() 
    : DigitizationBase("Scintillation") {
}

void ScintillationDigitizer::initializeTree() {
    // 创建Tree来保存事件数据
    dataTree = std::make_unique<TTree>("scintEvents", "Scintillation Digitization Events");
    
    // 添加分支
    dataTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    dataTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
    dataTree->Branch("phScin", &phScin, "phScin/D");
    dataTree->Branch("phScinAtt", &phScinAtt, "phScinAtt/D");
    dataTree->Branch("phScinAttLYRand", &phScinAttLYRand, "phScinAttLYRand/D");
}

void ScintillationDigitizer::initializeSamplingTree() {
    // 创建均匀抽样树
    samplingTree = std::make_unique<TTree>("scintSampling", "Scintillation Uniform Sampling Events");
    
    // 添加分支 - 与常规树相同的分支
    samplingTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    samplingTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
    samplingTree->Branch("phScin", &phScin, "phScin/D");
    samplingTree->Branch("phScinAtt", &phScinAtt, "phScinAtt/D");
    samplingTree->Branch("phScinAttLYRand", &phScinAttLYRand, "phScinAttLYRand/D");
}

double ScintillationDigitizer::digitize(double energy) {
    // 记录输入能量
    inputEnergy = energy;

    double LY = params.getParameter("EcalCryIntLY");
    double CryAtt = params.getParameter("EcalCryAtt");
    double LYRandFactor = rand.Gaus(1.0, params.getParameter("EcalCryLYUn"));
    
    // 闪烁体产生光子
    int ScinGen = 0;
    // ScinGen = std::round(rand.Gaus(energy * LY, energy * LY * params.getParameter("EcalCryLOFlu")));
    if(params.getParameter("EcalCryLOFlu") == 0.0) {
        ScinGen = std::round(rand.Poisson(energy * LY));
    } else {
        ScinGen = std::round(rand.Gaus(energy * LY, energy * LY * params.getParameter("EcalCryLOFlu")));
    }
    phScin = ScinGen;
    // 考虑光衰减后到达SiPM的光子数
    int sEcalCryAttLO = 0;
    if(ScinGen < 100)
    {
        sEcalCryAttLO = std::round(rand.Binomial(ScinGen, CryAtt));
    }
    else{
        if(ScinGen * CryAtt < 20)
        {
            sEcalCryAttLO = std::round(rand.Poisson(ScinGen * CryAtt));
        }
        else{
            sEcalCryAttLO = std::round(rand.Gaus(ScinGen * CryAtt, sqrt(ScinGen * CryAtt * (1 - CryAtt))));
        }
    }
    // sEcalCryAttLO = ScinGen * CryAtt;
    phScinAtt = sEcalCryAttLO;

    // 给LY添加涨落
    int NofPhotons = std::round(sEcalCryAttLO * LYRandFactor);
    if (NofPhotons < 0) NofPhotons = 0;
    phScinAttLYRand = NofPhotons;
    
    // 计算输出能量
    outputEnergy = NofPhotons / (LY * CryAtt);
    
    // 填充Tree
    if(dataTree) dataTree->Fill();
    
    return outputEnergy;
} 