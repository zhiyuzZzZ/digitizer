#include "TotalDigitizer.h"
#include <iostream>
#include <TCanvas.h>

TotalDigitizer::TotalDigitizer() 
    : DigitizationBase("Total") {
}

void TotalDigitizer::initializeTree() {
    // 创建Tree
    dataTree = std::make_unique<TTree>("totalEvents", "Total Digitization Chain Events");
    
    // 添加分支
    dataTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    dataTree->Branch("phScin", &phScin, "phScin/D");
    dataTree->Branch("phScinAtt", &phScinAtt, "phScinAtt/D");
    dataTree->Branch("phScinAttLYRand", &phScinAttLYRand, "phScinAttLYRand/D");
    dataTree->Branch("peSiPM", &peSiPM, "peSiPM/D");
    dataTree->Branch("peSiPMSat", &peSiPMSat, "peSiPMSat/D");
    dataTree->Branch("dc", &dc, "dc/D");
    dataTree->Branch("dcCT", &dcCT, "dcCT/D");
    dataTree->Branch("peSiPMSatDark", &peSiPMSatDark, "peSiPMSatDark/D");
    dataTree->Branch("peSiPMSatDarkGainFlu", &peSiPMSatDarkGainFlu, "peSiPMSatDarkGainFlu/D");
    dataTree->Branch("peSiPMSatDarkGainFluPedSub", &peSiPMSatDarkGainFluPedSub, "peSiPMSatDarkGainFluPedSub/D");
    dataTree->Branch("peSiPMSatDarkGainFluPedSubCut", &peSiPMSatDarkGainFluPedSubCut, "peSiPMSatDarkGainFluPedSubCut/D");
    dataTree->Branch("adcInitial", &adcInitial, "adcInitial/D");
    dataTree->Branch("adcGainCorr", &adcGainCorr, "adcGainCorr/D");
    dataTree->Branch("gainMode", &gainMode, "gainMode/D");
    dataTree->Branch("gain", &gain, "gain/D");
    dataTree->Branch("noiseFEE", &noiseFEE, "noiseFEE/D");
    dataTree->Branch("noiseASIC", &noiseASIC, "noiseASIC/D");
    dataTree->Branch("pedMean", &pedMean, "pedMean/D");
    dataTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
}

void TotalDigitizer::initializeSamplingTree() {
    // 创建均匀抽样树
    samplingTree = std::make_unique<TTree>("totalSampling", "Total Uniform Sampling Events");
    
    // 添加分支 - 与常规树相同的分支
    samplingTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    samplingTree->Branch("phScin", &phScin, "phScin/D");
    samplingTree->Branch("phScinAtt", &phScinAtt, "phScinAtt/D");
    samplingTree->Branch("phScinAttLYRand", &phScinAttLYRand, "phScinAttLYRand/D");
    samplingTree->Branch("peSiPM", &peSiPM, "peSiPM/D");
    samplingTree->Branch("peSiPMSat", &peSiPMSat, "peSiPMSat/D");
    samplingTree->Branch("dc", &dc, "dc/D");
    samplingTree->Branch("dcCT", &dcCT, "dcCT/D");
    samplingTree->Branch("peSiPMSatDark", &peSiPMSatDark, "peSiPMSatDark/D");
    samplingTree->Branch("peSiPMSatDarkGainFlu", &peSiPMSatDarkGainFlu, "peSiPMSatDarkGainFlu/D");
    samplingTree->Branch("peSiPMSatDarkGainFluPedSub", &peSiPMSatDarkGainFluPedSub, "peSiPMSatDarkGainFluPedSub/D");
    samplingTree->Branch("peSiPMSatDarkGainFluPedSubCut", &peSiPMSatDarkGainFluPedSubCut, "peSiPMSatDarkGainFluPedSubCut/D");
    samplingTree->Branch("adcInitial", &adcInitial, "adcInitial/D");
    samplingTree->Branch("adcGainCorr", &adcGainCorr, "adcGainCorr/D");
    samplingTree->Branch("gainMode", &gainMode, "gainMode/D");
    samplingTree->Branch("gain", &gain, "gain/D");
    samplingTree->Branch("noiseFEE", &noiseFEE, "noiseFEE/D");
    samplingTree->Branch("noiseASIC", &noiseASIC, "noiseASIC/D");
    samplingTree->Branch("pedMean", &pedMean, "pedMean/D");
    samplingTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
}

