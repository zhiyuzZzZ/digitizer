#include "SiPMDigitizer.h"

SiPMDigitizer::SiPMDigitizer() 
    : DigitizationBase("SiPM") {
}

void SiPMDigitizer::initializeTree() {
    // 创建Tree
    dataTree = std::make_unique<TTree>("sipmEvents", "SiPM Digitization Events");
    
    // 添加分支
    dataTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    dataTree->Branch("peSiPM", &peSiPM, "peSiPM/D");
    dataTree->Branch("peSiPMSat", &peSiPMSat, "peSiPMSat/D");
    dataTree->Branch("dc", &dc, "dc/D");
    dataTree->Branch("dcCT", &dcCT, "dcCT/D");
    dataTree->Branch("peTotal", &peTotal, "peTotal/D");
    dataTree->Branch("peTotalGainFluc", &peTotalGainFluc, "peTotalGainFluc/D");
    dataTree->Branch("peTotalGainFlucPedSub", &peTotalGainFlucPedSub, "peTotalGainFlucPedSub/D");
    dataTree->Branch("peTotalGainFlucPedSub_corr", &peTotalGainFlucPedSub_corr, "peTotalGainFlucPedSub_corr/D");
    dataTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
}

void SiPMDigitizer::initializeSamplingTree() {
    // 创建均匀抽样树
    samplingTree = std::make_unique<TTree>("sipmSampling", "SiPM Uniform Sampling Events");
    
    // 添加分支 - 与常规树相同的分支
    samplingTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    samplingTree->Branch("peSiPM", &peSiPM, "peSiPM/D");
    samplingTree->Branch("peSiPMSat", &peSiPMSat, "peSiPMSat/D");
    samplingTree->Branch("dc", &dc, "dc/D");
    samplingTree->Branch("dcCT", &dcCT, "dcCT/D");
    samplingTree->Branch("peTotal", &peTotal, "peTotal/D");
    samplingTree->Branch("peTotalGainFluc", &peTotalGainFluc, "peTotalGainFluc/D");
    samplingTree->Branch("peTotalGainFlucPedSub", &peTotalGainFlucPedSub, "peTotalGainFlucPedSub/D");
    samplingTree->Branch("peTotalGainFlucPedSub_corr", &peTotalGainFlucPedSub_corr, "peTotalGainFlucPedSub_corr/D");
    samplingTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
}

double SiPMDigitizer::digitize(double energy) {
    // 记录输入能量
    inputEnergy = energy;
    
    // 获取光子信号（来自闪烁体）
    double LY = params.getParameter("EcalCryIntLY");
    double CryAtt = params.getParameter("EcalCryAtt");
    double PDE = params.getParameter("EcalSiPMPDE");
    
    double NPE = energy *  LY * CryAtt * PDE;
    peSiPM = NPE;

    TF1* f_SiPMResponse = params.getSiPMResponseFunction();
    TF1* f_SiPMSigmaDet = params.getSiPMSigmaDetFunction();
    TF1* f_SiPMSigmaRecp = params.getSiPMSigmaRecpFunction();
    TF1* f_SiPMSigmaRecm = params.getSiPMSigmaRecmFunction();
    TF1* f_AsymGauss = params.getAsymGaussFunction();

    double NPESat;
    double SiPMCT = params.getParameter("EcalSiPMCT");
    if(params.getParameter("EcalSiPMDigiVerbose") == 0 || NPE < 100) {
        NPESat = rand.Poisson(NPE) * (1 + SiPMCT);
    } else {
        double NPE_ = f_SiPMResponse->Eval(NPE);
        NPESat = rand.Gaus(NPE_, f_SiPMSigmaDet->Eval(NPE_));
    }
    if(NPESat < 0) NPESat = 0;
    peSiPMSat = NPESat;
    
    // 计算暗噪声和串扰
    double darkRate = params.getParameter("EcalSiPMDCR");
    double gateTime = params.getParameter("EcalTimeInterval");
    int darkCount = rand.Poisson(darkRate * gateTime);
    dc = darkCount;

    // 计算串扰
    int darkCountCT = 0;
    for(int i=0;i<darkCount;i++) {
        double dark_rdm = rand.Uniform(0, 1);
        int sum_dc = 1;
        if(! (dark_rdm <= f_DarkNoise->Eval(sum_dc))) {
            double prob = f_DarkNoise->Eval(sum_dc);
            while(dark_rdm > prob) {
                sum_dc++;
                prob += f_DarkNoise->Eval(sum_dc);
            }
        }
        darkCountCT += sum_dc;
    }
    dcCT = darkCountCT;
    
    // 总信号（光信号+暗噪声）
    double NPETotal = NPESat + darkCountCT;
    peTotal = NPETotal;

    double SiPMGainMean = params.getParameter("EcalSiPMGainMean");
    double SiPMGainSigma = params.getParameter("EcalSiPMGainSigma");
    
    double SiPMCharge_sigma = std::sqrt(NPETotal * pow(SiPMGainMean * SiPMGainSigma, 2));
    double SiPMCharge = gRandom->Gaus(NPETotal * SiPMGainMean, SiPMCharge_sigma);

    if(SiPMCharge < 0) SiPMCharge = 0;
    double NPETotal_GainFluc_PedSub = SiPMCharge / SiPMGainMean - darkRate * gateTime * (1 + SiPMCT);
    peTotalGainFluc = SiPMCharge / SiPMGainMean;
    peTotalGainFlucPedSub = NPETotal_GainFluc_PedSub;

    if(params.getParameter("EcalSiPMDigiVerbose") <= 1 ) {
        peTotalGainFlucPedSub_corr = NPETotal_GainFluc_PedSub;
        
        // 转换回能量
        outputEnergy = NPETotal_GainFluc_PedSub / (LY * CryAtt * PDE);
    }
    else if(params.getParameter("EcalSiPMDigiVerbose") >= 2) {
        // 转换回能量
        double signal_corr = f_SiPMResponse->GetX(NPETotal_GainFluc_PedSub);

        outputEnergy = signal_corr / (LY * CryAtt * PDE);
    } 

    // 填充Tree
    if(dataTree) dataTree->Fill();
    
    return outputEnergy;
} 