double TotalDigitizer::digitize(double energy) {
    // 记录输入能量
    inputEnergy = energy;
    
    // 闪烁体数字化
    double MIPEnergy = params.getParameter("EcalMIPEnergy");
    double LY = params.getParameter("EcalCryIntLY");
    double Att = params.getParameter("EcalCryAtt");
    double LYRandFactor = rand.Gaus(1.0, params.getParameter("EcalCryLYUn"));
    
    int ScinGen = 0;
    if(params.getParameter("EcalCryLOFlu") == 0.0) {
        ScinGen = std::round(rand.Poisson(energy * LY));
    } else {
        ScinGen = std::round(rand.Gaus(energy * LY, energy * LY * params.getParameter("EcalCryLOFlu")));
    }
    phScin = ScinGen;

    int ScinGenAtt = 0;
    if(ScinGen < 100)
    {
        ScinGenAtt = std::round(rand.Binomial(ScinGen, Att));
    }
    else{
        if(ScinGen * Att < 20)
        {
            ScinGenAtt = std::round(rand.Poisson(ScinGen * Att));
        }
        else{
            ScinGenAtt = std::round(rand.Gaus(ScinGen * Att, sqrt(ScinGen * Att * (1 - Att))));
        }
    }
    phScinAtt = ScinGenAtt;
    double nPhotons = std::round(ScinGenAtt * LYRandFactor);
    if (nPhotons < 0) nPhotons = 0;
    phScinAttLYRand = nPhotons;
    
    // SiPM数字化
    double SiPMPDE = params.getParameter("EcalSiPMPDE");
    double SiPMCT = params.getParameter("EcalSiPMCT");
    double darkRate = params.getParameter("EcalSiPMDCR");
    double gateTime = params.getParameter("EcalTimeInterval");
    double SiPMGainMean = params.getParameter("EcalSiPMGainMean");
    double SiPMGainSigma = params.getParameter("EcalSiPMGainSigma");

    TF1* f_SiPMResponse = params.getSiPMResponseFunction();
    TF1* f_SiPMSigmaDet = params.getSiPMSigmaDetFunction();
    TF1* f_SiPMSigmaRecp = params.getSiPMSigmaRecpFunction();
    TF1* f_SiPMSigmaRecm = params.getSiPMSigmaRecmFunction();
    TF1* f_AsymGauss = params.getAsymGaussFunction();
    
    int peSignal = std::round(nPhotons * SiPMPDE);
    peSiPM = peSignal;
    double peSignalSat = 0;
    if(params.getParameter("EcalSiPMDigiVerbose") == 0 || peSignal < 100) {
        peSignalSat = rand.Poisson(peSignal) * (1 + SiPMCT);
    } else {
        double peSignal_ = f_SiPMResponse->Eval(peSignal);
        peSignalSat = rand.Gaus(peSignal_, f_SiPMSigmaDet->Eval(peSignal_));
    }
    if(peSignalSat < 0) peSignalSat = 0;
    peSiPMSat = peSignalSat;

    // 计算暗噪声和串扰
    double darkCount = rand.Poisson(darkRate * gateTime);
    dc = darkCount;

    int darkCount_CT = 0;
    for(int i=0;i<darkCount;i++) {
        double dark_rdm = rand.Uniform(0, 1);
        int sum_darkcounts = 1;
        if(! (dark_rdm <= f_DarkNoise->Eval(sum_darkcounts))) {
            double prob = f_DarkNoise->Eval(sum_darkcounts);
            while(dark_rdm > prob) {
                sum_darkcounts++;
                prob += f_DarkNoise->Eval(sum_darkcounts);
            }
        }
        darkCount_CT += sum_darkcounts;
    }
    dcCT = darkCount_CT;

    double signalSiPM = peSignalSat + darkCount_CT;
    peSiPMSatDark = signalSiPM;
    double SiPMCharge_sigma = std::sqrt(signalSiPM * pow(SiPMGainMean * SiPMGainSigma, 2));
    double SiPMCharge = gRandom->Gaus(signalSiPM * SiPMGainMean, SiPMCharge_sigma);
    peSiPMSatDarkGainFlu = SiPMCharge / SiPMGainMean;

    double totalSignal_PedSub = SiPMCharge / SiPMGainMean - darkRate * gateTime * (1 + SiPMCT);

    if(totalSignal_PedSub < 0){
        signalSiPM = 0;
        totalSignal_PedSub = 0;
    }
    else {
        signalSiPM = totalSignal_PedSub;
    }
    peSiPMSatDarkGainFluPedSub = totalSignal_PedSub;
    
    // ADC数字化
    double FEENoiseSigma = params.getParameter("EcalFEENoiseSigma");
    double ASICNoiseSigma = params.getParameter("EcalASICNoiseSigma");
    double pedestal = params.getParameter("Pedestal");
    int ADCbit = params.getParameter("ADCbit");
    int adcSwitch = params.getParameter("ADCSwitch");
    // int adcMax = std::pow(2, ADCbit) - 1;
    int adcMax = adcSwitch;
    double gainRatio12 = params.getParameter("GainRatio_12");
    double gainRatio23 = params.getParameter("GainRatio_23");
    double MIPThreshold = params.getParameter("EcalMIP_Thre");
 
    signalSiPM = signalSiPM * params.getParameter("EcalRatioTimeInterval");
    peSiPMSatDarkGainFluPedSubCut = signalSiPM;

    // 计算ADC值
    double adcMean = signalSiPM * SiPMGainMean + pedestal;
    double adcSigma = std::sqrt(FEENoiseSigma * FEENoiseSigma + ASICNoiseSigma * ASICNoiseSigma);
    int adc = std::round(rand.Gaus(adcMean, adcSigma));
    if (adc < 0) adc = 0;
    adcInitial = adc;
    // 增益范围1（高增益）
    if (adc <= adcSwitch) {
        gainMode = 1;
        gain = SiPMGainMean;
        noiseFEE = FEENoiseSigma;
        noiseASIC = ASICNoiseSigma;

        if(params.getParameter("EcalSiPMDigiVerbose") >= 2 && signalSiPM >= 100) {
            double signalSiPM_ADCRec = (adc - pedestal) / SiPMGainMean;
            double signalSiPM_ADCRec_mean = f_SiPMResponse->GetX(signalSiPM_ADCRec);
            adc = signalSiPM_ADCRec_mean * SiPMGainMean + pedestal;
        }

        double pedestal_mean = pedestal + darkRate * gateTime * (1 + SiPMCT) * SiPMGainMean;
        pedMean = pedestal_mean;

        if(adc < 0) adc = 0;
        adcGainCorr = SiPMGainMean;
        
        // 转换回能量
        outputEnergy = (adc - pedestal_mean) / SiPMGainMean / (LY * SiPMPDE * Att);
    }
    else if(adc > adcSwitch && static_cast<int>(adc / gainRatio12) <= adcSwitch) {
        gainMode = 2;
        
        // 调整增益和噪声
        double adjustedGain = SiPMGainMean / gainRatio12;
        double adjustedFEENoise = FEENoiseSigma / gainRatio12;

        gain = adjustedGain;
        noiseFEE = adjustedFEENoise;
        noiseASIC = ASICNoiseSigma;

        adcMean = signalSiPM * adjustedGain + pedestal;
        adcSigma = std::sqrt(adjustedFEENoise * adjustedFEENoise + ASICNoiseSigma * ASICNoiseSigma);
        adc = std::round(rand.Gaus(adcMean, adcSigma));
        if (adc < 0) adc = 0;
        adcInitial = adc;

        if(params.getParameter("EcalSiPMDigiVerbose") >= 2 && signalSiPM >= 100) {
            double signalSiPM_ADCRec = (adc - pedestal) / SiPMGainMean;
            double signalSiPM_ADCRec_mean = f_SiPMResponse->GetX(signalSiPM_ADCRec);
            adc = signalSiPM_ADCRec_mean * adjustedGain + pedestal;
        }
        double pedestal_mean = pedestal + darkRate * gateTime * (1 + SiPMCT) * adjustedGain;
        pedMean = pedestal_mean;
        
        if(adc < 0) adc = 0;
        adcGainCorr = adjustedGain;

        // 转换回能量
        outputEnergy = (adc - pedestal_mean) / adjustedGain / (LY * SiPMPDE * Att);
    }
    else if(static_cast<int>(adc / gainRatio12) > adcSwitch) {
        gainMode = 3;

        // 调整增益和噪声
        double adjustedGain = SiPMGainMean / gainRatio12 / gainRatio23;
        double adjustedFEENoise = FEENoiseSigma / gainRatio12 / gainRatio23;

        gain = adjustedGain;
        noiseFEE = adjustedFEENoise;
        noiseASIC = ASICNoiseSigma;

        adcMean = signalSiPM * adjustedGain + pedestal;
        adcSigma = std::sqrt(adjustedFEENoise * adjustedFEENoise + ASICNoiseSigma * ASICNoiseSigma);
        adc = std::round(rand.Gaus(adcMean, adcSigma));
        if (adc < 0) adc = 0;
        if(adc > adcMax) adc = adcMax;
        adcInitial = adc;

        if(params.getParameter("EcalSiPMDigiVerbose") >= 2 && signalSiPM >= 100) {
            double signalSiPM_ADCRec = (adc - pedestal) / SiPMGainMean;
            double signalSiPM_ADCRec_mean = f_SiPMResponse->GetX(signalSiPM_ADCRec);
            adc = signalSiPM_ADCRec_mean * adjustedGain + pedestal;
        }
        
        double pedestal_mean = pedestal + darkRate * gateTime * (1 + SiPMCT) * adjustedGain;
        pedMean = pedestal_mean;

        if(adc < 0) adc = 0;
        adcGainCorr = adjustedGain;
        
        // 转换回能量
        outputEnergy = (adc - pedestal_mean) / adjustedGain / (LY * SiPMPDE * Att);
    }
    if (outputEnergy < MIPThreshold * MIPEnergy) outputEnergy = 0;
    // 填充Tree
    if(dataTree) dataTree->Fill();
    
    return outputEnergy;
}

void TotalDigitizer::run(int nEvents) {
    // 调用基类的run方法
    DigitizationBase::run(nEvents);
    
    // 计算等效噪声能量
    calculateENE();
}

void TotalDigitizer::calculateENE() {
    double ENE[3]; // 三种增益下的等效噪声能量
    
    // 计算各增益区间的等效噪声能量
    double fEcalSiPMGainMean = params.getParameter("EcalSiPMGainMean");
    double fEcalFEENoiseSigma = params.getParameter("EcalFEENoiseSigma");
    double fEcalASICNoiseSigma = params.getParameter("EcalASICNoiseSigma");
    double fPedestal = params.getParameter("Pedestal");
    double fEcalCryMipLY = params.getParameter("EcalCryMipLY");
    double fEcalMIPEnergy = params.getParameter("EcalMIPEnergy");
    double fEcalCryEffLY = params.getParameter("EcalCryEffLY");
    double fGainRatio_12 = params.getParameter("GainRatio_12");
    double fGainRatio_23 = params.getParameter("GainRatio_23");
    
    // 增益1
    ENE[0] = std::sqrt(fEcalFEENoiseSigma * fEcalFEENoiseSigma + 
                      fEcalASICNoiseSigma * fEcalASICNoiseSigma) / 
                      fEcalSiPMGainMean / fEcalCryEffLY;
    
    // 增益2
    ENE[1] = std::sqrt(fEcalFEENoiseSigma * fEcalFEENoiseSigma / (fGainRatio_12 * fGainRatio_12) + 
                      fEcalASICNoiseSigma * fEcalASICNoiseSigma) / 
                      (fEcalSiPMGainMean / fGainRatio_12) / fEcalCryEffLY;
    
    // 增益3
    ENE[2] = std::sqrt(fEcalFEENoiseSigma * fEcalFEENoiseSigma / (fGainRatio_12 * fGainRatio_12 * fGainRatio_23 * fGainRatio_23) + 
                      fEcalASICNoiseSigma * fEcalASICNoiseSigma) / 
                      (fEcalSiPMGainMean / fGainRatio_12 / fGainRatio_23) / fEcalCryEffLY;
    
    // 计算每个增益区间的能量边界
    double E_gain1_left = 0.1;
    double E_gain1_right = (params.getParameter("ADCSwitch") - fPedestal) / 
                          fEcalSiPMGainMean / fEcalCryEffLY;
    
    double E_gain2_left = E_gain1_right;
    double E_gain2_right = (params.getParameter("ADCSwitch") - fPedestal) / 
                          (fEcalSiPMGainMean / fGainRatio_12) / fEcalCryEffLY;
    
    double E_gain3_left = E_gain2_right;
    double E_gain3_right = (params.getParameter("ADCSwitch") - fPedestal) / 
                          (fEcalSiPMGainMean / fGainRatio_12 / fGainRatio_23) / fEcalCryEffLY;
    
    // 设置bin边界和创建直方图
    double binEdges[] = {E_gain1_left, E_gain1_right, E_gain2_right, E_gain3_right};
    h_ENE = std::make_unique<TH1D>("h_ENE", ";Channel Energy [MeV];ENE [MeV]", 
                                   params.getParameter("NofGain"), binEdges);
    
    // 填充直方图
    for (int i = 0; i < params.getParameter("NofGain"); i++) {
        h_ENE->SetBinContent(i+1, ENE[i]);
    }
}

void TotalDigitizer::saveResults(const std::string& outputFile) {
    // 调用基类的保存方法
    DigitizationBase::saveResults(outputFile);
    
    // 追加保存ENE直方图
    TFile* file = TFile::Open(outputFile.c_str(), "UPDATE");
    if (h_ENE) {
        h_ENE->Write("", TObject::kWriteDelete);
    }
    file->Close();
    delete file;
}

void TotalDigitizer::plotResults(const std::string& outputPrefix) {
    // 由于DigitizationBase没有plotResults方法，不能调用基类方法
    // 删除这一行:
    // DigitizationBase::plotResults(outputPrefix);
    
    // 应该改为直接实现绘图逻辑，或者使用ROOT脚本
    std::cout << "使用ROOT脚本绘制结果图:" << std::endl;
    std::cout << "root -l 'scripts/plot_results.C(\"digi_out_Total.root\", \"" 
              << outputPrefix << "\")'" << std::endl;
} 